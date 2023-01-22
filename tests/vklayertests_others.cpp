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
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"
#include "core_validation_error_enums.h"

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
    if (IsPlatform(kNexusPlayer)) {
        GTEST_SKIP() << "This test should not run on Nexus Player";
    }

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

    PFN_vkDestroyPrivateDataSlotEXT pfn_vkDestroyPrivateDataSlotEXT =
        (PFN_vkDestroyPrivateDataSlotEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkDestroyPrivateDataSlotEXT");
    PFN_vkCreatePrivateDataSlotEXT pfn_vkCreatePrivateDataSlotEXT =
        (PFN_vkCreatePrivateDataSlotEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCreatePrivateDataSlotEXT");
    PFN_vkGetPrivateDataEXT pfn_vkGetPrivateDataEXT =
        (PFN_vkGetPrivateDataEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkGetPrivateDataEXT");
    PFN_vkSetPrivateDataEXT pfn_vkSetPrivateDataEXT =
        (PFN_vkSetPrivateDataEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkSetPrivateDataEXT");

    VkPrivateDataSlotEXT data_slot;
    VkPrivateDataSlotCreateInfoEXT data_create_info = LvlInitStruct<VkPrivateDataSlotCreateInfoEXT>();
    data_create_info.flags = 0;
    VkResult err = pfn_vkCreatePrivateDataSlotEXT(m_device->handle(), &data_create_info, NULL, &data_slot);
    if (err != VK_SUCCESS) {
        printf("Failed to create private data slot, VkResult %d.\n", err);
    }

    VkSampler sampler;
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
    vk::CreateSampler(m_device->handle(), &sampler_info, NULL, &sampler);

    static const uint64_t data_value = 0x70AD;
    err = pfn_vkSetPrivateDataEXT(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler, data_slot, data_value);
    if (err != VK_SUCCESS) {
        printf("Failed to set private data. VkResult = %d\n", err);
    }
    uint64_t data;
    pfn_vkGetPrivateDataEXT(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler, data_slot, &data);
    if (data != data_value) {
        m_errorMonitor->SetError("Got unexpected private data, %s.\n");
    }
    pfn_vkDestroyPrivateDataSlotEXT(m_device->handle(), data_slot, NULL);
    vk::DestroySampler(m_device->handle(), sampler, NULL);
}

TEST_F(VkLayerTest, PrivateDataFeature) {
    TEST_DESCRIPTION("Test privateData feature not being enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_PRIVATE_DATA_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // feature not enabled
    ASSERT_NO_FATAL_FAILURE(InitState());

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);
    PFN_vkCreatePrivateDataSlotEXT vkCreatePrivateDataSlotEXT =
        (PFN_vkCreatePrivateDataSlotEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCreatePrivateDataSlotEXT");

    VkPrivateDataSlotEXT data_slot;
    VkPrivateDataSlotCreateInfoEXT data_create_info = LvlInitStruct<VkPrivateDataSlotCreateInfoEXT>();
    data_create_info.flags = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreatePrivateDataSlot-privateData-04564");
    vkCreatePrivateDataSlotEXT(m_device->handle(), &data_create_info, NULL, &data_slot);
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
    VkBufferView buffer_view;
    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>(&custom_struct);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;

    vk::CreateBufferView(m_device->device(), &bvci, NULL, &buffer_view);

    vk::DestroyBufferView(m_device->device(), buffer_view, nullptr);
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
    VkBufferView buffer_view;
    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>(&custom_struct_b);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;

    vk::CreateBufferView(m_device->device(), &bvci, NULL, &buffer_view);

    vk::DestroyBufferView(m_device->device(), buffer_view, nullptr);
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
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    // Create an invalid pNext structure to trigger the stateless validation warning
    VkBaseOutStructure bogus_struct{};
    bogus_struct.sType = static_cast<VkStructureType>(0x33333333);
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&bogus_struct);

    // Should get the first three errors just fine
    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();
    // VUID-VkPhysicalDeviceProperties2-pNext-pNext produces a massive ~3600 character log message, which hits a
    // complex string buffer reallocation path inside of the logging code. Make sure it successfully prints out
    // the very end of the message.
    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "is undefined and may not work correctly with validation enabled");
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();

    // Limit should prevent the message from coming through a fourth time
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
}

TEST_F(VkLayerTest, MessageIdFilterString) {
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
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964",
                         nullptr);
}

TEST_F(VkLayerTest, MessageIdFilterHexInt) {
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
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964",
                         nullptr);
}

TEST_F(VkLayerTest, MessageIdFilterInt) {
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
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964",
                         nullptr);
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
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = &stageFlags;
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
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, bindStateTscShaderText, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, bindStateTeshaderText, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
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
    VkQueueFamilyProperties *queue_props = new VkQueueFamilyProperties[queue_node_count];
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, queue_props);
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

    PFN_vkDebugMarkerSetObjectNameEXT fpvkDebugMarkerSetObjectNameEXT =
        (PFN_vkDebugMarkerSetObjectNameEXT)vk::GetInstanceProcAddr(instance(), "vkDebugMarkerSetObjectNameEXT");
    if (!(fpvkDebugMarkerSetObjectNameEXT)) {
        GTEST_SKIP() << "Can't find fpvkDebugMarkerSetObjectNameEXT; skipped";
    }

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
    fpvkDebugMarkerSetObjectNameEXT(device(), &name_info);

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
    VkCommandPool commandpool_1;
    VkCommandPool commandpool_2;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_1);
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_2);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = commandpool_1;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.object = (uint64_t)commandBuffer;
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
    name_info.pObjectName = commandBuffer_name.c_str();
    fpvkDebugMarkerSetObjectNameEXT(device(), &name_info);

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
    vk::FreeCommandBuffers(device(), commandpool_2, 1, &commandBuffer);
    m_errorMonitor->VerifyFound();

    vk::DestroyCommandPool(device(), commandpool_1, NULL);
    vk::DestroyCommandPool(device(), commandpool_2, NULL);
}

TEST_F(VkLayerTest, DebugUtilsNameTest) {
    TEST_DESCRIPTION("Ensure debug utils object names are printed in debug messenger output");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT =
        (PFN_vkSetDebugUtilsObjectNameEXT)vk::GetInstanceProcAddr(instance(), "vkSetDebugUtilsObjectNameEXT");
    ASSERT_TRUE(vkSetDebugUtilsObjectNameEXT);  // Must be extant if extension is enabled
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vk::GetInstanceProcAddr(instance(), "vkCreateDebugUtilsMessengerEXT");
    ASSERT_TRUE(vkCreateDebugUtilsMessengerEXT);  // Must be extant if extension is enabled
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vk::GetInstanceProcAddr(instance(), "vkDestroyDebugUtilsMessengerEXT");
    ASSERT_TRUE(vkDestroyDebugUtilsMessengerEXT);  // Must be extant if extension is enabled
    PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT =
        (PFN_vkCmdInsertDebugUtilsLabelEXT)vk::GetInstanceProcAddr(instance(), "vkCmdInsertDebugUtilsLabelEXT");
    ASSERT_TRUE(vkCmdInsertDebugUtilsLabelEXT);  // Must be extant if extension is enabled

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
    vkCreateDebugUtilsMessengerEXT(instance(), &callback_create_info, nullptr, &my_messenger);

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
    vkSetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    // Pass in null handle
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02588");
    name_info.objectHandle = 0;
    vkSetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    // Pass in 'unknown' object type and see if parameter validation catches it
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02587");
    name_info.objectHandle = (uint64_t)memory_2;
    name_info.objectType = VK_OBJECT_TYPE_UNKNOWN;
    vkSetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    name_info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
    vkSetDebugUtilsObjectNameEXT(device(), &name_info);

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
    VkCommandPool commandpool_1;
    VkCommandPool commandpool_2;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_1);
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_2);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = commandpool_1;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.objectHandle = (uint64_t)commandBuffer;
    name_info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    name_info.pObjectName = commandBuffer_name.c_str();
    vkSetDebugUtilsObjectNameEXT(device(), &name_info);

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

    vkCmdInsertDebugUtilsLabelEXT(commandBuffer, &command_label);
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
    vk::FreeCommandBuffers(device(), commandpool_2, 1, &commandBuffer);
    m_errorMonitor->VerifyFound();

    vk::DestroyCommandPool(device(), commandpool_1, NULL);
    vk::DestroyCommandPool(device(), commandpool_2, NULL);
    vkDestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
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
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

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

    vkGetPhysicalDeviceProperties2KHR(gpu(), &physical_device_properties2);
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

TEST_F(VkLayerTest, SubmitSignaledFence) {
    vk_testing::Fence testFence;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "submitted in SIGNALED state.  Fences must be reset before being submitted");

    VkFenceCreateInfo fenceInfo = LvlInitStruct<VkFenceCreateInfo>();
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->ClearAllBuffers(m_renderTargets, m_clear_color, nullptr, m_depth_clear_color, m_stencil_clear_color);
    m_commandBuffer->end();

    testFence.init(*m_device, fenceInfo);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, testFence.handle());
    vk::QueueWaitIdle(m_device->m_queue);

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

    // vk::CreateInstance, and vk::CreateDevice tend to crash in the Loader Trampoline ATM, so choosing vk::CreateCommandPool
    const VkCommandPoolCreateInfo cpci = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, 0,
                                          DeviceObj()->QueueFamilyMatching(0, 0, true)};
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

TEST_F(VkLayerTest, TemporaryExternalSemaphore) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Check for external semaphore import and export capability
    auto esi = LvlInitStruct<VkPhysicalDeviceExternalSemaphoreInfoKHR>();
    esi.handleType = handle_type;

    auto esp = LvlInitStruct<VkExternalSemaphorePropertiesKHR>();

    auto vkGetPhysicalDeviceExternalSemaphorePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
    }

    VkResult err;

    // Create a semaphore to export payload from
    auto esci = LvlInitStruct<VkExportSemaphoreCreateInfoKHR>();
    esci.handleTypes = handle_type;
    auto sci = LvlInitStruct<VkSemaphoreCreateInfo>(&esci);

    vk_testing::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    sci.pNext = nullptr;
    vk_testing::Semaphore import_semaphore(*m_device, sci);

    vk_testing::Semaphore::ExternalHandle ext_handle{};
    err = export_semaphore.export_handle(ext_handle, handle_type);
    ASSERT_VK_SUCCESS(err);
    err = import_semaphore.import_handle(ext_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR);
    ASSERT_VK_SUCCESS(err);

    // Wait on the imported semaphore twice in vk::QueueSubmit, the second wait should be an error
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    std::vector<VkSubmitInfo> si(4, LvlInitStruct<VkSubmitInfo>());
    si[0].signalSemaphoreCount = 1;
    si[0].pSignalSemaphores = &export_semaphore.handle();

    si[1].waitSemaphoreCount = 1;
    si[1].pWaitSemaphores = &import_semaphore.handle();
    si[1].pWaitDstStageMask = &flags;

    si[2] = si[0];
    si[3] = si[1];

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress");
    vk::QueueSubmit(m_device->m_queue, si.size(), si.data(), VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    auto index = m_device->graphics_queue_node_index_;
    if (m_device->queue_props[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        // Wait on the imported semaphore twice in vk::QueueBindSparse, the second wait should be an error
        std::vector<VkBindSparseInfo> bi(4, LvlInitStruct<VkBindSparseInfo>());
        bi[0].signalSemaphoreCount = 1;
        bi[0].pSignalSemaphores = &export_semaphore.handle();

        bi[1].waitSemaphoreCount = 1;
        bi[1].pWaitSemaphores = &import_semaphore.handle();

        bi[2] = bi[0];
        bi[3] = bi[1];
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress");
        vk::QueueBindSparse(m_device->m_queue, bi.size(), bi.data(), VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Cleanup
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(VkLayerTest, ExternalTimelineSemaphore) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
    const char *no_tempory_tl_vuid = "VUID-VkImportSemaphoreWin32HandleInfoKHR-flags-03322";
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const char *no_tempory_tl_vuid = "VUID-VkImportSemaphoreFdInfoKHR-flags-03323";
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        GTEST_SKIP() << "Timeline semaphore not supported";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Check for external semaphore import and export capability
    {
        auto sti = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
        sti.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        auto esi = LvlInitStruct<VkPhysicalDeviceExternalSemaphoreInfoKHR>(&sti);
        esi.handleType = handle_type;

        auto esp = LvlInitStruct<VkExternalSemaphorePropertiesKHR>();

        auto vkGetPhysicalDeviceExternalSemaphorePropertiesKHR =
            (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vk::GetInstanceProcAddr(
                instance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
        vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

        if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
            !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
            GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
        }
    }

    VkResult err;

    // Create a semaphore to export payload from
    auto esci = LvlInitStruct<VkExportSemaphoreCreateInfoKHR>();
    esci.handleTypes = handle_type;
    auto stci = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>(&esci);
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    auto sci = LvlInitStruct<VkSemaphoreCreateInfo>(&stci);

    vk_testing::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore to import payload into
    stci.pNext = nullptr;
    vk_testing::Semaphore import_semaphore(*m_device, sci);

    vk_testing::Semaphore::ExternalHandle ext_handle{};
    err = export_semaphore.export_handle(ext_handle, handle_type);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, no_tempory_tl_vuid);
    err = import_semaphore.import_handle(ext_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR);
    m_errorMonitor->VerifyFound();

    err = import_semaphore.import_handle(ext_handle, handle_type);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(VkLayerTest, ExternalSyncFdSemaphore) {
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        GTEST_SKIP() << "Timeline semaphore not supported";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Check for external semaphore import and export capability
    auto esi = LvlInitStruct<VkPhysicalDeviceExternalSemaphoreInfoKHR>();
    esi.handleType = handle_type;

    auto esp = LvlInitStruct<VkExternalSemaphorePropertiesKHR>();

    auto vkGetPhysicalDeviceExternalSemaphorePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting";
    }

    if (!(esp.compatibleHandleTypes & handle_type)) {
        GTEST_SKIP() << "External semaphore does not support VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT";
    }

    VkResult err;

    // create a timeline semaphore.
    // Note that adding a sync fd VkExportSemaphoreCreateInfo will cause creation to fail.
    auto stci = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    auto sci = LvlInitStruct<VkSemaphoreCreateInfo>(&stci);

    vk_testing::Semaphore timeline_sem(*m_device, sci);

    // binary semaphore works fine.
    auto esci = LvlInitStruct<VkExportSemaphoreCreateInfo>();
    esci.handleTypes = handle_type;
    stci.pNext = &esci;

    stci.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    vk_testing::Semaphore binary_sem(*m_device, sci);

    // Create a semaphore to import payload into
    vk_testing::Semaphore import_semaphore(*m_device);

    vk_testing::Semaphore::ExternalHandle ext_handle{};

    // timeline not allowed
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreGetFdInfoKHR-handleType-01132");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreGetFdInfoKHR-handleType-03253");
    timeline_sem.export_handle(ext_handle, handle_type);
    m_errorMonitor->VerifyFound();

    // must have pending signal
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreGetFdInfoKHR-handleType-03254");
    binary_sem.export_handle(ext_handle, handle_type);
    m_errorMonitor->VerifyFound();

    auto si = LvlInitStruct<VkSubmitInfo>();
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &binary_sem.handle();

    err = vk::QueueSubmit(m_device->m_queue, 1, &si, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    err = binary_sem.export_handle(ext_handle, handle_type);
    ASSERT_VK_SUCCESS(err);

    // must be temporary
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportSemaphoreFdInfoKHR-handleType-07307");
    import_semaphore.import_handle(ext_handle, handle_type);
    m_errorMonitor->VerifyFound();

    err = import_semaphore.import_handle(ext_handle, handle_type, VK_SEMAPHORE_IMPORT_TEMPORARY_BIT);
    ASSERT_VK_SUCCESS(err);

    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(VkLayerTest, TemporaryExternalFence) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Check for external fence import and export capability
    auto efi = LvlInitStruct<VkPhysicalDeviceExternalFenceInfoKHR>();
    efi.handleType = handle_type;
    auto efp = LvlInitStruct<VkExternalFencePropertiesKHR>();
    auto vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vk::GetInstanceProcAddr(
        instance(), "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    vkGetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    VkResult err;

    // Create a fence to export payload from
    auto efci = LvlInitStruct<VkExportFenceCreateInfoKHR>();
    efci.handleTypes = handle_type;
    auto fci = LvlInitStruct<VkFenceCreateInfo>(&efci);
    vk_testing::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vk_testing::Fence import_fence(*m_device, fci);

    // Export fence payload to an opaque handle
    vk_testing::Fence::ExternalHandle ext_fence{};
    err = export_fence.export_handle(ext_fence, handle_type);
    ASSERT_VK_SUCCESS(err);
    err = import_fence.import_handle(ext_fence, handle_type, VK_FENCE_IMPORT_TEMPORARY_BIT_KHR);
    ASSERT_VK_SUCCESS(err);

    // Undo the temporary import
    vk::ResetFences(m_device->device(), 1, &import_fence.handle());

    // Signal the previously imported fence twice, the second signal should produce a validation error
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, import_fence.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-fence-00064");
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, import_fence.handle());
    m_errorMonitor->VerifyFound();

    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);

    // Signal without reseting
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-fence-00063");
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, import_fence.handle());
    m_errorMonitor->VerifyFound();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(VkLayerTest, InvalidExternalFence) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    const auto other_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
    const auto bad_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const char *bad_export_type_vuid = "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01452";
    const char *other_export_type_vuid = "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01448";
    const char *bad_import_type_vuid = "VUID-VkImportFenceWin32HandleInfoKHR-handleType-01457";
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const auto other_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR;
    const auto bad_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
    const char *bad_export_type_vuid = "VUID-VkFenceGetFdInfoKHR-handleType-01456";
    const char *other_export_type_vuid = "VUID-VkFenceGetFdInfoKHR-handleType-01453";
    const char *bad_import_type_vuid = "VUID-VkImportFenceFdInfoKHR-handleType-01464";
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Check for external fence import and export capability
    auto efi = LvlInitStruct<VkPhysicalDeviceExternalFenceInfoKHR>();
    efi.handleType = handle_type;
    auto efp = LvlInitStruct<VkExternalFencePropertiesKHR>();
    auto vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vk::GetInstanceProcAddr(
        instance(), "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    vkGetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    // Create a fence to export payload from
    auto efci = LvlInitStruct<VkExportFenceCreateInfoKHR>();
    efci.handleTypes = handle_type;
    auto fci = LvlInitStruct<VkFenceCreateInfo>(&efci);
    vk_testing::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vk_testing::Fence import_fence(*m_device, fci);

    vk_testing::Fence::ExternalHandle ext_handle{};

    // windows vs unix mismatch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, bad_export_type_vuid);
    export_fence.export_handle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();

    // a valid type for the platform which we didn't ask for during create
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, other_export_type_vuid);
    if (other_type == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR) {
        // SYNC_FD is a special snowflake
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkFenceGetFdInfoKHR-handleType-01454");
    }
    export_fence.export_handle(ext_handle, other_type);
    m_errorMonitor->VerifyFound();

    export_fence.export_handle(ext_handle, handle_type);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, bad_import_type_vuid);
    import_fence.import_handle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();
#ifdef VK_USE_PLATFORM_WIN32_KHR
    auto ifi = LvlInitStruct<VkImportFenceWin32HandleInfoKHR>();
    ifi.fence = import_fence.handle();
    ifi.handleType = handle_type;
    ifi.handle = ext_handle;
    ifi.flags = 0;
    ifi.name = L"something";

    auto vkImportFenceWin32HandleKHR =
        reinterpret_cast<PFN_vkImportFenceWin32HandleKHR>(vk::GetDeviceProcAddr(device(), "vkImportFenceWin32HandleKHR"));

    // If handleType is not VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT, name must be NULL
    // However, it looks like at least some windows drivers don't support exporting KMT handles for fences
    if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportFenceWin32HandleInfoKHR-handleType-01459");
    }
    // If handle is not NULL, name must be NULL
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportFenceWin32HandleInfoKHR-handle-01462");
    vkImportFenceWin32HandleKHR(m_device->device(), &ifi);
    m_errorMonitor->VerifyFound();
#endif
    auto err = import_fence.import_handle(ext_handle, handle_type);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(VkLayerTest, ExternalSyncFdFence) {
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR;

    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Check for external fence import and export capability
    auto efi = LvlInitStruct<VkPhysicalDeviceExternalFenceInfoKHR>();
    efi.handleType = handle_type;
    auto efp = LvlInitStruct<VkExternalFencePropertiesKHR>();
    auto vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vk::GetInstanceProcAddr(
        instance(), "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    vkGetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External fence does not support importing and exporting, skipping test.";
    }

    // Create a fence to export payload from
    auto efci = LvlInitStruct<VkExportFenceCreateInfoKHR>();
    efci.handleTypes = handle_type;
    auto fci = LvlInitStruct<VkFenceCreateInfo>(&efci);
    vk_testing::Fence export_fence(*m_device, fci);

    // Create a fence to import payload into
    fci.pNext = nullptr;
    vk_testing::Fence import_fence(*m_device, fci);

    vk_testing::Fence::ExternalHandle ext_handle{};

    // SYNC_FD must have a pending signal for export
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFenceGetFdInfoKHR-handleType-01454");
    export_fence.export_handle(ext_handle, handle_type);
    m_errorMonitor->VerifyFound();

    vk::QueueSubmit(m_device->m_queue, 0, nullptr, export_fence.handle());

    export_fence.export_handle(ext_handle, handle_type);

    // must be temporary
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportFenceFdInfoKHR-handleType-07306");
    import_fence.import_handle(ext_handle, handle_type);
    m_errorMonitor->VerifyFound();

    import_fence.import_handle(ext_handle, handle_type, VK_FENCE_IMPORT_TEMPORARY_BIT);

    import_fence.wait(1000000000);
}

TEST_F(VkLayerTest, InvalidCmdBufferEventDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to an event dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkEvent event;
    VkEventCreateInfo evci = LvlInitStruct<VkEventCreateInfo>();
    VkResult result = vk::CreateEvent(m_device->device(), &evci, NULL, &event);
    ASSERT_VK_SUCCESS(result);

    m_commandBuffer->begin();
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkEvent");
    // Destroy event dependency prior to submit to cause ERROR
    vk::DestroyEvent(m_device->device(), event, NULL);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdBufferQueryPoolDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a query pool dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    qpci.queryCount = 1;
    VkResult result = vk::CreateQueryPool(m_device->device(), &qpci, nullptr, &query_pool);
    ASSERT_VK_SUCCESS(result);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkQueryPool");
    // Destroy query pool dependency prior to submit to cause ERROR
    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

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

TEST_F(VkLayerTest, FeaturesMultiview) {
    TEST_DESCRIPTION("Checks VK_KHR_multiview features.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    std::vector<const char *> device_extensions;
    device_extensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);

    // Set false to trigger VUs
    multiview_features.multiview = VK_FALSE;

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

    if ((multiview_features.multiviewGeometryShader == VK_FALSE) && (multiview_features.multiviewTessellationShader == VK_FALSE)) {
        GTEST_SKIP() << "multiviewGeometryShader and multiviewTessellationShader feature not supported";
    }

    if (multiview_features.multiviewGeometryShader == VK_TRUE) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceMultiviewFeatures-multiviewGeometryShader-00580");
    }
    if (multiview_features.multiviewTessellationShader == VK_TRUE) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceMultiviewFeatures-multiviewTessellationShader-00581");
    }
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, BeginQueryOnTimestampPool) {
    TEST_DESCRIPTION("Call CmdBeginQuery on a TIMESTAMP query pool.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-02804");
    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();

    vk::BeginCommandBuffer(m_commandBuffer->handle(), &begin_info);
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, ValidationCacheTestBadMerge) {
    AddRequiredExtensions(VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Load extension functions
    auto fpCreateValidationCache =
        (PFN_vkCreateValidationCacheEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCreateValidationCacheEXT");
    auto fpDestroyValidationCache =
        (PFN_vkDestroyValidationCacheEXT)vk::GetDeviceProcAddr(m_device->device(), "vkDestroyValidationCacheEXT");
    auto fpMergeValidationCaches =
        (PFN_vkMergeValidationCachesEXT)vk::GetDeviceProcAddr(m_device->device(), "vkMergeValidationCachesEXT");

    VkValidationCacheCreateInfoEXT validationCacheCreateInfo = LvlInitStruct<VkValidationCacheCreateInfoEXT>();
    validationCacheCreateInfo.initialDataSize = 0;
    validationCacheCreateInfo.pInitialData = NULL;
    validationCacheCreateInfo.flags = 0;
    VkValidationCacheEXT validationCache = VK_NULL_HANDLE;
    VkResult res = fpCreateValidationCache(m_device->device(), &validationCacheCreateInfo, nullptr, &validationCache);
    ASSERT_VK_SUCCESS(res);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMergeValidationCachesEXT-dstCache-01536");
    res = fpMergeValidationCaches(m_device->device(), validationCache, 1, &validationCache);
    m_errorMonitor->VerifyFound();

    fpDestroyValidationCache(m_device->device(), validationCache, nullptr);
}

TEST_F(VkLayerTest, InvalidQueueFamilyIndex) {
    // Miscellaneous queueFamilyIndex validation tests
    bool get_physical_device_properties2 = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (get_physical_device_properties2) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkBufferCreateInfo buffCI = LvlInitStruct<VkBufferCreateInfo>();
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffCI.queueFamilyIndexCount = 2;
    // Introduce failure by specifying invalid queue_family_index
    uint32_t qfi[2];
    qfi[0] = 777;
    qfi[1] = 0;

    buffCI.pQueueFamilyIndices = qfi;
    buffCI.sharingMode = VK_SHARING_MODE_CONCURRENT;  // qfi only matters in CONCURRENT mode

    const char *vuid = (get_physical_device_properties2) ? "VUID-VkBufferCreateInfo-sharingMode-01419"
                                                         : "VUID-VkBufferCreateInfo-sharingMode-01391";
    // Test for queue family index out of range
    CreateBufferTest(*this, &buffCI, vuid);

    // Test for non-unique QFI in array
    qfi[0] = 0;
    CreateBufferTest(*this, &buffCI, vuid);

    if (m_device->queue_props.size() > 2) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pSubmits-04626");

        // Create buffer shared to queue families 1 and 2, but submitted on queue family 0
        buffCI.queueFamilyIndexCount = 2;
        qfi[0] = 1;
        qfi[1] = 2;
        VkBufferObj ib;
        ib.init(*m_device, buffCI);

        m_commandBuffer->begin();
        vk::CmdFillBuffer(m_commandBuffer->handle(), ib.handle(), 0, 16, 5);
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer(false);
        m_errorMonitor->VerifyFound();
    }

    // If there is more than one queue family, create a device with a single queue family, then create a buffer
    // with SHARING_MODE_CONCURRENT that uses a non-device PDEV queue family.
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    VkQueueFamilyProperties *queue_props = new VkQueueFamilyProperties[queue_count];
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props);

    if (queue_count < 3) {
        GTEST_SKIP() << "Multiple queue families are required to run this test.";
    }
    std::vector<float> priorities(queue_props->queueCount, 1.0f);
    VkDeviceQueueCreateInfo queue_info = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = queue_props->queueCount;
    queue_info.pQueuePriorities = priorities.data();
    VkDeviceCreateInfo dev_info = LvlInitStruct<VkDeviceCreateInfo>();
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    // Create a device with a single queue family
    VkDevice second_device;
    ASSERT_VK_SUCCESS(vk::CreateDevice(gpu(), &dev_info, nullptr, &second_device));

    // Select Queue family for CONCURRENT buffer that is not owned by device
    buffCI.queueFamilyIndexCount = 2;
    qfi[1] = 2;
    VkBuffer buffer = VK_NULL_HANDLE;
    vk::CreateBuffer(second_device, &buffCI, NULL, &buffer);
    vk::DestroyBuffer(second_device, buffer, nullptr);
    vk::DestroyDevice(second_device, nullptr);
}

TEST_F(VkLayerTest, InvalidQueryPoolCreate) {
    TEST_DESCRIPTION("Attempt to create a query pool for PIPELINE_STATISTICS without enabling pipeline stats for the device.");

    ASSERT_NO_FATAL_FAILURE(Init());

    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);

    VkDevice local_device;
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>();
    auto features = m_device->phy().features();
    // Intentionally disable pipeline stats
    features.pipelineStatisticsQuery = VK_FALSE;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.pEnabledFeatures = &features;
    VkResult err = vk::CreateDevice(gpu(), &device_create_info, nullptr, &local_device);
    ASSERT_VK_SUCCESS(err);

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
    qpci.queryCount = 1;
    VkQueryPool query_pool;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-queryType-00791");
    vk::CreateQueryPool(local_device, &qpci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-queryCount-02763");
    vk::CreateQueryPool(local_device, &qpci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    vk::DestroyDevice(local_device, nullptr);
}

TEST_F(VkLayerTest, InvalidQuerySizes) {
    TEST_DESCRIPTION("Invalid size of using queries commands.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (IsPlatform(kPixel2XL)) {
        GTEST_SKIP() << "This test should not run on Pixel 2 XL";
    }

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    const uint32_t timestampValidBits = queue_props[m_device->graphics_queue_node_index_].timestampValidBits;

    VkBufferObj buffer;
    buffer.init(*m_device, 128, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkMemoryRequirements mem_reqs = {};
    vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &mem_reqs);
    const VkDeviceSize buffer_size = mem_reqs.size;

    const uint32_t query_pool_size = 4;
    VkQueryPool occlusion_query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = query_pool_size;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &occlusion_query_pool);

    m_commandBuffer->begin();

    // FirstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00796");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00797");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool, query_pool_size, 1);
    m_errorMonitor->VerifyFound();

    // Sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00797");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool, 1, query_pool_size);
    m_errorMonitor->VerifyFound();

    // Actually reset all queries so they can be used
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool, 0, query_pool_size);

    vk::CmdBeginQuery(m_commandBuffer->handle(), occlusion_query_pool, 0, 0);

    // Query index to large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-query-00810");
    vk::CmdEndQuery(m_commandBuffer->handle(), occlusion_query_pool, query_pool_size);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(m_commandBuffer->handle(), occlusion_query_pool, 0);

    // FirstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00820");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool, query_pool_size, 1, buffer.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool, 1, query_pool_size, buffer.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // Offset larger than buffer size
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-dstOffset-00819");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool, 0, 1, buffer.handle(), buffer_size + 4, 0, 0);
    m_errorMonitor->VerifyFound();

    // Buffer does not have enough storage from offset to contain result of each query
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00824");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), occlusion_query_pool, 0, 2, buffer.handle(), buffer_size - 4, 4, 0);
    m_errorMonitor->VerifyFound();

    // Query is not a timestamp type
    if (timestampValidBits == 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-timestampValidBits-00829");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-queryPool-01416");
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, occlusion_query_pool, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];

    // FirstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00813");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00816");
    vk::GetQueryPoolResults(m_device->device(), occlusion_query_pool, query_pool_size, 1, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();

    // Sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00816");
    vk::GetQueryPoolResults(m_device->device(), occlusion_query_pool, 1, query_pool_size, out_data_size, &data, 4, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), occlusion_query_pool, nullptr);
}

TEST_F(VkLayerTest, UnclosedAndDuplicateQueries) {
    TEST_DESCRIPTION("End a command buffer with a query still in progress, create nested queries.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 5;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);
    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 5);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-01922");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 1, 0);
    // Attempt to begin a query that has the same type as an active query
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 3, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkEndCommandBuffer-commandBuffer-00061");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, QueryPreciseBit) {
    TEST_DESCRIPTION("Check for correct Query Precise Bit circumstances.");
    ASSERT_NO_FATAL_FAILURE(Init());

    // These tests require that the device support pipeline statistics query
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (VK_TRUE != device_features.pipelineStatisticsQuery) {
        GTEST_SKIP() << "Test requires unsupported pipelineStatisticsQuery feature";
    }

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Test for precise bit when query type is not OCCLUSION
    if (features.occlusionQueryPrecise) {
        VkEvent event;
        VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
        vk::CreateEvent(m_device->handle(), &event_create_info, nullptr, &event);

        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00800");

        VkQueryPool query_pool;
        VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_create_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_create_info.queryCount = 3;
        query_pool_create_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                                                    VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                                                    VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
        vk::CreateQueryPool(m_device->handle(), &query_pool_create_info, nullptr, &query_pool);

        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_errorMonitor->VerifyFound();
        // vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_commandBuffer->end();

        const size_t out_data_size = 64;
        uint8_t data[out_data_size];
        // The dataSize is too small to return the data
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 3, 8, &data, 12, 0);
        m_errorMonitor->VerifyFound();

        vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
        vk::DestroyEvent(m_device->handle(), event, nullptr);
    }

    // Test for precise bit when precise feature is not available
    features.occlusionQueryPrecise = false;
    VkDeviceObj test_device(0, gpu(), device_extension_names, &features);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vk::CreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_VK_SUCCESS(err);

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(test_device.handle(), &event_create_info, nullptr, &event);

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    vk::BeginCommandBuffer(cmd_buffer, &begin_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00800");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 2;
    vk::CreateQueryPool(test_device.handle(), &query_pool_create_info, nullptr, &query_pool);

    vk::CmdResetQueryPool(cmd_buffer, query_pool, 0, 2);
    vk::CmdBeginQuery(cmd_buffer, query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
    m_errorMonitor->VerifyFound();
    vk::EndCommandBuffer(cmd_buffer);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    // The dataSize is too small to return the data
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-dataSize-00817");
    vk::GetQueryPoolResults(test_device.handle(), query_pool, 0, 2, 8, &data, out_data_size / 2, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(test_device.handle(), query_pool, nullptr);
    vk::DestroyEvent(test_device.handle(), event, nullptr);
    vk::DestroyCommandPool(test_device.handle(), command_pool, nullptr);
}

TEST_F(VkLayerTest, StageMaskGsTsEnabled) {
    TEST_DESCRIPTION(
        "Attempt to use a stageMask w/ geometry shader and tesselation shader bits enabled when those features are disabled on the "
        "device.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();
    // Make sure gs & ts are disabled
    features.geometryShader = false;
    features.tessellationShader = false;
    // The sacrificial device object
    VkDeviceObj test_device(0, gpu(), device_extension_names, &features);

    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vk::CreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_VK_SUCCESS(err);

    VkEvent event;
    VkEventCreateInfo evci = LvlInitStruct<VkEventCreateInfo>();
    VkResult result = vk::CreateEvent(test_device.handle(), &evci, NULL, &event);
    ASSERT_VK_SUCCESS(result);

    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    vk::BeginCommandBuffer(cmd_buffer, &cbbi);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-04090");
    vk::CmdSetEvent(cmd_buffer, event, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-04091");
    vk::CmdSetEvent(cmd_buffer, event, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT);
    m_errorMonitor->VerifyFound();

    vk::DestroyEvent(test_device.handle(), event, NULL);
    vk::DestroyCommandPool(test_device.handle(), command_pool, NULL);
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

TEST_F(VkLayerTest, DescriptorPoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete a DescriptorPool with a DescriptorSet that is in use.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create image to update the descriptor with
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);
    // Create Sampler
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    VkResult err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // Update descriptor with image and sampler
    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put pool in-flight
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Destroy pool while in-flight, causing error
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyDescriptorPool-descriptorPool-00303");
    vk::DestroyDescriptorPool(m_device->device(), pipe.descriptor_set_->pool_, NULL);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
    // Cleanup
    vk::DestroySampler(m_device->device(), sampler, NULL);
    m_errorMonitor->SetUnexpectedError(
        "If descriptorPool is not VK_NULL_HANDLE, descriptorPool must be a valid VkDescriptorPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove DescriptorPool obj");
    // TODO : It seems Validation layers think ds_pool was already destroyed, even though it wasn't?
}

TEST_F(VkLayerTest, FramebufferInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use framebuffer.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 256, 256, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Just use default renderpass with our framebuffer
    m_renderPassBeginInfo.framebuffer = fb;
    // Create Null cmd buffer for submit
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put it in-flight
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Destroy framebuffer while in-flight
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyFramebuffer-framebuffer-00892");
    vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    m_errorMonitor->VerifyFound();
    // Wait for queue to complete so we can safely destroy everything
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If framebuffer is not VK_NULL_HANDLE, framebuffer must be a valid VkFramebuffer handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Framebuffer obj");
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
}

TEST_F(VkLayerTest, PushDescriptorUniformDestroySignaled) {
    TEST_DESCRIPTION("Destroy a uniform buffer in use by a push descriptor set");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        GTEST_SKIP() << VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDevicePushDescriptorPropertiesKHR push_descriptor_prop = LvlInitStruct<VkPhysicalDevicePushDescriptorPropertiesKHR>();
    GetPhysicalDeviceProperties2(push_descriptor_prop);
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        GTEST_SKIP() << "maxPushDescriptors is zero, skipping tests";
    }

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});
    // Create push descriptor set layout
    const VkDescriptorSetLayoutObj push_ds_layout(m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    // Use helper to create graphics pipeline
    CreatePipelineHelper helper(*this);
    helper.InitInfo();
    helper.InitState();
    helper.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&push_ds_layout, &ds_layout});
    helper.CreateGraphicsPipeline();

    const float vbo_data[3] = {1.f, 0.f, 1.f};
    auto vbo = std::make_unique<VkConstantBufferObj>(m_device, sizeof(vbo_data), &vbo_data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buff_info;
    buff_info.buffer = vbo->handle();
    buff_info.offset = 0;
    buff_info.range = sizeof(vbo_data);
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = 0;  // Should not cause a validation error

    // Find address of extension call and make the call
    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetKHR");
    assert(vkCmdPushDescriptorSetKHR != nullptr);

    m_commandBuffer->begin();

    // In Intel GPU, it needs to bind pipeline before push descriptor set.
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);
    vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_layout_.handle(), 0, 1,
                              &descriptor_write);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), vbo->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
    vbo.reset();
}

TEST_F(VkLayerTest, FramebufferImageInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use image that's child of framebuffer.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 256;
    image_ci.extent.height = 256;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.flags = 0;
    VkImageObj image(m_device);
    image.init(&image_ci);

    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 256, 256, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Just use default renderpass with our framebuffer
    m_renderPassBeginInfo.framebuffer = fb;
    // Create Null cmd buffer for submit
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put it (and attached imageView) in-flight
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer to put framebuffer and children in-flight
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Destroy image attached to framebuffer while in-flight
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyImage-image-01000");
    vk::DestroyImage(m_device->device(), image.handle(), NULL);
    m_errorMonitor->VerifyFound();
    // Wait for queue to complete so we can safely destroy image and other objects
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If image is not VK_NULL_HANDLE, image must be a valid VkImage handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Image obj");
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
}

TEST_F(VkLayerTest, EventInUseDestroyedSignaled) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    m_commandBuffer->end();
    vk::DestroyEvent(m_device->device(), event, nullptr);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "that is invalid because bound");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InUseDestroyedSignaled) {
    TEST_DESCRIPTION(
        "Use vkCmdExecuteCommands with invalid state in primary and secondary command buffers. Delete objects that are in use. "
        "Call VkQueueSubmit with an event that has been deleted.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    VkFence fence;
    ASSERT_VK_SUCCESS(vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence));

    VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_test.GetBuffer(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    m_commandBuffer->begin();

    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, fence);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyEvent-event-01145");
    vk::DestroyEvent(m_device->device(), event, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroySemaphore-semaphore-01137");
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyFence-fence-01120");
    vk::DestroyFence(m_device->device(), fence, nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If semaphore is not VK_NULL_HANDLE, semaphore must be a valid VkSemaphore handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Semaphore obj");
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    m_errorMonitor->SetUnexpectedError("If fence is not VK_NULL_HANDLE, fence must be a valid VkFence handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Fence obj");
    vk::DestroyFence(m_device->device(), fence, nullptr);
    m_errorMonitor->SetUnexpectedError("If event is not VK_NULL_HANDLE, event must be a valid VkEvent handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Event obj");
    vk::DestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, EventStageMaskOneCommandBufferPass) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj commandBuffer1(m_device, m_commandPool);
    VkCommandBufferObj commandBuffer2(m_device, m_commandPool);

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    vk::CmdWaitEvents(commandBuffer1.handle(), 1, &event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer1.end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer1.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, EventStageMaskOneCommandBufferFail) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj commandBuffer1(m_device, m_commandPool);
    VkCommandBufferObj commandBuffer2(m_device, m_commandPool);

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    // wrong srcStageMask
    vk::CmdWaitEvents(commandBuffer1.handle(), 1, &event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer1.end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer1.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-parameter");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, EventStageMaskTwoCommandBufferPass) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj commandBuffer1(m_device, m_commandPool);
    VkCommandBufferObj commandBuffer2(m_device, m_commandPool);

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    commandBuffer1.end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer1.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    commandBuffer2.begin();
    vk::CmdWaitEvents(commandBuffer2.handle(), 1, &event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer2.end();

    submit_info.pCommandBuffers = &commandBuffer2.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, EventStageMaskTwoCommandBufferFail) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj commandBuffer1(m_device, m_commandPool);
    VkCommandBufferObj commandBuffer2(m_device, m_commandPool);

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    commandBuffer1.end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer1.handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    commandBuffer2.begin();
    // wrong srcStageMask
    vk::CmdWaitEvents(commandBuffer2.handle(), 1, &event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer2.end();

    submit_info.pCommandBuffers = &commandBuffer2.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-parameter");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, QueryPoolPartialTimestamp) {
    TEST_DESCRIPTION("Request partial result on timestamp query.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    VkBufferObj buffer;
    buffer.init(*m_device, 128, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    // Use setup as a positive test...
    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-00827");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 0, 8, VK_QUERY_RESULT_PARTIAL_BIT);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    // Submit cmd buffer and wait for it.
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    // Attempt to obtain partial results.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-00818");
    uint32_t data_space[16];
    m_errorMonitor->SetUnexpectedError("Cannot get query results on queryPool");
    vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space), &data_space, sizeof(uint32_t),
                            VK_QUERY_RESULT_PARTIAL_BIT);
    m_errorMonitor->VerifyFound();

    // Destroy query pool.
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerTest, PerformanceQueryIntel) {
    TEST_DESCRIPTION("Call CmdCopyQueryPoolResults for an Intel performance query.");

    AddRequiredExtensions(VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto performance_api_info_intel = LvlInitStruct<VkInitializePerformanceApiInfoINTEL>();
    PFN_vkInitializePerformanceApiINTEL vkInitializePerformanceApiINTEL =
        (PFN_vkInitializePerformanceApiINTEL)vk::GetDeviceProcAddr(m_device->device(), "vkInitializePerformanceApiINTEL");
    vkInitializePerformanceApiINTEL(m_device->device(), &performance_api_info_intel);

    VkBufferObj buffer;
    buffer.init(*m_device, 128, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-02734");
    m_commandBuffer->begin();
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 0, 8, 0);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();

    // Destroy query pool.
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerTest, QueryPoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use query pool.");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_commandBuffer->begin();
    // Use query pool to create binding with cmd buffer
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
    m_commandBuffer->end();

    // Submit cmd buffer and then destroy query pool while in-flight
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyQueryPool-queryPool-00793");
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
    // Now that cmd buffer done we can safely destroy query_pool
    m_errorMonitor->SetUnexpectedError("If queryPool is not VK_NULL_HANDLE, queryPool must be a valid VkQueryPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove QueryPool obj");
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerTest, PipelineInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use pipeline.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkPipelineLayoutObj pipeline_layout(m_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyPipeline-pipeline-00765");
    // Create PSO to be used for draw-time errors below

    // Store pipeline handle so we can actually delete it before test finishes
    VkPipeline delete_this_pipeline;
    {  // Scope pipeline so it will be auto-deleted
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        delete_this_pipeline = pipe.pipeline_;

        m_commandBuffer->begin();
        // Bind pipeline to cmd buffer
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        // Submit cmd buffer and then pipeline destroyed while in-flight
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }  // Pipeline deletion triggered here
    m_errorMonitor->VerifyFound();
    // Make sure queue finished and then actually delete pipeline
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If pipeline is not VK_NULL_HANDLE, pipeline must be a valid VkPipeline handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Pipeline obj");
    vk::DestroyPipeline(m_device->handle(), delete_this_pipeline, nullptr);
}

TEST_F(VkLayerTest, ImageViewInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use imageView.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;

    VkResult err;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    // Create PSO to use the sampler
    VkShaderObj fs(this, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyImageView-imageView-01026");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    // Bind pipeline to cmd buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer then destroy sampler
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy imageView while in-flight
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::DestroyImageView(m_device->device(), view, nullptr);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
    // Now we can actually destroy imageView
    m_errorMonitor->SetUnexpectedError("If imageView is not VK_NULL_HANDLE, imageView must be a valid VkImageView handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove ImageView obj");
    vk::DestroySampler(m_device->device(), sampler, nullptr);
}

TEST_F(VkLayerTest, BufferViewInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use bufferView.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkBufferView view;
    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>();
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;

    VkResult err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
    ASSERT_VK_SUCCESS(err);

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0, r32f) uniform readonly imageBuffer s;
        layout(location=0) out vec4 x;
        void main(){
           x = imageLoad(s, 0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    err = pipe.CreateGraphicsPipeline();
    if (err != VK_SUCCESS) {
        GTEST_SKIP() << "Unable to compile shader";
    }

    pipe.descriptor_set_->WriteDescriptorBufferView(0, view, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBufferView-bufferView-00936");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    // Bind pipeline to cmd buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy bufferView while in-flight
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::DestroyBufferView(m_device->device(), view, nullptr);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
    // Now we can actually destroy bufferView
    m_errorMonitor->SetUnexpectedError("If bufferView is not VK_NULL_HANDLE, bufferView must be a valid VkBufferView handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove BufferView obj");
    vk::DestroyBufferView(m_device->device(), view, NULL);
}

TEST_F(VkLayerTest, SamplerInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use sampler.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;

    VkResult err;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    // Create PSO to use the sampler
    VkShaderObj fs(this, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroySampler-sampler-01082");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    // Bind pipeline to cmd buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer then destroy sampler
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy sampler while in-flight
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::DestroySampler(m_device->device(), sampler, nullptr);  // Destroyed too soon
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);

    // Now we can actually destroy sampler
    m_errorMonitor->SetUnexpectedError("If sampler is not VK_NULL_HANDLE, sampler must be a valid VkSampler handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Sampler obj");
    vk::DestroySampler(m_device->device(), sampler, NULL);  // Destroyed for real
}

TEST_F(VkLayerTest, QueueForwardProgressFenceWait) {
    TEST_DESCRIPTION("Call VkQueueSubmit with a semaphore that is already signaled but not waited on by the queue.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char *queue_forward_progress_message = "UNASSIGNED-CoreValidation-DrawState-QueueForwardProgress";

    VkCommandBufferObj cb1(m_device, m_commandPool);
    cb1.begin();
    cb1.end();

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cb1.handle();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_commandBuffer->begin();
    m_commandBuffer->end();
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, queue_forward_progress_message);
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DeviceWaitIdle(m_device->device());
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
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
    VkEvent event;
    VkResult err;

    err = vk::CreateEvent(device(), &event_info, NULL, &event);
    ASSERT_VK_SUCCESS(err);

    err = vk::ResetEvent(device(), event);
    ASSERT_VK_SUCCESS(err);

    ThreadTestData data;
    data.commandBuffer = commandBuffer.handle();
    data.event = event;
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

    vk::DestroyEvent(device(), event, NULL);
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

    VkBufferObj buffer;
    buffer.init(*m_device, 256, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

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

    VkBufferObj buffer;
    buffer.init(*m_device, 256, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

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

TEST_F(VkLayerTest, HostQueryResetNotEnabled) {
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT without enabling the feature");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-None-02665");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, HostQueryResetBadFirstQuery) {
    TEST_DESCRIPTION("Bad firstQuery in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features =
        LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&host_query_reset_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-firstQuery-02666");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 1, 0);
    m_errorMonitor->VerifyFound();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
        auto fpvkResetQueryPool = (PFN_vkResetQueryPool)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPool");
        if (nullptr == fpvkResetQueryPool) {
            m_errorMonitor->SetError("No ProcAddr for 1.2 core vkResetQueryPool");
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-firstQuery-02666");
            fpvkResetQueryPool(m_device->device(), query_pool, 1, 0);
            m_errorMonitor->VerifyFound();
        }
    }

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, HostQueryResetBadRange) {
    TEST_DESCRIPTION("Bad range in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features =
        LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&host_query_reset_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-firstQuery-02667");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 2);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, HostQueryResetInvalidQueryPool) {
    TEST_DESCRIPTION("Invalid queryPool in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features =
        LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&host_query_reset_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    // Create and destroy a query pool.
    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);

    // Attempt to reuse the query pool handle.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-queryPool-parameter");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, HostQueryResetWrongDevice) {
    TEST_DESCRIPTION("Device not matching queryPool in vkResetQueryPoolEXT");

    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features =
        LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&host_query_reset_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    // Create a second device with the feature enabled.
    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    auto features = m_device->phy().features();

    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&host_query_reset_features);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.pEnabledFeatures = &features;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice second_device;
    ASSERT_VK_SUCCESS(vk::CreateDevice(gpu(), &device_create_info, nullptr, &second_device));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetQueryPool-queryPool-parent");
    // Run vk::ResetQueryPoolExt on the wrong device.
    fpvkResetQueryPoolEXT(second_device, query_pool, 0, 1);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
    vk::DestroyDevice(second_device, nullptr);
}

TEST_F(VkLayerTest, ResetEventThenSet) {
    TEST_DESCRIPTION("Reset an event then set it after the reset has been submitted.");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        vk::BeginCommandBuffer(command_buffer, &begin_info);

        vk::CmdResetEvent(command_buffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
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
        vk::SetEvent(m_device->device(), event);
        m_errorMonitor->VerifyFound();
    }

    vk::QueueWaitIdle(queue);

    vk::DestroyEvent(m_device->device(), event, nullptr);
    vk::FreeCommandBuffers(m_device->device(), command_pool, 1, &command_buffer);
    vk::DestroyCommandPool(m_device->device(), command_pool, NULL);
}

TEST_F(VkLayerTest, ShadingRateImageNV) {
    TEST_DESCRIPTION("Test VK_NV_shading_rate_image.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // Create a device that enables shading_rate_image but disables multiViewport
    auto shading_rate_image_features = LvlInitStruct<VkPhysicalDeviceShadingRateImageFeaturesNV>();
    auto features2 = GetPhysicalDeviceFeatures2(shading_rate_image_features);
    if (!shading_rate_image_features.shadingRateImage) {
        GTEST_SKIP() << "shadingRateImage not supported";
    }

    features2.features.multiViewport = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test shading rate image creation
    VkResult result = VK_RESULT_MAX_ENUM;
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8_UINT;
    image_create_info.extent.width = 4;
    image_create_info.extent.height = 4;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

    // image type must be 2D
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-02082");

    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.arrayLayers = 6;

    // must be single sample
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-samples-02083");

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    // tiling must be optimal
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-shadingRateImage-07727");

    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    // Should succeed.
    VkImageObj image(m_device);
    image.init(&image_create_info);

    // Test image view creation
    VkImageView view;
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8_UINT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // view type must be 2D or 2D_ARRAY
    ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    ivci.subresourceRange.layerCount = 6;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02086");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-01003");
    result = vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == result) {
        vk::DestroyImageView(m_device->device(), view, NULL);
        view = VK_NULL_HANDLE;
    }
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.subresourceRange.layerCount = 1;

    // format must be R8_UINT
    ivci.format = VK_FORMAT_R8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02087");
    result = vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == result) {
        vk::DestroyImageView(m_device->device(), view, NULL);
        view = VK_NULL_HANDLE;
    }
    ivci.format = VK_FORMAT_R8_UINT;

    vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);

    // Test pipeline creation
    VkPipelineViewportShadingRateImageStateCreateInfoNV vsrisci = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV};

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkViewport viewports[20] = {viewport, viewport};
    VkRect2D scissor = {{0, 0}, {64, 64}};
    VkRect2D scissors[20] = {scissor, scissor};
    VkDynamicState dynPalette = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
    VkPipelineDynamicStateCreateInfo dyn = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 1, &dynPalette};

    // viewportCount must be 0 or 1 when multiViewport is disabled
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 2;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 2;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;
            helper.dyn_state_ci_ = dyn;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 2;
        };
        CreatePipelineHelper::OneshotTest(
            *this, break_vp, kErrorBit,
            vector<std::string>({"VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02054",
                                 "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
                                 "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}));
    }

    // viewportCounts must match
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 1;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 1;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;
            helper.dyn_state_ci_ = dyn;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 0;
        };
        CreatePipelineHelper::OneshotTest(
            *this, break_vp, kErrorBit,
            vector<std::string>({"VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-shadingRateImageEnable-02056"}));
    }

    // pShadingRatePalettes must not be NULL.
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 1;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 1;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 1;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04057"}));
    }

    // Create an image without the SRI bit
    VkImageObj nonSRIimage(m_device);
    nonSRIimage.Init(256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(nonSRIimage.initialized());
    VkImageView nonSRIview = nonSRIimage.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    // Test SRI layout on non-SRI image
    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;
    img_barrier.image = nonSRIimage.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();

    // Error trying to convert it to SRI layout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-oldLayout-02088");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // succeed converting it to GENERAL
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    // Test vk::CmdBindShadingRateImageNV errors
    auto vkCmdBindShadingRateImageNV =
        (PFN_vkCmdBindShadingRateImageNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBindShadingRateImageNV");

    // if the view is non-NULL, it must be R8_UINT, USAGE_SRI, image layout must match, layout must be valid
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadingRateImageNV-imageView-02060");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadingRateImageNV-imageView-02061");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadingRateImageNV-imageView-02062");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindShadingRateImageNV-imageLayout-02063");
    vkCmdBindShadingRateImageNV(m_commandBuffer->handle(), nonSRIview, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    m_errorMonitor->VerifyFound();

    // Test vk::CmdSetViewportShadingRatePaletteNV errors
    auto vkCmdSetViewportShadingRatePaletteNV =
        (PFN_vkCmdSetViewportShadingRatePaletteNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetViewportShadingRatePaletteNV");

    VkShadingRatePaletteEntryNV paletteEntries[100] = {};
    VkShadingRatePaletteNV palette = {100, paletteEntries};
    VkShadingRatePaletteNV palettes[] = {palette, palette};

    // errors on firstViewport/viewportCount
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02067");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02068");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportShadingRatePaletteNV-viewportCount-02069");
    vkCmdSetViewportShadingRatePaletteNV(m_commandBuffer->handle(), 20, 2, palettes);
    m_errorMonitor->VerifyFound();

    // shadingRatePaletteEntryCount must be in range
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShadingRatePaletteNV-shadingRatePaletteEntryCount-02071");
    vkCmdSetViewportShadingRatePaletteNV(m_commandBuffer->handle(), 0, 1, palettes);
    m_errorMonitor->VerifyFound();

    VkCoarseSampleLocationNV locations[100] = {
        {0, 0, 0},    {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {0, 1, 1},  // duplicate
        {1000, 0, 0},                                              // pixelX too large
        {0, 1000, 0},                                              // pixelY too large
        {0, 0, 1000},                                              // sample too large
    };

    // Test custom sample orders, both via pipeline state and via dynamic state
    {
        VkCoarseSampleOrderCustomNV sampOrdBadShadingRate = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV, 1, 1,
                                                             locations};
        VkCoarseSampleOrderCustomNV sampOrdBadSampleCount = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 3, 1,
                                                             locations};
        VkCoarseSampleOrderCustomNV sampOrdBadSampleLocationCount = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV,
                                                                     2, 2, locations};
        VkCoarseSampleOrderCustomNV sampOrdDuplicateLocations = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2,
                                                                 1 * 2 * 2, &locations[1]};
        VkCoarseSampleOrderCustomNV sampOrdOutOfRangeLocations = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2,
                                                                  1 * 2 * 2, &locations[4]};
        VkCoarseSampleOrderCustomNV sampOrdTooLargeSampleLocationCount = {
            VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV, 4, 64, &locations[8]};
        VkCoarseSampleOrderCustomNV sampOrdGood = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2, 1 * 2 * 2,
                                                   &locations[0]};

        VkPipelineViewportCoarseSampleOrderStateCreateInfoNV csosci = {
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV};
        csosci.sampleOrderType = VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV;
        csosci.customSampleOrderCount = 1;

        using std::vector;
        struct TestCase {
            const VkCoarseSampleOrderCustomNV *order;
            vector<std::string> vuids;
        };

        vector<TestCase> test_cases = {
            {&sampOrdBadShadingRate, {"VUID-VkCoarseSampleOrderCustomNV-shadingRate-02073"}},
            {&sampOrdBadSampleCount,
             {"VUID-VkCoarseSampleOrderCustomNV-sampleCount-02074", "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075"}},
            {&sampOrdBadSampleLocationCount, {"VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075"}},
            {&sampOrdDuplicateLocations, {"VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077"}},
            {&sampOrdOutOfRangeLocations,
             {"VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077", "VUID-VkCoarseSampleLocationNV-pixelX-02078",
              "VUID-VkCoarseSampleLocationNV-pixelY-02079", "VUID-VkCoarseSampleLocationNV-sample-02080"}},
            {&sampOrdTooLargeSampleLocationCount,
             {"VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02076",
              "VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077"}},
            {&sampOrdGood, {}},
        };

        for (const auto &test_case : test_cases) {
            const auto break_vp = [&](CreatePipelineHelper &helper) {
                helper.vp_state_ci_.pNext = &csosci;
                csosci.pCustomSampleOrders = test_case.order;
            };
            CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, test_case.vuids);
        }

        // Test vk::CmdSetCoarseSampleOrderNV errors
        auto vkCmdSetCoarseSampleOrderNV =
            (PFN_vkCmdSetCoarseSampleOrderNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetCoarseSampleOrderNV");

        for (const auto &test_case : test_cases) {
            for (uint32_t i = 0; i < test_case.vuids.size(); ++i) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.vuids[i]);
            }
            vkCmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV, 1, test_case.order);
            if (test_case.vuids.size()) {
                m_errorMonitor->VerifyFound();
            } else {
            }
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetCoarseSampleOrderNV-sampleOrderType-02081");
        vkCmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV, 1, &sampOrdGood);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();

    vk::DestroyImageView(m_device->device(), view, NULL);
}

TEST_F(VkLayerTest, ValidateStride) {
    TEST_DESCRIPTION("Validate Stride.");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (IsPlatform(kPixelC)) {
        GTEST_SKIP() << "This test should not run on Pixel C";
    }

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << " Device graphic queue has timestampValidBits of 0, skipping.";
    }

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    char data_space;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-02827");
    vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space), &data_space, 1, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-00815");
    vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space), &data_space, 1,
                            (VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT));
    m_errorMonitor->VerifyFound();

    char data_space4[4] = "";
    vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space4), &data_space4, 4, VK_QUERY_RESULT_WAIT_BIT);

    char data_space8[8] = "";
    vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space8), &data_space8, 8,
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
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 1, 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-flags-00823");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 1, 1, VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 4, 4, 0);

    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 8, 8, VK_QUERY_RESULT_64_BIT);

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
        if (draw_count != UINT32_MAX) {
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
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerTest, QueryPerformanceCreation) {
    TEST_DESCRIPTION("Create performance query without support");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto performance_features = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&performance_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performance_features.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performance_features));
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;

    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;
        break;
    }

    if (counters.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter.";
    }

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counters.size();
    std::vector<uint32_t> counterIndices;
    for (uint32_t c = 0; c < counters.size(); c++) counterIndices.push_back(c);
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    // Missing pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolCreateInfo-queryType-03222");
    VkQueryPool query_pool;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    query_pool_ci.pNext = &perf_query_pool_ci;

    // Invalid counter indices
    counterIndices.push_back(counters.size());
    perf_query_pool_ci.counterIndexCount++;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkQueryPoolPerformanceCreateInfoKHR-pCounterIndices-03321");
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
    perf_query_pool_ci.counterIndexCount--;
    counterIndices.pop_back();

    // Success
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_commandBuffer->begin();

    // Missing acquire lock
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-03223");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerTest, QueryPerformanceCounterCommandbufferScope) {
    TEST_DESCRIPTION("Insert a performance query begin/end with respect to the command buffer counter scope");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));


    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto performanceFeatures = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performanceFeatures, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_COMMAND_BUFFER_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_BUFFER_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }

        if (counterIndices.empty()) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter with command buffer scope.";
    }

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    // Not the first command.
    {
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->begin();
        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-03224");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    // First command: success.
    {
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->begin();

        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);

        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);

        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);

        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);

    vkReleaseProfilingLockKHR(device());
}

TEST_F(VkLayerTest, QueryPerformanceCounterRenderPassScope) {
    TEST_DESCRIPTION("Insert a performance query begin/end with respect to the render pass counter scope");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto performanceFeatures = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_RENDER_PASS_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_RENDER_PASS_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }

        if (counterIndices.empty()) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter with render pass scope.";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    // Inside a render pass.
    {
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryPool-03225");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);
    }

    vkReleaseProfilingLockKHR(device());

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerTest, QueryPerformanceReleaseProfileLockBeforeSubmit) {
    TEST_DESCRIPTION("Verify that we get an error if we release the profiling lock during the recording of performance queries");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto performanceFeatures = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performanceFeatures, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }

        if (counterIndices.empty()) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported any performance counter with render pass scope.";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;

        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(queue);
    }

    {
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->reset();
        m_commandBuffer->begin();

        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);

        // Release while recording.
        vkReleaseProfilingLockKHR(device());
        {
            VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
            VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
            ASSERT_TRUE(result == VK_SUCCESS);
        }

        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);

        m_commandBuffer->end();

        VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-03220");
        vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        vk::QueueWaitIdle(queue);

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vkReleaseProfilingLockKHR(device());

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerTest, QueryPerformanceIncompletePasses) {
    TEST_DESCRIPTION("Verify that we get an error if we don't submit a command buffer for each passes before getting the results.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));


    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto hostQueryResetFeatures = LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    auto performanceFeatures = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>(&hostQueryResetFeatures);
    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }
    if (!hostQueryResetFeatures.hostQueryReset) {
        GTEST_SKIP() << "Missing host query reset.";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performanceFeatures, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR =
        (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    ASSERT_TRUE(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;
    uint32_t nPasses = 0;

    // Find all counters with VK_QUERY_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_KHR) counterIndices.push_back(counterIdx);
        }

        VkQueryPoolPerformanceCreateInfoKHR create_info = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
        create_info.queueFamilyIndex = idx;
        create_info.counterIndexCount = counterIndices.size();
        create_info.pCounterIndices = &counterIndices[0];

        vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(gpu(), &create_info, &nPasses);

        if (nPasses < 2) {
            counters.clear();
            continue;
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported a set of counters that needs more than one pass.";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);
    PFN_vkResetQueryPoolEXT fpvkResetQueryPoolEXT =
        (PFN_vkResetQueryPoolEXT)vk::GetInstanceProcAddr(instance(), "vkResetQueryPoolEXT");

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        VkCommandBufferBeginInfo command_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);

        m_commandBuffer->begin(&command_buffer_begin_info);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);
        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
        m_commandBuffer->end();

        // Invalid pass index
        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = LvlInitStruct<VkPerformanceQuerySubmitInfoKHR>();
            perf_submit_info.counterPassIndex = nPasses;
            VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_commandBuffer->handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPerformanceQuerySubmitInfoKHR-counterPassIndex-03221");
            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }

        // Leave the last pass out.
        for (uint32_t passIdx = 0; passIdx < (nPasses - 1); passIdx++) {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = LvlInitStruct<VkPerformanceQuerySubmitInfoKHR>();
            perf_submit_info.counterPassIndex = passIdx;
            VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_commandBuffer->handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        }

        vk::QueueWaitIdle(queue);

        std::vector<VkPerformanceCounterResultKHR> results;
        results.resize(counterIndices.size());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03231");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WAIT_BIT);
        m_errorMonitor->VerifyFound();

        // submit the last pass
        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = LvlInitStruct<VkPerformanceQuerySubmitInfoKHR>();
            perf_submit_info.counterPassIndex = nPasses - 1;
            VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_commandBuffer->handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        }

        vk::QueueWaitIdle(queue);

        // The stride is too small to return the data
        if (counterIndices.size() > 2) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-04519");
            vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(),
                                    &results[0], sizeof(VkPerformanceCounterResultKHR) * (results.size() - 1), 0);
            m_errorMonitor->VerifyFound();
        }


        // Invalid stride
        {
            std::vector<VkPerformanceCounterResultKHR> results_invalid_stride;
            results_invalid_stride.resize(counterIndices.size() * 2);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03229");
            vk::GetQueryPoolResults(
                device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results_invalid_stride.size(),
                &results_invalid_stride[0], sizeof(VkPerformanceCounterResultKHR) * results_invalid_stride.size() + 4,
                VK_QUERY_RESULT_WAIT_BIT);
            m_errorMonitor->VerifyFound();
        }

        // Invalid flags
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_PARTIAL_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_64_BIT);
        m_errorMonitor->VerifyFound();

        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) * results.size(), VK_QUERY_RESULT_WAIT_BIT);

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vkReleaseProfilingLockKHR(device());

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerTest, QueryPerformanceResetAndBegin) {
    TEST_DESCRIPTION("Verify that we get an error if we reset & begin a performance query within the same primary command buffer.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto hostQueryResetFeatures = LvlInitStruct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    auto performanceFeatures = LvlInitStruct<VkPhysicalDevicePerformanceQueryFeaturesKHR>(&hostQueryResetFeatures);
    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        GTEST_SKIP() << "Performance query pools are not supported.";
    }
    if (!hostQueryResetFeatures.hostQueryReset) {
        GTEST_SKIP() << "Missing host query reset.";
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performanceFeatures, pool_flags));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk::GetInstanceProcAddr(
                instance(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_TRUE(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != nullptr);

    PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR =
        (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    ASSERT_TRUE(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR != nullptr);

    auto queueFamilyProperties = m_device->phy().queue_properties();
    uint32_t queueFamilyIndex = queueFamilyProperties.size();
    std::vector<VkPerformanceCounterKHR> counters;
    std::vector<uint32_t> counterIndices;

    // Find a single counter with VK_QUERY_SCOPE_COMMAND_KHR scope.
    for (uint32_t idx = 0; idx < queueFamilyProperties.size(); idx++) {
        uint32_t nCounters;

        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, nullptr, nullptr);
        if (nCounters == 0) continue;

        counters.resize(nCounters);
        for (auto &c : counters) {
            c = LvlInitStruct<VkPerformanceCounterKHR>();
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_KHR) {
                counterIndices.push_back(counterIdx);
                break;
            }
        }
        break;
    }

    if (counterIndices.empty()) {
        GTEST_SKIP() << "No queue reported a set of counters that needs more than one pass.";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci = LvlInitStruct<VkQueryPoolPerformanceCreateInfoKHR>();
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>(&perf_query_pool_ci);
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), queueFamilyIndex, 0, &queue);

    PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR =
        (PFN_vkAcquireProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkAcquireProfilingLockKHR");
    ASSERT_TRUE(vkAcquireProfilingLockKHR != nullptr);
    PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR =
        (PFN_vkReleaseProfilingLockKHR)vk::GetInstanceProcAddr(instance(), "vkReleaseProfilingLockKHR");
    ASSERT_TRUE(vkReleaseProfilingLockKHR != nullptr);

    {
        VkAcquireProfilingLockInfoKHR lock_info = LvlInitStruct<VkAcquireProfilingLockInfoKHR>();
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        VkCommandBufferBeginInfo command_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBeginQuery-None-02863");

        m_commandBuffer->reset();
        m_commandBuffer->begin(&command_buffer_begin_info);
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
        m_commandBuffer->end();

        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info = LvlInitStruct<VkPerformanceQuerySubmitInfoKHR>();
            perf_submit_info.counterPassIndex = 0;
            VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&perf_submit_info);
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = NULL;
            submit_info.pWaitDstStageMask = NULL;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &m_commandBuffer->handle();
            submit_info.signalSemaphoreCount = 0;
            submit_info.pSignalSemaphores = NULL;

            vk::QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        }

        vk::QueueWaitIdle(queue);
        m_errorMonitor->VerifyFound();

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vkReleaseProfilingLockKHR(device());

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerTest, QueueSubmitNoTimelineSemaphoreInfo) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore but not a VkTimelineSemaphoreSubmitInfoKHR.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        GTEST_SKIP() << "Timeline semaphore not supported";
    }

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info[2] = {};
    submit_info[0] = LvlInitStruct<VkSubmitInfo>();
    submit_info[0].commandBufferCount = 0;
    submit_info[0].pWaitDstStageMask = &stageFlags;
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitSemaphores-03239");
    vk::QueueSubmit(m_device->m_queue, 1, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
    uint64_t signalValue = 1;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    submit_info[0].pNext = &timeline_semaphore_submit_info;

    submit_info[1] = LvlInitStruct<VkSubmitInfo>();
    submit_info[1].commandBufferCount = 0;
    submit_info[1].pWaitDstStageMask = &stageFlags;
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &semaphore;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitSemaphores-03239");
    vk::QueueSubmit(m_device->m_queue, 2, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
}

TEST_F(VkLayerTest, QueueSubmitTimelineSemaphoreBadValue) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore using a wrong payload value.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        GTEST_SKIP() << "Timeline semaphore not supported, skipping test";
    }

    auto vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto timelineproperties = LvlInitStruct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);
    auto vkSignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)vk::GetDeviceProcAddr(m_device->device(), "vkSignalSemaphoreKHR");

    auto semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    auto timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
    uint64_t signalValue = 1;
    uint64_t waitValue = 3;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    auto submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_semaphore_submit_info);
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-03241");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-03240");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signalValue = 5;
    {
        auto semaphore_signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
        semaphore_signal_info.semaphore = semaphore.handle();
        semaphore_signal_info.value = signalValue;
        ASSERT_VK_SUCCESS(vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));
    }

    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;

    // Check for re-signalling an already completed value (5)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03242");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Submit (6)
    signalValue++;
    auto err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Check against a pending value (6)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03242");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    {
        // Double signal with the same value (7)
        signalValue++;
        uint64_t signal_values[2] = {signalValue, signalValue};
        VkSemaphore signal_sems[2] = {semaphore.handle(), semaphore.handle()};

        auto tl_info_2 = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
        tl_info_2.signalSemaphoreValueCount = 2;
        tl_info_2.pSignalSemaphoreValues = signal_values;

        auto submit_info2 = LvlInitStruct<VkSubmitInfo>(&tl_info_2);
        submit_info2.signalSemaphoreCount = 2;
        submit_info2.pSignalSemaphores = signal_sems;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03242");
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info2, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < UINT64_MAX) {
        uint64_t bigValue = signalValue + timelineproperties.maxTimelineSemaphoreValueDifference + 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &bigValue;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03244");
        vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        if (signalValue < UINT64_MAX) {
            signalValue++;
            timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
            waitValue = signalValue + timelineproperties.maxTimelineSemaphoreValueDifference + 1;

            submit_info.signalSemaphoreCount = 0;
            timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
            timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitSemaphores-03243");
            vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }
    }
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkLayerTest, QueueBindSparseTimelineSemaphoreBadValue) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore using a wrong payload value.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        GTEST_SKIP() << "Timeline semaphore not supported, skipping test";
    }
    auto index = m_device->graphics_queue_node_index_;
    if ((m_device->queue_props[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == 0) {
        GTEST_SKIP() << "Sparse binding not supported, skipping test";
    }

    auto vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto timelineproperties = LvlInitStruct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);
    auto vkSignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)vk::GetDeviceProcAddr(m_device->device(), "vkSignalSemaphoreKHR");

    auto semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    auto timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
    uint64_t signalValue = 1;
    uint64_t waitValue = 3;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

    auto submit_info = LvlInitStruct<VkBindSparseInfo>();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore.handle();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();

    // error for both signal and wait
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pWaitSemaphores-03246");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pWaitSemaphores-03246");
    vk::QueueBindSparse(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    submit_info.pNext = &timeline_semaphore_submit_info;

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pNext-03248");
    vk::QueueBindSparse(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    submit_info.signalSemaphoreCount = 1;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    submit_info.waitSemaphoreCount = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pNext-03247");
    vk::QueueBindSparse(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signalValue = 5;
    {
        auto semaphore_signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
        semaphore_signal_info.semaphore = semaphore.handle();
        semaphore_signal_info.value = signalValue;
        ASSERT_VK_SUCCESS(vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));
    }

    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    submit_info.waitSemaphoreCount = 1;

    // Check for re-signalling an already completed value (5)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pSignalSemaphores-03249");
    vk::QueueBindSparse(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Submit (6)
    signalValue++;
    auto err = vk::QueueBindSparse(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Check against a pending value (6)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pSignalSemaphores-03249");
    vk::QueueBindSparse(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    {
        // Double signal with the same value (7)
        signalValue++;
        uint64_t signal_values[2] = {signalValue, signalValue};
        VkSemaphore signal_sems[2] = {semaphore.handle(), semaphore.handle()};

        auto tl_info_2 = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
        tl_info_2.signalSemaphoreValueCount = 2;
        tl_info_2.pSignalSemaphoreValues = signal_values;

        auto submit_info2 = LvlInitStruct<VkBindSparseInfo>(&tl_info_2);
        submit_info2.signalSemaphoreCount = 2;
        submit_info2.pSignalSemaphores = signal_sems;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pSignalSemaphores-03249");
        vk::QueueBindSparse(m_device->m_queue, 1, &submit_info2, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < UINT64_MAX) {
        uint64_t bigValue = signalValue + timelineproperties.maxTimelineSemaphoreValueDifference + 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &bigValue;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pSignalSemaphores-03251");
        vk::QueueBindSparse(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        if (signalValue < UINT64_MAX) {
            waitValue = bigValue;

            submit_info.signalSemaphoreCount = 0;
            submit_info.waitSemaphoreCount = 1;
            timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
            timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindSparseInfo-pWaitSemaphores-03250");
            vk::QueueBindSparse(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }
    }
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkLayerTest, Sync2QueueSubmitTimelineSemaphoreBadValue) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore using a wrong payload value.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.2.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto vk12_features = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>(&vk12_features);
    auto features2 = GetPhysicalDeviceFeatures2(sync2_features);
    if (!sync2_features.synchronization2) {
        GTEST_SKIP() << "VkPhysicalDeviceSynchronization2FeaturesKHR::synchronization2 required";
    }
    if (!vk12_features.timelineSemaphore) {
        GTEST_SKIP() << "VkPhysicalDeviceVulkan12Features::timelineSemaphore required";
    }
    InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    auto fpQueueSubmit2KHR =
        reinterpret_cast<PFN_vkQueueSubmit2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2KHR"));

    auto timelineproperties = LvlInitStruct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    GetPhysicalDeviceProperties2(prop2);

    auto semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);

    auto signal_sem_info = LvlInitStruct<VkSemaphoreSubmitInfo>();
    signal_sem_info.value = semaphore_type_create_info.initialValue;
    signal_sem_info.semaphore = semaphore.handle();
    signal_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    auto submit_info = LvlInitStruct<VkSubmitInfo2>();
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_sem_info;

    // Check for re-signalling an already completed value (5)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03882");
    fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Submit (6)
    signal_sem_info.value++;
    auto err = fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(err);

    // Check against a pending value (6)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03882");
    fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    {
        // Double signal with the same value (7)
        signal_sem_info.value++;
        VkSemaphoreSubmitInfo double_signal_info[2];
        double_signal_info[0] = signal_sem_info;
        double_signal_info[1] = signal_sem_info;

        submit_info.signalSemaphoreInfoCount = 2;
        submit_info.pSignalSemaphoreInfos = double_signal_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03882");
        fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < UINT64_MAX) {
        signal_sem_info.value += timelineproperties.maxTimelineSemaphoreValueDifference + 1;

        submit_info.waitSemaphoreInfoCount = 0;
        submit_info.signalSemaphoreInfoCount = 1;
        submit_info.pSignalSemaphoreInfos = &signal_sem_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03884");
        fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        if (signal_sem_info.value < UINT64_MAX) {
            auto wait_sem_info = LvlInitStruct<VkSemaphoreSubmitInfo>();
            wait_sem_info.semaphore = semaphore.handle();
            wait_sem_info.value = signal_sem_info.value + 1;
            wait_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

            signal_sem_info.value = 1;

            submit_info.signalSemaphoreInfoCount = 0;
            submit_info.waitSemaphoreInfoCount = 1;
            submit_info.pWaitSemaphoreInfos = &wait_sem_info;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03883");
            fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }
    }
    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkLayerTest, QueueSubmitBinarySemaphoreNotSignaled) {
    TEST_DESCRIPTION("Submit a queue with a waiting binary semaphore not previously signaled.");

    AddOptionalExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    auto timeline_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeaturesKHR>();
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>(&timeline_features);
    auto features2 = GetPhysicalDeviceFeatures2(sync2_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    // VUIDs reported change if the extension is enabled, even if the timelineSemaphore feature isn't supported.
    const bool has_timeline_sem_ext = DeviceExtensionEnabled(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    {
        vk_testing::Semaphore semaphore[3];
        semaphore[0].init(*m_device, semaphore_create_info);
        semaphore[1].init(*m_device, semaphore_create_info);
        semaphore[2].init(*m_device, semaphore_create_info);

        const char *expected_vuid =
            has_timeline_sem_ext ? "VUID-vkQueueSubmit-pWaitSemaphores-03238" : "VUID-vkQueueSubmit-pWaitSemaphores-00069";

        VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkSubmitInfo submit_info[3] = {};
        submit_info[0] = LvlInitStruct<VkSubmitInfo>();
        submit_info[0].pWaitDstStageMask = &stage_flags;
        submit_info[0].waitSemaphoreCount = 1;
        submit_info[0].pWaitSemaphores = &semaphore[0].handle();
        submit_info[0].signalSemaphoreCount = 1;
        submit_info[0].pSignalSemaphores = &semaphore[1].handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, expected_vuid);
        vk::QueueSubmit(m_device->m_queue, 1, submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[1] = LvlInitStruct<VkSubmitInfo>();
        submit_info[1].pWaitDstStageMask = &stage_flags;
        submit_info[1].waitSemaphoreCount = 1;
        submit_info[1].pWaitSemaphores = &semaphore[1].handle();
        submit_info[1].signalSemaphoreCount = 1;
        submit_info[1].pSignalSemaphores = &semaphore[2].handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, expected_vuid);
        vk::QueueSubmit(m_device->m_queue, 2, &submit_info[0], VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[2] = LvlInitStruct<VkSubmitInfo>();
        submit_info[2].signalSemaphoreCount = 1;
        submit_info[2].pSignalSemaphores = &semaphore[0].handle();

        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info[2], VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 2, submit_info, VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    }
    if (m_device->queue_props[m_device->m_queue_obj->get_family_index()].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        vk_testing::Semaphore semaphore[3];
        semaphore[0].init(*m_device, semaphore_create_info);
        semaphore[1].init(*m_device, semaphore_create_info);
        semaphore[2].init(*m_device, semaphore_create_info);

        const char *expected_vuid =
            has_timeline_sem_ext ? "VUID-vkQueueBindSparse-pWaitSemaphores-03245" : "VUID-vkQueueBindSparse-pWaitSemaphores-01117";

        VkBindSparseInfo bind_info[3] = {};

        bind_info[0] = LvlInitStruct<VkBindSparseInfo>();
        bind_info[0].waitSemaphoreCount = 1;
        bind_info[0].pWaitSemaphores = &semaphore[0].handle();
        bind_info[0].signalSemaphoreCount = 1;
        bind_info[0].pSignalSemaphores = &semaphore[1].handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, expected_vuid);
        vk::QueueBindSparse(m_device->m_queue, 1, &bind_info[0], VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        bind_info[1] = LvlInitStruct<VkBindSparseInfo>();
        bind_info[1].waitSemaphoreCount = 1;
        bind_info[1].pWaitSemaphores = &semaphore[1].handle();
        bind_info[1].signalSemaphoreCount = 1;
        bind_info[1].pSignalSemaphores = &semaphore[2].handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, expected_vuid);
        vk::QueueBindSparse(m_device->m_queue, 2, bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        bind_info[2] = LvlInitStruct<VkBindSparseInfo>();
        bind_info[2].signalSemaphoreCount = 1;
        bind_info[2].pSignalSemaphores = &semaphore[0].handle();

        ASSERT_VK_SUCCESS(vk::QueueBindSparse(m_device->m_queue, 1, &bind_info[2], VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueBindSparse(m_device->m_queue, 2, bind_info, VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    }
    if (sync2_features.synchronization2) {
        vk_testing::Semaphore semaphore[3];
        semaphore[0].init(*m_device, semaphore_create_info);
        semaphore[1].init(*m_device, semaphore_create_info);
        semaphore[2].init(*m_device, semaphore_create_info);

        // the odds of having sync2 but not timeline semaphores are low, but the 'old' vuid is defined...
        const char *expected_vuid =
            has_timeline_sem_ext ? "VUID-vkQueueSubmit2-semaphore-03873" : "VUID-vkQueueSubmit2-semaphore-03872";

        auto vkQueueSubmit2KHR =
            reinterpret_cast<PFN_vkQueueSubmit2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2KHR"));

        VkSemaphoreSubmitInfo sem_info[3];
        for (int i = 0; i < 3; i++) {
            sem_info[i] = LvlInitStruct<VkSemaphoreSubmitInfo>();
            sem_info[i].semaphore = semaphore[i].handle();
            sem_info[i].stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }

        VkSubmitInfo2 submit_info[3] = {};
        submit_info[0] = LvlInitStruct<VkSubmitInfo2>();
        submit_info[0].waitSemaphoreInfoCount = 1;
        submit_info[0].pWaitSemaphoreInfos = &sem_info[0];
        submit_info[0].signalSemaphoreInfoCount = 1;
        submit_info[0].pSignalSemaphoreInfos = &sem_info[1];

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, expected_vuid);
        vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info[0], VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[1] = LvlInitStruct<VkSubmitInfo2>();
        submit_info[1].waitSemaphoreInfoCount = 1;
        submit_info[1].pWaitSemaphoreInfos = &sem_info[1];
        submit_info[1].signalSemaphoreInfoCount = 1;
        submit_info[1].pSignalSemaphoreInfos = &sem_info[2];

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, expected_vuid);
        vkQueueSubmit2KHR(m_device->m_queue, 2, submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        submit_info[2] = LvlInitStruct<VkSubmitInfo2>();
        submit_info[2].signalSemaphoreInfoCount = 1;
        submit_info[2].pSignalSemaphoreInfos = &sem_info[0];

        ASSERT_VK_SUCCESS(vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info[2], VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vkQueueSubmit2KHR(m_device->m_queue, 2, submit_info, VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    }
}

TEST_F(VkLayerTest, QueueSubmitTimelineSemaphoreOutOfOrder) {
    TEST_DESCRIPTION("Submit out-of-order timeline semaphores.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        GTEST_SKIP() << "Timeline semaphore not supported";
    }

    // We need two queues for this
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());

    uint32_t family_index[2] = {0};
    uint32_t queue_index[2] = {0};

    if (queue_count > 1) {
        family_index[1]++;
    } else {
        // If there's only one family index, check if it supports more than 1 queue
        if (queue_props[0].queueCount > 1) {
            queue_index[1]++;
        } else {
            GTEST_SKIP() << "Multiple queues are required to run this test";
        }
    }

    float priorities[] = {1.0f, 1.0f};
    VkDeviceQueueCreateInfo queue_info[2] = {};
    queue_info[0] = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_info[0].queueFamilyIndex = family_index[0];
    queue_info[0].queueCount = queue_count > 1 ? 1 : 2;
    queue_info[0].pQueuePriorities = &(priorities[0]);

    queue_info[1] = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_info[1].queueFamilyIndex = family_index[1];
    queue_info[1].queueCount = queue_count > 1 ? 1 : 2;
    queue_info[1].pQueuePriorities = &(priorities[0]);

    VkDeviceCreateInfo dev_info = LvlInitStruct<VkDeviceCreateInfo>();
    dev_info.queueCreateInfoCount = queue_count > 1 ? 2 : 1;
    dev_info.pQueueCreateInfos = &(queue_info[0]);
    dev_info.enabledLayerCount = 0;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    timeline_semaphore_features.timelineSemaphore = true;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&timeline_semaphore_features);
    dev_info.pNext = &features2;

    VkDevice dev;
    ASSERT_VK_SUCCESS(vk::CreateDevice(gpu(), &dev_info, nullptr, &dev));

    VkQueue queue[2];
    vk::GetDeviceQueue(dev, family_index[0], queue_index[0], &(queue[0]));
    vk::GetDeviceQueue(dev, family_index[1], queue_index[1], &(queue[1]));

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(dev, &semaphore_create_info, nullptr, &semaphore));

    uint64_t semaphoreValues[] = {10, 100, 0, 10};
    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &(semaphoreValues[0]);
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &(semaphoreValues[1]);

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_semaphore_submit_info);
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;

    ASSERT_VK_SUCCESS(vk::QueueSubmit(queue[0], 1, &submit_info, VK_NULL_HANDLE));

    timeline_semaphore_submit_info.pWaitSemaphoreValues = &(semaphoreValues[2]);
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &(semaphoreValues[3]);

    ASSERT_VK_SUCCESS(vk::QueueSubmit(queue[1], 1, &submit_info, VK_NULL_HANDLE));

    vk::DeviceWaitIdle(dev);
    vk::DestroySemaphore(dev, semaphore, nullptr);
    vk::DestroyDevice(dev, nullptr);
}

TEST_F(VkLayerTest, InvalidExternalSemaphore) {
    TEST_DESCRIPTION("Import and export invalid external semaphores, no queue sumbits involved.");
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
    const auto bad_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const auto other_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
    const char *bad_export_type_vuid = "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01131";
    const char *other_export_type_vuid = "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01126";
    const char *bad_import_type_vuid = "VUID-VkImportSemaphoreWin32HandleInfoKHR-handleType-01140";
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    const auto bad_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
    const auto other_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR;
    const char *bad_export_type_vuid = "VUID-VkSemaphoreGetFdInfoKHR-handleType-01136";
    const char *other_export_type_vuid = "VUID-VkSemaphoreGetFdInfoKHR-handleType-01132";
    const char *bad_import_type_vuid = "VUID-VkImportSemaphoreFdInfoKHR-handleType-01143";
#endif
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(extension_name);
    AddRequiredExtensions(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    ASSERT_NO_FATAL_FAILURE(InitState());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    // Check for external semaphore import and export capability
    auto esi = LvlInitStruct<VkPhysicalDeviceExternalSemaphoreInfoKHR>();
    esi.handleType = handle_type;

    auto esp = LvlInitStruct<VkExternalSemaphorePropertiesKHR>();

    auto vkGetPhysicalDeviceExternalSemaphorePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External semaphore does not support importing and exporting, skipping test";
    }
    // Create a semaphore to export payload from
    auto esci = LvlInitStruct<VkExportSemaphoreCreateInfoKHR>();
    esci.handleTypes = handle_type;
    auto sci = LvlInitStruct<VkSemaphoreCreateInfo>(&esci);

    vk_testing::Semaphore export_semaphore(*m_device, sci);

    // Create a semaphore for importing
    vk_testing::Semaphore import_semaphore(*m_device);

    vk_testing::Semaphore::ExternalHandle ext_handle{};

    // windows vs unix mismatch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, bad_export_type_vuid);
    export_semaphore.export_handle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();

    // not specified during create
    if (other_type == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR) {
        // SYNC_FD must have pending signal
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreGetFdInfoKHR-handleType-03254");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, other_export_type_vuid);
    export_semaphore.export_handle(ext_handle, other_type);
    m_errorMonitor->VerifyFound();

    export_semaphore.export_handle(ext_handle, handle_type);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, bad_import_type_vuid);
    export_semaphore.import_handle(ext_handle, bad_type);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidWaitSemaphoresType) {
    TEST_DESCRIPTION("Wait for a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        GTEST_SKIP() << "Timeline semaphore not supported";
    }

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    VkSemaphore semaphore[2];
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &(semaphore[0])));

    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &(semaphore[1])));

    VkSemaphoreWaitInfo semaphore_wait_info = LvlInitStruct<VkSemaphoreWaitInfo>();
    semaphore_wait_info.semaphoreCount = 2;
    semaphore_wait_info.pSemaphores = &semaphore[0];
    const uint64_t wait_values[] = {10, 40};
    semaphore_wait_info.pValues = &wait_values[0];
    auto vkWaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR)vk::GetDeviceProcAddr(m_device->device(), "vkWaitSemaphoresKHR");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreWaitInfo-pSemaphores-03256");
    vkWaitSemaphoresKHR(m_device->device(), &semaphore_wait_info, 10000);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore[0], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[1], nullptr);
}

TEST_F(VkLayerTest, InvalidSignalSemaphoreType) {
    TEST_DESCRIPTION("Signal a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto timelinefeatures = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeaturesKHR>();
    GetPhysicalDeviceFeatures2(timelinefeatures);
    if (!timelinefeatures.timelineSemaphore) {
        GTEST_SKIP() << "Timeline semaphores are not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkSemaphoreSignalInfo semaphore_signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
    semaphore_signal_info.semaphore = semaphore;
    semaphore_signal_info.value = 10;
    auto vkSignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)vk::GetDeviceProcAddr(m_device->device(), "vkSignalSemaphoreKHR");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-semaphore-03257");
    vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
}

TEST_F(VkLayerTest, InvalidSignalSemaphoreValue) {
    TEST_DESCRIPTION("Signal a Timeline Semaphore with invalid values");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        GTEST_SKIP() << "Timeline semaphore not supported";
    }

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto timelineproperties = LvlInitStruct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    VkSemaphore semaphore[2];
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[0]));
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[1]));

    VkSemaphoreSignalInfo semaphore_signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
    semaphore_signal_info.semaphore = semaphore[0];
    semaphore_signal_info.value = 3;
    auto vkSignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)vk::GetDeviceProcAddr(m_device->device(), "vkSignalSemaphoreKHR");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03258");
    vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    semaphore_signal_info.value = 10;
    ASSERT_VK_SUCCESS(vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
    uint64_t waitValue = 10;
    uint64_t signalValue = 20;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_semaphore_submit_info);
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &(semaphore[1]);
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &(semaphore[0]);
    ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));

    semaphore_signal_info.value = 25;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03259");
    vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    semaphore_signal_info.value = 15;
    ASSERT_VK_SUCCESS(vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));
    semaphore_signal_info.semaphore = semaphore[1];
    ASSERT_VK_SUCCESS(vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < UINT64_MAX) {
        VkSemaphore sem;

        semaphore_type_create_info.initialValue = 0;
        ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &sem));

        semaphore_signal_info.semaphore = sem;
        semaphore_signal_info.value = timelineproperties.maxTimelineSemaphoreValueDifference + 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03260");
        vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);
        m_errorMonitor->VerifyFound();

        semaphore_signal_info.value--;
        ASSERT_VK_SUCCESS(vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));

        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));

        vk::DestroySemaphore(m_device->device(), sem, nullptr);

        // Regression test for value difference validations ran against binary semaphores
        {
            VkSemaphore timeline_sem;
            VkSemaphore binary_sem;

            semaphore_type_create_info.initialValue = 0;
            ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &timeline_sem));

            VkSemaphoreCreateInfo binary_semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();

            ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &binary_semaphore_create_info, nullptr, &binary_sem));

            signalValue = 1;
            uint64_t offendingValue = timelineproperties.maxTimelineSemaphoreValueDifference + 1;

            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &timeline_sem;
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &binary_sem;

            timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pWaitSemaphoreValues = &signalValue;

            // These two assignments are not required by the spec, but would segfault on older versions of validation layers
            timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pSignalSemaphoreValues = &offendingValue;

            vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

            semaphore_signal_info.semaphore = timeline_sem;
            semaphore_signal_info.value = 1;
            vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);

            ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));

            vk::DestroySemaphore(m_device->device(), binary_sem, nullptr);
            vk::DestroySemaphore(m_device->device(), timeline_sem, nullptr);
        }
    }

    ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    vk::DestroySemaphore(m_device->device(), semaphore[0], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[1], nullptr);
}

TEST_F(VkLayerTest, Sync2InvalidSignalSemaphoreValue) {
    TEST_DESCRIPTION("Signal a Timeline Semaphore with invalid values");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.2.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    auto vk12_features = LvlInitStruct<VkPhysicalDeviceVulkan12Features>();
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>(&vk12_features);
    auto features2 = GetPhysicalDeviceFeatures2(sync2_features);
    if (!sync2_features.synchronization2) {
        GTEST_SKIP() << "VkPhysicalDeviceSynchronization2FeaturesKHR::synchronization2 required";
    }
    if (!vk12_features.timelineSemaphore) {
        GTEST_SKIP() << "VkPhysicalDeviceVulkan12Features::timelineSemaphore required";
    }
    InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    auto fpQueueSubmit2KHR =
        reinterpret_cast<PFN_vkQueueSubmit2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2KHR"));

    auto timelineproperties = LvlInitStruct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    GetPhysicalDeviceProperties2(prop2);

    auto semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    auto semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    vk_testing::Semaphore semaphore[2];
    semaphore[0].init(*m_device, semaphore_create_info);
    semaphore[1].init(*m_device, semaphore_create_info);

    auto semaphore_signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
    semaphore_signal_info.semaphore = semaphore[0].handle();
    semaphore_signal_info.value = 10;
    ASSERT_VK_SUCCESS(vk::SignalSemaphore(m_device->device(), &semaphore_signal_info));

    auto signal_info = LvlInitStruct<VkSemaphoreSubmitInfoKHR>();
    signal_info.semaphore = semaphore[0].handle();

    auto wait_info = LvlInitStruct<VkSemaphoreSubmitInfoKHR>();
    wait_info.semaphore = semaphore[0].handle();
    wait_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    auto submit_info = LvlInitStruct<VkSubmitInfo2KHR>();
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_info;
    submit_info.waitSemaphoreInfoCount = 1;
    submit_info.pWaitSemaphoreInfos = &wait_info;

    // signal value > wait value
    signal_info.value = 11;
    wait_info.value = 11;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03881");
    fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // signal value == current value
    signal_info.value = 10;
    wait_info.value = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2-semaphore-03882");
    fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    signal_info.value = 20;
    wait_info.semaphore = semaphore[1].handle();
    ASSERT_VK_SUCCESS(fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));

    semaphore_signal_info.value = 25;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03259");
    vk::SignalSemaphore(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    semaphore_signal_info.value = 15;
    ASSERT_VK_SUCCESS(vk::SignalSemaphore(m_device->device(), &semaphore_signal_info));
    semaphore_signal_info.semaphore = semaphore[1].handle();
    ASSERT_VK_SUCCESS(vk::SignalSemaphore(m_device->device(), &semaphore_signal_info));

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < UINT64_MAX) {
        // Regression test for value difference validations ran against binary semaphores
        semaphore_type_create_info.initialValue = 0;
        vk_testing::Semaphore timeline_sem(*m_device, semaphore_create_info);

        auto binary_semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
        vk_testing::Semaphore binary_sem(*m_device, binary_semaphore_create_info);

        wait_info.semaphore = timeline_sem.handle();
        wait_info.value = 1;

        signal_info.semaphore = binary_sem.handle();
        signal_info.value = timelineproperties.maxTimelineSemaphoreValueDifference + 1;

        fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

        semaphore_signal_info.semaphore = timeline_sem.handle();
        semaphore_signal_info.value = 1;
        vk::SignalSemaphore(m_device->device(), &semaphore_signal_info);

        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    }

    ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
}

TEST_F(VkLayerTest, InvalidSemaphoreCounterType) {
    TEST_DESCRIPTION("Get payload from a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto timelinefeatures = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeaturesKHR>();
    GetPhysicalDeviceFeatures2(timelinefeatures);
    if (!timelinefeatures.timelineSemaphore) {
        GTEST_SKIP() << "Timeline semaphores are not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    auto vkGetSemaphoreCounterValueKHR =
        (PFN_vkGetSemaphoreCounterValueKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetSemaphoreCounterValueKHR");
    uint64_t value = 0xdeadbeef;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetSemaphoreCounterValue-semaphore-03255");
    vkGetSemaphoreCounterValueKHR(m_device->device(), semaphore, &value);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
}

TEST_F(VkLayerTest, ImageDrmFormatModifer) {
    TEST_DESCRIPTION("General testing of VK_EXT_image_drm_format_modifier");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    PFN_vkGetImageDrmFormatModifierPropertiesEXT vkGetImageDrmFormatModifierPropertiesEXT =
        (PFN_vkGetImageDrmFormatModifierPropertiesEXT)vk::GetInstanceProcAddr(instance(),
                                                                              "vkGetImageDrmFormatModifierPropertiesEXT");
    ASSERT_TRUE(vkGetImageDrmFormatModifierPropertiesEXT != nullptr);

    ASSERT_NO_FATAL_FAILURE(InitState());

    constexpr std::array<uint64_t, 2> dummy_modifiers = {0, 1};

    VkImageCreateInfo image_info = LvlInitStruct<VkImageCreateInfo>();
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageFormatProperties2 image_format_prop = LvlInitStruct<VkImageFormatProperties2>();
    VkPhysicalDeviceImageFormatInfo2 image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>();
    image_format_info.format = image_info.format;
    image_format_info.tiling = image_info.tiling;
    image_format_info.type = image_info.imageType;
    image_format_info.usage = image_info.usage;
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_mod_info =
        LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
    drm_format_mod_info.drmFormatModifier = dummy_modifiers[0];
    drm_format_mod_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    drm_format_mod_info.queueFamilyIndexCount = 0;
    image_format_info.pNext = (void *)&drm_format_mod_info;
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info, &image_format_prop);

    {
        VkImageFormatProperties dummy_props;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceImageFormatProperties-tiling-02248");
        vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), image_info.format, image_info.imageType,
                                                   image_info.tiling, image_info.usage, image_info.flags, &dummy_props);
        m_errorMonitor->VerifyFound();
    }

    VkSubresourceLayout dummyPlaneLayout = {0, 0, 0, 0, 0};

    VkImageDrmFormatModifierListCreateInfoEXT drm_format_mod_list = LvlInitStruct<VkImageDrmFormatModifierListCreateInfoEXT>();
    drm_format_mod_list.drmFormatModifierCount = dummy_modifiers.size();
    drm_format_mod_list.pDrmFormatModifiers = dummy_modifiers.data();

    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_mod_explicit =
        LvlInitStruct<VkImageDrmFormatModifierExplicitCreateInfoEXT>();
    drm_format_mod_explicit.drmFormatModifierPlaneCount = 1;
    drm_format_mod_explicit.pPlaneLayouts = &dummyPlaneLayout;

    VkImage image = VK_NULL_HANDLE;

    // No pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-tiling-02261");
    vk::CreateImage(device(), &image_info, nullptr, &image);
    m_errorMonitor->VerifyFound();

    // Having wrong size, arrayPitch and depthPitch in VkSubresourceLayout
    dummyPlaneLayout.size = 1;
    dummyPlaneLayout.arrayPitch = 1;
    dummyPlaneLayout.depthPitch = 1;

    image_info.pNext = (void *)&drm_format_mod_explicit;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-size-02267");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-arrayPitch-02268");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-depthPitch-02269");
    vk::CreateImage(device(), &image_info, nullptr, &image);
    m_errorMonitor->VerifyFound();

    // reset dummy plane layout
    memset(&dummyPlaneLayout, 0, sizeof(dummyPlaneLayout));

    auto drm_format_modifier = LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
    drm_format_modifier.drmFormatModifier = dummy_modifiers[1];
    image_format_info.pNext = &drm_format_modifier;
    VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info, &image_format_prop);
    if (result == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        printf("Format VK_FORMAT_R8G8B8A8_UNORM not supported with format modifiers, Skipping the remaining tests.\n");
        return;
    }
    // Postive check if only 1
    image_info.pNext = (void *)&drm_format_mod_list;
    vk::CreateImage(device(), &image_info, nullptr, &image);
    vk::DestroyImage(device(), image, nullptr);

    image_info.pNext = (void *)&drm_format_mod_explicit;
    vk::CreateImage(device(), &image_info, nullptr, &image);
    vk::DestroyImage(device(), image, nullptr);

    // Having both in pNext
    drm_format_mod_explicit.pNext = (void *)&drm_format_mod_list;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-tiling-02261");
    vk::CreateImage(device(), &image_info, nullptr, &image);
    m_errorMonitor->VerifyFound();

    // Only 1 pNext but wrong tiling
    image_info.pNext = (void *)&drm_format_mod_list;
    image_info.tiling = VK_IMAGE_TILING_LINEAR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02262");
    vk::CreateImage(device(), &image_info, nullptr, &image);
    m_errorMonitor->VerifyFound();
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

    auto vkGetQueueCheckpointDataNV =
        (PFN_vkGetQueueCheckpointDataNV)vk::GetDeviceProcAddr(m_device->device(), "vkGetQueueCheckpointDataNV");

    auto vkCmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetCheckpointNV");

    ASSERT_TRUE(vkGetQueueCheckpointDataNV != nullptr);
    ASSERT_TRUE(vkCmdSetCheckpointNV != nullptr);

    uint32_t data = 100;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetCheckpointNV-commandBuffer-recording");
    vkCmdSetCheckpointNV(m_commandBuffer->handle(), &data);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidGetDeviceQueue) {
    TEST_DESCRIPTION("General testing of vkGetDeviceQueue and general Device creation cases");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Needed for both protected memory and vkGetDeviceQueue2
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        GTEST_SKIP() << "This test should not be run on the RADV driver.";
    }

    VkDeviceQueueInfo2 queue_info_2 = LvlInitStruct<VkDeviceQueueInfo2>();
    VkDevice test_device = VK_NULL_HANDLE;
    VkQueue test_queue = VK_NULL_HANDLE;
    VkResult result;

    // Use the first Physical device and queue family
    // Makes test more portable as every driver has at least 1 queue with a queueCount of 1
    uint32_t queue_family_count = 1;
    uint32_t queue_family_index = 0;
    VkQueueFamilyProperties queue_properties;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, &queue_properties);

    float queue_priority = 1.0;
    VkDeviceQueueCreateInfo queue_create_info = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_create_info.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceProtectedMemoryFeatures protect_features = LvlInitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
    protect_features.protectedMemory = VK_FALSE;  // Starting with it off

    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&protect_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    // Protect feature not set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueCreateInfo-flags-02861");
    vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();

    GetPhysicalDeviceFeatures2(protect_features);

    if (protect_features.protectedMemory == VK_TRUE) {
        result = vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
        if (result != VK_SUCCESS) {
            GTEST_SKIP() << "CreateDevice returned back not VK_SUCCESS";
        }

        // Try using GetDeviceQueue with a queue that has as flag
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-flags-01841");
        vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);
        m_errorMonitor->VerifyFound();

        PFN_vkGetDeviceQueue2 vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2)vk::GetDeviceProcAddr(test_device, "vkGetDeviceQueue2");
        ASSERT_TRUE(vkGetDeviceQueue2 != nullptr);

        // Test device created with flag and trying to query with no flag
        queue_info_2.flags = 0;
        queue_info_2.queueFamilyIndex = queue_family_index;
        queue_info_2.queueIndex = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueInfo2-flags-06225");
        vkGetDeviceQueue2(test_device, &queue_info_2, &test_queue);
        m_errorMonitor->VerifyFound();

        vk::DestroyDevice(test_device, nullptr);
        test_device = VK_NULL_HANDLE;
    }

    // Create device without protected queue
    protect_features.protectedMemory = VK_FALSE;
    queue_create_info.flags = 0;
    result = vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "CreateDevice returned back not VK_SUCCESS";
    }

    if (queue_properties.queueCount > 1) {
        // Set queueIndex 1 over size of queueCount used to create device
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-queueIndex-00385");
        vk::GetDeviceQueue(test_device, queue_family_index, 1, &test_queue);
        m_errorMonitor->VerifyFound();
    }

    // Use an unknown queue family index
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-queueFamilyIndex-00384");
    vk::GetDeviceQueue(test_device, queue_family_index + 1, 0, &test_queue);
    m_errorMonitor->VerifyFound();

    PFN_vkGetDeviceQueue2 vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2)vk::GetDeviceProcAddr(test_device, "vkGetDeviceQueue2");
    ASSERT_TRUE(vkGetDeviceQueue2 != nullptr);

    queue_info_2.flags = 0;  // same as device creation
    queue_info_2.queueFamilyIndex = queue_family_index;
    queue_info_2.queueIndex = 0;

    if (queue_properties.queueCount > 1) {
        // Set queueIndex 1 over size of queueCount used to create device
        queue_info_2.queueIndex = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueInfo2-queueIndex-01843");
        vkGetDeviceQueue2(test_device, &queue_info_2, &test_queue);
        m_errorMonitor->VerifyFound();
        queue_info_2.queueIndex = 0;  // reset
    }

    // Use an unknown queue family index
    queue_info_2.queueFamilyIndex = queue_family_index + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueInfo2-queueFamilyIndex-01842");
    vkGetDeviceQueue2(test_device, &queue_info_2, &test_queue);
    m_errorMonitor->VerifyFound();
    queue_info_2.queueFamilyIndex = queue_family_index;  // reset

    // Test device created with no flags and trying to query with flag
    queue_info_2.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueInfo2-flags-06225");
    vkGetDeviceQueue2(test_device, &queue_info_2, &test_queue);
    m_errorMonitor->VerifyFound();
    queue_info_2.flags = 0;  // reset

    // Sanity check can still get the queue
    vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);

    vk::DestroyDevice(test_device, nullptr);
}

TEST_F(VkLayerTest, UniqueQueueDeviceCreation) {
    TEST_DESCRIPTION("Vulkan 1.0 unique queue detection");
    SetTargetApiVersion(VK_API_VERSION_1_0);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // use first queue family with at least 2 queues in it
    bool found_queue = false;
    VkQueueFamilyProperties queue_properties;  // selected queue family used
    uint32_t queue_family_index = 0;
    uint32_t queue_family_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, queue_families.data());

    for (size_t i = 0; i < queue_families.size(); i++) {
        if (queue_families[i].queueCount > 1) {
            found_queue = true;
            queue_family_index = i;
            queue_properties = queue_families[i];
            break;
        }
    }

    if (found_queue == false) {
        GTEST_SKIP() << "test requires queue family with 2 queues, not available";
    }

    float queue_priority = 1.0;
    VkDeviceQueueCreateInfo queue_create_info[2];
    queue_create_info[0] = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_create_info[0].flags = 0;
    queue_create_info[0].queueFamilyIndex = queue_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = &queue_priority;

    // queueFamilyIndex is the same
    queue_create_info[1] = queue_create_info[0];

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>();
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.queueCreateInfoCount = 2;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    const char *vuid = (DeviceValidationVersion() == VK_API_VERSION_1_0) ? "VUID-VkDeviceCreateInfo-queueFamilyIndex-00372"
                                                                         : "VUID-VkDeviceCreateInfo-queueFamilyIndex-02802";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UniqueQueueDeviceCreationBothProtected) {
    TEST_DESCRIPTION("Vulkan 1.1 unique queue detection where both are protected and same queue family");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Needed for both protected memory and vkGetDeviceQueue2
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto protected_features = LvlInitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
    GetPhysicalDeviceFeatures2(protected_features);

    if (protected_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "test requires protectedMemory";
    }

    // Try to find a protected queue family type
    bool protected_queue = false;
    VkQueueFamilyProperties queue_properties;  // selected queue family used
    uint32_t queue_family_index = 0;
    uint32_t queue_family_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, queue_families.data());

    for (size_t i = 0; i < queue_families.size(); i++) {
        // need to have at least 2 queues to use
        if (((queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) != 0) && (queue_families[i].queueCount > 1)) {
            protected_queue = true;
            queue_family_index = i;
            queue_properties = queue_families[i];
            break;
        }
    }

    if (protected_queue == false) {
        GTEST_SKIP() << "test requires queue with VK_QUEUE_PROTECTED_BIT with 2 queues, not available";
    }

    float queue_priority = 1.0;

    VkDeviceQueueCreateInfo queue_create_info[2];
    queue_create_info[0] = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_create_info[0].flags = 0;
    queue_create_info[0].queueFamilyIndex = queue_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = &queue_priority;

    // queueFamilyIndex is the same and both are empty flags
    queue_create_info[1] = queue_create_info[0];

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&protected_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.queueCreateInfoCount = 2;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-queueFamilyIndex-02802");
    vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();

    // both protected
    queue_create_info[0].flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info[1].flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-queueFamilyIndex-02802");
    vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidProtectedQueue) {
    TEST_DESCRIPTION("Try creating queue without VK_QUEUE_PROTECTED_BIT capability");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto protected_features = LvlInitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
    GetPhysicalDeviceFeatures2(protected_features);

    if (protected_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "test requires protectedMemory";
    }

    // Try to find a protected queue family type
    bool unprotected_queue = false;
    uint32_t queue_family_index = 0;
    uint32_t queue_family_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, queue_families.data());

    // need to find a queue without protected support
    for (size_t i = 0; i < queue_families.size(); i++) {
        if ((queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) == 0) {
            unprotected_queue = true;
            queue_family_index = i;
            break;
        }
    }

    if (unprotected_queue == false) {
        GTEST_SKIP() << "test requires queue without VK_QUEUE_PROTECTED_BIT";
    }

    float queue_priority = 1.0;
    VkDeviceQueueCreateInfo queue_create_info = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_create_info.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkDevice test_device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>(&protected_features);
    device_create_info.flags = 0;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueCreateInfo-flags-06449");
    vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidProtectedSubmit) {
    TEST_DESCRIPTION("Setting protectedSubmit with a queue not created with VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    // creates a queue without VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = LvlInitStruct<VkCommandPoolCreateInfo>();
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandPoolCreateInfo-flags-02860");
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &command_pool);
    m_errorMonitor->VerifyFound();

    VkBuffer buffer;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    buffer_create_info.size = 4096;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferCreateInfo-flags-01887");
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer);
    m_errorMonitor->VerifyFound();

    VkImage image;
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    image_create_info.extent = {64, 64, 1};
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-flags-01890");
    vk::CreateImage(device(), &image_create_info, nullptr, &image);
    m_errorMonitor->VerifyFound();

    // Try to find memory with protected bit in it at all
    VkDeviceMemory memory_protected = VK_NULL_HANDLE;
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = 4096;

    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);
    alloc_info.memoryTypeIndex = phys_mem_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
        // Check just protected bit is in type at all
        if ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0) {
            alloc_info.memoryTypeIndex = i;
            break;
        }
    }
    if (alloc_info.memoryTypeIndex < phys_mem_props.memoryTypeCount) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-memoryTypeIndex-01872");
        vk::AllocateMemory(device(), &alloc_info, NULL, &memory_protected);
        m_errorMonitor->VerifyFound();
    }

    VkProtectedSubmitInfo protected_submit_info = LvlInitStruct<VkProtectedSubmitInfo>();
    protected_submit_info.protectedSubmit = VK_TRUE;

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&protected_submit_info);
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    m_commandBuffer->begin();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-queue-06448");
    m_errorMonitor->SetUnexpectedError("VUID-VkSubmitInfo-pNext-04148");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidProtectedMemory) {
    TEST_DESCRIPTION("Validate cases where protectedMemory feature is enabled and usages are invalid");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto protected_memory_features = LvlInitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(protected_memory_features);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        GTEST_SKIP() << "protectedMemory feature not supported";
    };

    // Turns m_commandBuffer into a protected command buffer
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    bool sparse_support = (m_device->phy().features().sparseBinding == VK_TRUE);

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    VkBuffer buffer_protected = VK_NULL_HANDLE;
    VkBuffer buffer_unprotected = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.size = 1 << 20;  // 1 MB
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (sparse_support == true) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferCreateInfo-None-01888");
        vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer_protected);
        m_errorMonitor->VerifyFound();
    }

    // Create actual protected and unprotected buffers
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer_protected);
    buffer_create_info.flags = 0;
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer_unprotected);

    VkImage image_protected = VK_NULL_HANDLE;
    VkImage image_unprotected = VK_NULL_HANDLE;
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    image_create_info.extent = {8, 8, 1};
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 1;

    if (sparse_support == true) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-None-01891");
        vk::CreateImage(device(), &image_create_info, nullptr, &image_protected);
        m_errorMonitor->VerifyFound();
    }

    // Create actual protected and unprotected images
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    vk::CreateImage(device(), &image_create_info, nullptr, &image_protected);
    image_create_info.flags = 0;
    vk::CreateImage(device(), &image_create_info, nullptr, &image_unprotected);

    // Create protected and unproteced memory
    VkDeviceMemory memory_protected = VK_NULL_HANDLE;
    VkDeviceMemory memory_unprotected = VK_NULL_HANDLE;

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = 0;

    // set allocationSize to buffer as it will be larger than the image, but query image to avoid BP warning
    VkMemoryRequirements mem_reqs_protected;
    vk::GetImageMemoryRequirements(device(), image_protected, &mem_reqs_protected);
    vk::GetBufferMemoryRequirements(device(), buffer_protected, &mem_reqs_protected);
    VkMemoryRequirements mem_reqs_unprotected;
    vk::GetImageMemoryRequirements(device(), image_unprotected, &mem_reqs_unprotected);
    vk::GetBufferMemoryRequirements(device(), buffer_unprotected, &mem_reqs_unprotected);

    // Get memory index for a protected and unprotected memory
    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);
    uint32_t memory_type_protected = phys_mem_props.memoryTypeCount + 1;
    uint32_t memory_type_unprotected = phys_mem_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
        if ((mem_reqs_unprotected.memoryTypeBits & (1 << i)) &&
            ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            memory_type_unprotected = i;
        }
        // Check just protected bit is in type at all
        if ((mem_reqs_protected.memoryTypeBits & (1 << i)) &&
            ((phys_mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0)) {
            memory_type_protected = i;
        }
    }
    if ((memory_type_protected >= phys_mem_props.memoryTypeCount) || (memory_type_unprotected >= phys_mem_props.memoryTypeCount)) {
        vk::DestroyImage(device(), image_protected, nullptr);
        vk::DestroyImage(device(), image_unprotected, nullptr);
        vk::DestroyBuffer(device(), buffer_protected, nullptr);
        vk::DestroyBuffer(device(), buffer_unprotected, nullptr);
        GTEST_SKIP() << "No valid memory type index could be found";
    }

    alloc_info.memoryTypeIndex = memory_type_protected;
    alloc_info.allocationSize = mem_reqs_protected.size;
    vk::AllocateMemory(device(), &alloc_info, NULL, &memory_protected);

    alloc_info.allocationSize = mem_reqs_unprotected.size;
    alloc_info.memoryTypeIndex = memory_type_unprotected;
    vk::AllocateMemory(device(), &alloc_info, NULL, &memory_unprotected);

    // Bind protected buffer with unprotected memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-None-01898");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindBufferMemory-memory-01035");
    vk::BindBufferMemory(device(), buffer_protected, memory_unprotected, 0);
    m_errorMonitor->VerifyFound();

    // Bind unprotected buffer with protected memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-None-01899");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindBufferMemory-memory-01035");
    vk::BindBufferMemory(device(), buffer_unprotected, memory_protected, 0);
    m_errorMonitor->VerifyFound();

    // Bind protected image with unprotected memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-None-01901");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-memory-01047");
    vk::BindImageMemory(device(), image_protected, memory_unprotected, 0);
    m_errorMonitor->VerifyFound();

    // Bind unprotected image with protected memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-None-01902");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-memory-01047");
    vk::BindImageMemory(device(), image_unprotected, memory_protected, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(device(), image_protected, nullptr);
    vk::DestroyImage(device(), image_unprotected, nullptr);
    vk::DestroyBuffer(device(), buffer_protected, nullptr);
    vk::DestroyBuffer(device(), buffer_unprotected, nullptr);
    vk::FreeMemory(device(), memory_protected, nullptr);
    vk::FreeMemory(device(), memory_unprotected, nullptr);
}

TEST_F(VkLayerTest, ValidateImportMemoryHandleType) {
    TEST_DESCRIPTION("Validate import memory handleType for buffers and images");

#ifdef _WIN32
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    const auto wrong_handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;

    AddRequiredExtensions(ext_mem_extension_name);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

    AddOptionalExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "External tests are not supported by MockICD, skipping tests";
    }

    auto vkGetPhysicalDeviceExternalBufferPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceExternalBufferPropertiesKHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceExternalBufferPropertiesKHR != nullptr);

    // Check for import/export capability
    // export used to feed memory to test import
    auto ebi = LvlInitStruct<VkPhysicalDeviceExternalBufferInfo>();
    ebi.handleType = handle_type;
    ebi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    auto ebp = LvlInitStruct<VkExternalBufferPropertiesKHR>();
    vkGetPhysicalDeviceExternalBufferPropertiesKHR(gpu(), &ebi, &ebp);
    if (!(ebp.externalMemoryProperties.compatibleHandleTypes & handle_type) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR)) {
        GTEST_SKIP() << "External buffer does not support importing and exporting, skipping test";
    }

    // Check if dedicated allocation is required
    const bool dedicated_allocation =
        ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT;

    if (dedicated_allocation) {
        if (!IsExtensionsEnabled(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
            GTEST_SKIP() << "Dedicated allocation extension not supported, skipping test";
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkBindBufferMemory2KHR vkBindBufferMemory2Function =
        reinterpret_cast<PFN_vkBindBufferMemory2KHR>(vk::GetDeviceProcAddr(m_device->handle(), "vkBindBufferMemory2KHR"));
    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function =
        reinterpret_cast<PFN_vkBindImageMemory2KHR>(vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR"));

    constexpr VkMemoryPropertyFlags mem_flags = 0;
    constexpr VkDeviceSize buffer_size = 1024;

    // Create export and import buffers
    auto external_buffer_info = LvlInitStruct<VkExternalMemoryBufferCreateInfo>();
    external_buffer_info.handleTypes = handle_type;

    auto buffer_info = VkBufferObj::create_info(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_info.pNext = &external_buffer_info;
    VkBufferObj buffer_export;
    buffer_export.init_no_mem(*m_device, buffer_info);
    external_buffer_info.handleTypes = wrong_handle_type;
    VkBufferObj buffer_import;
    buffer_import.init_no_mem(*m_device, buffer_info);

    // Allocation info
    auto alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer_export.memory_requirements(), mem_flags);

    // Add export allocation info to pNext chain
    auto export_info = LvlInitStruct<VkExportMemoryAllocateInfoKHR>();
    export_info.handleTypes = handle_type;

    alloc_info.pNext = &export_info;

    // Add dedicated allocation info to pNext chain if required
    auto dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    dedicated_info.buffer = buffer_export.handle();

    if (dedicated_allocation) {
        export_info.pNext = &dedicated_info;
    }

    // Allocate memory to be exported
    vk_testing::DeviceMemory memory_buffer_export;
    memory_buffer_export.init(*m_device, alloc_info);

    // Bind exported memory
    buffer_export.bind_memory(memory_buffer_export, 0);

    auto external_image_info = LvlInitStruct<VkExternalMemoryImageCreateInfoKHR>();
    external_image_info.handleTypes = handle_type;

    VkImageCreateInfo image_info = LvlInitStruct<VkImageCreateInfo>(&external_image_info);
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;
    VkImageObj image_export(m_device);
    image_export.init_no_mem(*m_device, image_info);
    external_image_info.handleTypes = wrong_handle_type;
    VkImageObj image_import(m_device);
    image_import.init_no_mem(*m_device, image_info);

    // Allocation info
    dedicated_info = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR, nullptr, image_export.handle(), VK_NULL_HANDLE};
    alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image_export.memory_requirements(), mem_flags);
    alloc_info.pNext = &export_info;

    // Allocate memory to be exported
    vk_testing::DeviceMemory memory_image_export;
    memory_image_export.init(*m_device, alloc_info);

    // Bind exported memory
    image_export.bind_memory(memory_image_export, 0);

#ifdef _WIN32
    // Export memory to handle
    auto vkGetMemoryWin32HandleKHR =
        (PFN_vkGetMemoryWin32HandleKHR)vk::GetInstanceProcAddr(instance(), "vkGetMemoryWin32HandleKHR");
    ASSERT_TRUE(vkGetMemoryWin32HandleKHR != nullptr);
    VkMemoryGetWin32HandleInfoKHR mghi_buffer = {VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR, nullptr,
                                                 memory_buffer_export.handle(), handle_type};
    VkMemoryGetWin32HandleInfoKHR mghi_image = {VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR, nullptr,
                                                memory_image_export.handle(), handle_type};
    HANDLE handle_buffer;
    HANDLE handle_image;
    ASSERT_VK_SUCCESS(vkGetMemoryWin32HandleKHR(m_device->device(), &mghi_buffer, &handle_buffer));
    ASSERT_VK_SUCCESS(vkGetMemoryWin32HandleKHR(m_device->device(), &mghi_image, &handle_image));

    VkImportMemoryWin32HandleInfoKHR import_info_buffer = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, nullptr,
                                                           handle_type, handle_buffer};
    VkImportMemoryWin32HandleInfoKHR import_info_image = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, nullptr,
                                                          handle_type, handle_image};
#else
    // Export memory to fd
    auto vkGetMemoryFdKHR = reinterpret_cast<PFN_vkGetMemoryFdKHR>(vk::GetInstanceProcAddr(instance(), "vkGetMemoryFdKHR"));
    ASSERT_TRUE(vkGetMemoryFdKHR != nullptr);

    auto mgfi_buffer = LvlInitStruct<VkMemoryGetFdInfoKHR>();
    mgfi_buffer.handleType = handle_type;
    mgfi_buffer.memory = memory_buffer_export.handle();

    auto mgfi_image = LvlInitStruct<VkMemoryGetFdInfoKHR>();
    mgfi_image.handleType = handle_type;
    mgfi_image.memory = memory_image_export.handle();

    int fd_buffer;
    int fd_image;
    ASSERT_VK_SUCCESS(vkGetMemoryFdKHR(m_device->device(), &mgfi_buffer, &fd_buffer));
    ASSERT_VK_SUCCESS(vkGetMemoryFdKHR(m_device->device(), &mgfi_image, &fd_image));

    auto import_info_buffer = LvlInitStruct<VkImportMemoryFdInfoKHR>();
    import_info_buffer.handleType = handle_type;
    import_info_buffer.fd = fd_buffer;

    auto import_info_image = LvlInitStruct<VkImportMemoryFdInfoKHR>();
    import_info_image.handleType = handle_type;
    import_info_image.fd = fd_image;
#endif

    // Import memory
    VkMemoryRequirements buffer_import_reqs = buffer_import.memory_requirements();
    if (buffer_import_reqs.memoryTypeBits == 0) {
        GTEST_SKIP() << "no suitable memory found, skipping test";
    }
    alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer_import_reqs, mem_flags);
    alloc_info.pNext = &import_info_buffer;
    vk_testing::DeviceMemory memory_buffer_import;
    memory_buffer_import.init(*m_device, alloc_info);
    ASSERT_TRUE(memory_buffer_import.initialized());

    VkMemoryRequirements image_import_reqs = image_import.memory_requirements();
    if (image_import_reqs.memoryTypeBits == 0) {
        GTEST_SKIP() << "no suitable memory found, skipping test";
    }
    alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image_import_reqs, mem_flags);
    alloc_info.pNext = &import_info_image;
    vk_testing::DeviceMemory memory_image_import;
    memory_image_import.init(*m_device, alloc_info);

    // Bind imported memory with different handleType
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-02727");
    vk::BindBufferMemory(device(), buffer_import.handle(), memory_buffer_import.handle(), 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = LvlInitStruct<VkBindBufferMemoryInfo>();
    bind_buffer_info.buffer = buffer_import.handle();
    bind_buffer_info.memory = memory_buffer_import.handle();
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryInfo-memory-02727");
    vkBindBufferMemory2Function(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-02729");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-memory-01614");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-memory-01612");
    vk::BindImageMemory(device(), image_import.handle(), memory_image_import.handle(), 0);
    m_errorMonitor->VerifyFound();

    VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>();
    bind_image_info.image = image_import.handle();
    bind_image_info.memory = memory_image_import.handle();
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-memory-02729");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-memory-01614");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-memory-01612");
    vkBindImageMemory2Function(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

template <typename ExtType, typename Parm>
void ExtendedDynStateCalls(ErrorMonitor *error_monitor, VkCommandBuffer cmd_buf, ExtType ext_call,
                           const char *vuid, Parm parm) {
    error_monitor->SetDesiredFailureMsg(kErrorBit, vuid);
    ext_call(cmd_buf, parm);
    error_monitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicStateDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state VUs");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "%s At least Vulkan version 1.1 is required, skipping test.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    // First test attempted uses of VK_EXT_extended_dynamic_state without it being enabled.
    extended_dynamic_state_features.extendedDynamicState = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetCullModeEXT");
    auto vkCmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetFrontFaceEXT");
    auto vkCmdSetPrimitiveTopologyEXT =
        (PFN_vkCmdSetPrimitiveTopologyEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetPrimitiveTopologyEXT");
    auto vkCmdSetViewportWithCountEXT =
        (PFN_vkCmdSetViewportWithCountEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetViewportWithCountEXT");
    auto vkCmdSetScissorWithCountEXT =
        (PFN_vkCmdSetScissorWithCountEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetScissorWithCountEXT");
    auto vkCmdSetDepthTestEnableEXT =
        (PFN_vkCmdSetDepthTestEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDepthTestEnableEXT");
    auto vkCmdSetDepthWriteEnableEXT =
        (PFN_vkCmdSetDepthWriteEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDepthWriteEnableEXT");
    auto vkCmdSetDepthCompareOpEXT =
        (PFN_vkCmdSetDepthCompareOpEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDepthCompareOpEXT");
    auto vkCmdSetDepthBoundsTestEnableEXT =
        (PFN_vkCmdSetDepthBoundsTestEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDepthBoundsTestEnableEXT");
    auto vkCmdSetStencilTestEnableEXT =
        (PFN_vkCmdSetStencilTestEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetStencilTestEnableEXT");
    auto vkCmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetStencilOpEXT");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {
        VK_DYNAMIC_STATE_CULL_MODE_EXT,           VK_DYNAMIC_STATE_FRONT_FACE_EXT,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT,  VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT,
        VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT,  VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT,
        VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,   VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
        VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT,    VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT,
        VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT, VK_DYNAMIC_STATE_STENCIL_OP_EXT,
    };
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.vp_state_ci_.viewportCount = 0;
    pipe.vp_state_ci_.scissorCount = 0;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03378");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    VkCommandBufferObj commandBuffer(m_device, m_commandPool);
    commandBuffer.begin();

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vkCmdSetCullModeEXT,
                          "VUID-vkCmdSetCullMode-None-03384", VK_CULL_MODE_NONE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vkCmdSetDepthBoundsTestEnableEXT,
        "VUID-vkCmdSetDepthBoundsTestEnable-None-03349", VK_FALSE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vkCmdSetDepthCompareOpEXT,
        "VUID-vkCmdSetDepthCompareOp-None-03353", VK_COMPARE_OP_NEVER);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vkCmdSetDepthTestEnableEXT,
        "VUID-vkCmdSetDepthTestEnable-None-03352", VK_FALSE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vkCmdSetDepthWriteEnableEXT,
        "VUID-vkCmdSetDepthWriteEnable-None-03354", VK_FALSE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vkCmdSetFrontFaceEXT,
                          "VUID-vkCmdSetFrontFace-None-03383", VK_FRONT_FACE_CLOCKWISE);

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vkCmdSetPrimitiveTopologyEXT,
                          "VUID-vkCmdSetPrimitiveTopology-None-03347", VK_PRIMITIVE_TOPOLOGY_POINT_LIST);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-None-03396");
    VkRect2D scissor = {{0, 0}, {1, 1}};
    vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetStencilOp-None-03351");
    vkCmdSetStencilOpEXT(commandBuffer.handle(), VK_STENCIL_FACE_BACK_BIT, VK_STENCIL_OP_ZERO, VK_STENCIL_OP_ZERO,
                         VK_STENCIL_OP_ZERO, VK_COMPARE_OP_NEVER);
    m_errorMonitor->VerifyFound();

    ExtendedDynStateCalls(m_errorMonitor, commandBuffer.handle(), vkCmdSetStencilTestEnableEXT,
        "VUID-vkCmdSetStencilTestEnable-None-03350", VK_FALSE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCount-None-03393");
    VkViewport viewport = {0, 0, 1, 1, 0.0f, 0.0f};
    vkCmdSetViewportWithCountEXT(commandBuffer.handle(), 1, &viewport);
    m_errorMonitor->VerifyFound();

    commandBuffer.end();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicStateEnabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state VUs");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required, skipping test.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdSetPrimitiveTopologyEXT =
        (PFN_vkCmdSetPrimitiveTopologyEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetPrimitiveTopologyEXT");
    auto vkCmdSetViewportWithCountEXT =
        (PFN_vkCmdSetViewportWithCountEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetViewportWithCountEXT");
    auto vkCmdSetScissorWithCountEXT =
        (PFN_vkCmdSetScissorWithCountEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetScissorWithCountEXT");
    auto vkCmdBindVertexBuffers2EXT =
        (PFN_vkCmdBindVertexBuffers2EXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBindVertexBuffers2EXT");
    auto vkCmdSetViewportWithCount =
        (PFN_vkCmdSetViewportWithCount)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetViewportWithCount");
    auto vkCmdSetScissorWithCount =
        (PFN_vkCmdSetScissorWithCount)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetScissorWithCount");
    auto vkCmdBindVertexBuffers2 =
        (PFN_vkCmdBindVertexBuffers2)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBindVertexBuffers2");
    if (vulkan_13) {
        assert(vkCmdSetViewportWithCount != nullptr);
        assert(vkCmdSetScissorWithCount != nullptr);
        assert(vkCmdBindVertexBuffers2 != nullptr);
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Verify viewportCount and scissorCount are specified as zero.
    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        const VkDynamicState dyn_states[] = {
            VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT,
            VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT,
        };
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03379");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03380");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    // Verify non-count and count dynamic states aren't used together
    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        const VkDynamicState dyn_states[] = {
            VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT, VK_DYNAMIC_STATE_VIEWPORT,  // viewports
            VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT, VK_DYNAMIC_STATE_SCISSOR     // scissors
        };
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 2;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();

        pipe.dyn_state_ci_.pDynamicStates = &dyn_states[0];  // viewports
        pipe.vp_state_ci_.viewportCount = 0;
        pipe.vp_state_ci_.scissorCount = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04132");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();

        pipe.dyn_state_ci_.pDynamicStates = &dyn_states[2];  // scissors
        pipe.vp_state_ci_.viewportCount = 1;
        pipe.vp_state_ci_.scissorCount = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04133");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    const VkDynamicState dyn_states[] = {
        VK_DYNAMIC_STATE_CULL_MODE_EXT,           VK_DYNAMIC_STATE_FRONT_FACE_EXT,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT,  VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT,
        VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT,  VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT,
        VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,   VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
        VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT,    VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT,
        VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT, VK_DYNAMIC_STATE_STENCIL_OP_EXT,
    };

    // Verify dupes of every state.
    for (size_t i = 0; i < size(dyn_states); ++i) {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 2;
        VkDynamicState dyn_state_dupes[2] = {dyn_states[i], dyn_states[i]};
        dyn_state_ci.pDynamicStates = dyn_state_dupes;
        pipe.dyn_state_ci_ = dyn_state_ci;
        if (dyn_states[i] == VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT) {
            pipe.vp_state_ci_.viewportCount = 0;
        }
        if (dyn_states[i] == VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT) {
            pipe.vp_state_ci_.scissorCount = 0;
        }
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    // Verify each vkCmdSet command
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.vp_state_ci_.viewportCount = 0;
    pipe.vp_state_ci_.scissorCount = 0;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    VkVertexInputBindingDescription inputBinding = {0, sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    pipe.vi_ci_.pVertexBindingDescriptions = &inputBinding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    VkVertexInputAttributeDescription attribute = {0, 0, VK_FORMAT_R32_SFLOAT, 0};
    pipe.vi_ci_.pVertexAttributeDescriptions = &attribute;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkBufferObj buffer;
    buffer.init(*m_device, 16, 0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    std::vector<VkBuffer> buffers(m_device->props.limits.maxVertexInputBindings + 1ull, buffer.handle());
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);

    VkCommandBufferObj commandBuffer(m_device, m_commandPool);
    commandBuffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-firstBinding-03355");
    vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), m_device->props.limits.maxVertexInputBindings, 1, buffers.data(),
                               offsets.data(), 0, 0);
    m_errorMonitor->VerifyFound();
    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-firstBinding-03355");
        vkCmdBindVertexBuffers2(commandBuffer.handle(), m_device->props.limits.maxVertexInputBindings, 1, buffers.data(),
                                offsets.data(), 0, 0);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-firstBinding-03356");
    vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, m_device->props.limits.maxVertexInputBindings + 1, buffers.data(),
                               offsets.data(), 0, 0);
    m_errorMonitor->VerifyFound();
    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-firstBinding-03356");
        vkCmdBindVertexBuffers2(commandBuffer.handle(), 0, m_device->props.limits.maxVertexInputBindings + 1, buffers.data(),
                                offsets.data(), 0, 0);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBufferObj bufferWrongUsage;
        bufferWrongUsage.init(*m_device, 16, 0, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pBuffers-03359");
        VkBuffer buffers2[1] = {bufferWrongUsage.handle()};
        VkDeviceSize offsets2[1] = {};
        vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, 0);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pBuffers-03359");
            vkCmdBindVertexBuffers2(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, 0);
            m_errorMonitor->VerifyFound();
        }
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pBuffers-04111");
        m_errorMonitor->SetUnexpectedError("UNASSIGNED-GeneralParameterError-RequiredParameter");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindVertexBuffers2-pBuffers-parameter");
        VkBuffer buffers2[1] = {VK_NULL_HANDLE};
        VkDeviceSize offsets2[1] = {16};
        VkDeviceSize strides[1] = {m_device->props.limits.maxVertexInputBindingStride + 1ull};
        vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, 0);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pBuffers-04111");
            m_errorMonitor->SetUnexpectedError("UNASSIGNED-GeneralParameterError-RequiredParameter");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindVertexBuffers2-pBuffers-parameter");
            vkCmdBindVertexBuffers2(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, 0);
            m_errorMonitor->VerifyFound();
        }

        buffers2[0] = buffers[0];
        VkDeviceSize sizes[1] = {16};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pOffsets-03357");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pSizes-03358");
        vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, sizes, 0);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pOffsets-03357");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pSizes-03358");
            vkCmdBindVertexBuffers2(commandBuffer.handle(), 0, 1, buffers2, offsets2, sizes, 0);
            m_errorMonitor->VerifyFound();
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pStrides-03362");
        vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, strides);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pStrides-03362");
            vkCmdBindVertexBuffers2(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, strides);
            m_errorMonitor->VerifyFound();
        }
    }

    commandBuffer.BeginRenderPass(m_renderPassBeginInfo);

    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    VkPipelineDynamicStateCreateInfo dyn_state_ci2 = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci2.dynamicStateCount = 1;
    VkDynamicState dynamic_state2 = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT;
    dyn_state_ci2.pDynamicStates = &dynamic_state2;
    pipe2.dyn_state_ci_ = dyn_state_ci2;
    pipe2.vp_state_ci_.viewportCount = 0;
    pipe2.InitState();
    pipe2.CreateGraphicsPipeline();
    vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewportCount-03417");
    vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    CreatePipelineHelper pipe3(*this);
    pipe3.InitInfo();
    VkPipelineDynamicStateCreateInfo dyn_state_ci3 = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci3.dynamicStateCount = 1;
    VkDynamicState dynamic_state3 = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT;
    dyn_state_ci3.pDynamicStates = &dynamic_state3;
    pipe3.dyn_state_ci_ = dyn_state_ci3;
    pipe3.vp_state_ci_.scissorCount = 0;
    pipe3.InitState();
    pipe3.CreateGraphicsPipeline();
    vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-scissorCount-03418");
    vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    VkDeviceSize strides[1] = {0};
    vkCmdSetPrimitiveTopologyEXT(commandBuffer.handle(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers.data(), offsets.data(), 0, strides);
    VkRect2D scissor = {{0, 0}, {1, 1}};
    vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewportCount-03419");
    vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    VkViewport viewport = {0, 0, 1, 1, 0.0f, 0.0f};
    vkCmdSetViewportWithCountEXT(commandBuffer.handle(), 1, &viewport);
    strides[0] = 1;
    vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers.data(), offsets.data(), 0, strides);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2-pStrides-06209");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");
    vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    strides[0] = 4;
    vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers.data(), offsets.data(), 0, strides);

    vkCmdSetPrimitiveTopologyEXT(commandBuffer.handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitiveTopology-03420");
    vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndRenderPass(commandBuffer.handle());

    if (features2.features.multiViewport) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCount-viewportCount-03394");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdSetViewportWithCount-viewportCount-arraylength");
        VkViewport viewport2 = {
            0, 0, 1, 1, 0.0f, 0.0f,
        };
        vkCmdSetViewportWithCountEXT(commandBuffer.handle(), 0, &viewport2);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCount-viewportCount-03394");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdSetViewportWithCount-viewportCount-arraylength");
            vkCmdSetViewportWithCount(commandBuffer.handle(), 0, &viewport2);
            m_errorMonitor->VerifyFound();
        }
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-offset-03400");
        VkRect2D scissor2 = {{1, 0}, {INT32_MAX, 16}};
        vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor2);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-offset-03400");
            vkCmdSetScissorWithCount(commandBuffer.handle(), 1, &scissor2);
            m_errorMonitor->VerifyFound();
        }
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-offset-03401");
        VkRect2D scissor2 = {{0, 1}, {16, INT32_MAX}};
        vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor2);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-offset-03401");
            vkCmdSetScissorWithCount(commandBuffer.handle(), 1, &scissor2);
            m_errorMonitor->VerifyFound();
        }
    }

    if (features2.features.multiViewport) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-scissorCount-03397");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdSetScissorWithCount-scissorCount-arraylength");
        vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 0, 0);
        m_errorMonitor->VerifyFound();
        if (vulkan_13) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-scissorCount-03397");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdSetScissorWithCount-scissorCount-arraylength");
            vkCmdSetScissorWithCount(commandBuffer.handle(), 0, 0);
            m_errorMonitor->VerifyFound();
        }
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-x-03399");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-x-03399");
    VkRect2D scissor3 = {{-1, -1}, {0, 0}};
    vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor3);
    m_errorMonitor->VerifyFound();
    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-x-03399");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-x-03399");
        vkCmdSetScissorWithCount(commandBuffer.handle(), 1, &scissor3);
        m_errorMonitor->VerifyFound();
    }

    if (vulkan_13) {
        vkCmdBindVertexBuffers2(commandBuffer.handle(), 0, 0, nullptr, nullptr, nullptr, nullptr);
    }
    vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 0, nullptr, nullptr, nullptr, nullptr);

    commandBuffer.end();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicStateEnabledNoMultiview) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required, skipping test.";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    features2.features.multiViewport = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdSetViewportWithCountEXT =
        (PFN_vkCmdSetViewportWithCountEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetViewportWithCountEXT");
    auto vkCmdSetScissorWithCountEXT =
        (PFN_vkCmdSetScissorWithCountEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetScissorWithCountEXT");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj commandBuffer(m_device, m_commandPool);
    commandBuffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCount-viewportCount-03395");
    VkViewport viewport = {0, 0, 1, 1, 0.0f, 0.0f};
    VkViewport viewports[] = {viewport, viewport};
    vkCmdSetViewportWithCountEXT(commandBuffer.handle(), size(viewports), viewports);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCount-scissorCount-03398");
    VkRect2D scissor = {{0, 0}, {1, 1}};
    VkRect2D scissors[] = {scissor, scissor};
    vkCmdSetScissorWithCountEXT(commandBuffer.handle(), size(scissors), scissors);
    m_errorMonitor->VerifyFound();

    commandBuffer.end();
}

TEST_F(VkLayerTest, InvalidFragmentShadingRateDeviceFeatureCombinations) {
    TEST_DESCRIPTION(
        "Specify invalid combinations of fragment shading rate, shading rate image, and fragment density map features");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    bool sri_extension = IsExtensionsEnabled(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    bool fdm_extension = IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);

    if (!fdm_extension && !sri_extension) {
        GTEST_SKIP() << "requires VK_NV_shading_rate_image or VK_EXT_fragment_density_map";
    }

    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_query_features =
        LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    VkPhysicalDeviceShadingRateImageFeaturesNV sri_query_features = LvlInitStruct<VkPhysicalDeviceShadingRateImageFeaturesNV>();
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_query_features =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();

    if (fdm_extension) {
        fsr_query_features.pNext = &fdm_query_features;
    }
    if (sri_extension) {
        if (fdm_extension) {
            fdm_query_features.pNext = &sri_query_features;
        } else {
            fsr_query_features.pNext = &sri_query_features;
        }
    }
    GetPhysicalDeviceFeatures2(fsr_query_features);

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

    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_features = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    VkPhysicalDeviceShadingRateImageFeaturesNV sri_features = LvlInitStruct<VkPhysicalDeviceShadingRateImageFeaturesNV>();
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    device_ci.pNext = &features2;

    VkDevice testDevice;

    if (sri_query_features.shadingRateImage) {
        sri_features.shadingRateImage = true;
        fsr_features.pNext = &sri_features;
        if (fsr_query_features.pipelineFragmentShadingRate) {
            fsr_features.pipelineFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-shadingRateImage-04478");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.pipelineFragmentShadingRate = false;
        }
        if (fsr_query_features.primitiveFragmentShadingRate) {
            fsr_features.primitiveFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-shadingRateImage-04479");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.primitiveFragmentShadingRate = false;
        }
        if (fsr_query_features.attachmentFragmentShadingRate) {
            fsr_features.attachmentFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-shadingRateImage-04480");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.attachmentFragmentShadingRate = false;
        }
        fsr_features.pNext = nullptr;
    }

    if (fdm_query_features.fragmentDensityMap) {
        fdm_features.fragmentDensityMap = true;
        fsr_features.pNext = &fdm_features;
        if (fsr_query_features.pipelineFragmentShadingRate) {
            fsr_features.pipelineFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-fragmentDensityMap-04481");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.pipelineFragmentShadingRate = false;
        }
        if (fsr_query_features.primitiveFragmentShadingRate) {
            fsr_features.primitiveFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-fragmentDensityMap-04482");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.primitiveFragmentShadingRate = false;
        }
        if (fsr_query_features.attachmentFragmentShadingRate) {
            fsr_features.attachmentFragmentShadingRate = true;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-fragmentDensityMap-04483");
            vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
            m_errorMonitor->VerifyFound();
            fsr_features.attachmentFragmentShadingRate = false;
        }
        fsr_features.pNext = nullptr;
    }
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

    VkFence fence;
    VkFenceCreateInfo fence_create_info = LvlInitStruct<VkFenceCreateInfo>();
    vk::CreateFence(device(), &fence_create_info, nullptr, &fence);

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(device(), &event_create_info, nullptr, &event);

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
    vk::ResetFences(device(), 0, &fence);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWaitForFences-fenceCount-arraylength");
    vk::WaitForFences(device(), 0, &fence, true, 1);
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
    vk::CmdWaitEvents(command_buffer.handle(), 0, &event, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                      nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    command_buffer.end();

    vk::DestroyFence(device(), fence, nullptr);
    vk::DestroyEvent(device(), event, nullptr);
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

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, doesn't support Vulkan 1.1+";
    }

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
    m_errorMonitor->SetUnexpectedError(kVUID_Core_Shader_InconsistentSpirv);
    if (!vs.InitFromASMTry()) {
        GTEST_SKIP() << "Failed to compile shader";
    }
    const VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-04146");
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidVkSemaphoreTypeCreateInfoCore) {
    TEST_DESCRIPTION("Invalid usage of VkSemaphoreTypeCreateInfo with a 1.2 core version");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    // Core 1.2 supports timelineSemaphore feature bit but not enabled
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSemaphore semaphore;

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 1;

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);
    semaphore_create_info.flags = 0;

    // timelineSemaphore feature bit not set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252");
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();

    // Binary semaphore can't be initialValue 0
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279");
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidVkSemaphoreTypeCreateInfoExtension) {
    TEST_DESCRIPTION("Invalid usage of VkSemaphoreTypeCreateInfo with extension");

    SetTargetApiVersion(VK_API_VERSION_1_1);  // before timelineSemaphore was added to core
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Enabled extension but not the timelineSemaphore feature bit
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSemaphore semaphore;

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphore_type_create_info.initialValue = 1;

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);
    semaphore_create_info.flags = 0;

    // timelineSemaphore feature bit not set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252");
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();

    // Binary semaphore can't be initialValue 0
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279");
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MixedTimelineAndBinarySemaphores) {
    TEST_DESCRIPTION("Submit mixtures of timeline and binary semaphores");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        GTEST_SKIP() << "Timeline semaphore not supported";
    }

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto timelineproperties = LvlInitStruct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    VkSemaphore semaphore[2];
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[0]));
    // index 1 should be a binary semaphore
    semaphore_create_info.pNext = nullptr;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[1]));
    VkSemaphore extra_binary;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &extra_binary));

    VkSemaphoreSignalInfo semaphore_signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
    semaphore_signal_info.semaphore = semaphore[0];
    semaphore_signal_info.value = 3;
    auto vkSignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)vk::GetDeviceProcAddr(m_device->device(), "vkSignalSemaphoreKHR");

    semaphore_signal_info.value = 10;
    ASSERT_VK_SUCCESS(vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
    uint64_t signalValue = 20;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = nullptr;
    // this array needs a length of 2, even though the binary semaphore won't look at the values array
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>(&timeline_semaphore_submit_info);
    submit_info.pWaitDstStageMask = &stageFlags;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.signalSemaphoreCount = 2;
    submit_info.pSignalSemaphores = semaphore;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-03241");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    uint64_t values[2] = {signalValue, 0 /*ignored*/};
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 2;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = values;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    // the indexes in pWaitSemaphores and pWaitSemaphoreValues should match
    VkSemaphore reversed[2] = {semaphore[1], semaphore[0]};
    uint64_t reversed_values[2] = {UINT64_MAX /* ignored */, 20};
    VkPipelineStageFlags wait_stages[2] = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    submit_info.waitSemaphoreCount = 2;
    submit_info.pWaitSemaphores = reversed;
    submit_info.pWaitDstStageMask = wait_stages;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = nullptr;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 2;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = reversed_values;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    // if we only signal a binary semaphore we don't need a 'values' array
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = nullptr;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = nullptr;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &extra_binary;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    vk::DestroySemaphore(m_device->device(), semaphore[0], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[1], nullptr);
    vk::DestroySemaphore(m_device->device(), extra_binary, nullptr);
}

TEST_F(VkLayerTest, ValidateExtendedDynamicState2Disabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 VUs");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Add first extension to catch bugs where layers check from wrong feature bit
    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto extended_dynamic_state2_features =
        LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>(&extended_dynamic_state_features);
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }

    // Attempt using VK_EXT_extended_dynamic_state2 without it being enabled.
    extended_dynamic_state2_features.extendedDynamicState2 = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdSetRasterizerDiscardEnableEXT =
        (PFN_vkCmdSetRasterizerDiscardEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetRasterizerDiscardEnableEXT");
    auto vkCmdSetDepthBiasEnableEXT =
        (PFN_vkCmdSetDepthBiasEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDepthBiasEnableEXT");
    auto vkCmdSetPrimitiveRestartEnableEXT =
        (PFN_vkCmdSetPrimitiveRestartEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetPrimitiveRestartEnableEXT");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT, VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04868");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    VkCommandBufferObj m_commandBuffer(m_device, m_commandPool);
    m_commandBuffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizerDiscardEnable-None-04871");
    vkCmdSetRasterizerDiscardEnableEXT(m_commandBuffer.handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBiasEnable-None-04872");
    vkCmdSetDepthBiasEnableEXT(m_commandBuffer.handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPrimitiveRestartEnable-None-04866");
    vkCmdSetPrimitiveRestartEnableEXT(m_commandBuffer.handle(), VK_TRUE);
    m_errorMonitor->VerifyFound();

    m_commandBuffer.end();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicState2PatchControlPointsDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 PatchControlPoints VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2LogicOp, skipping";
    }

    // Attempt using VK_EXT_extended_dynamic_state2 without it being enabled.
    extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdSetPatchControlPointsEXT =
        (PFN_vkCmdSetPatchControlPointsEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetPatchControlPointsEXT");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04870");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    VkCommandBufferObj m_commandBuffer(m_device, m_commandPool);
    m_commandBuffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPatchControlPointsEXT-None-04873");
    vkCmdSetPatchControlPointsEXT(m_commandBuffer.handle(), 3);
    m_errorMonitor->VerifyFound();

    m_commandBuffer.end();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicState2LogicOpDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2LogicOp VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2LogicOp) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2LogicOp, skipping";
    }

    // Attempt using VK_EXT_extended_dynamic_state2 without it being enabled.
    extended_dynamic_state2_features.extendedDynamicState2LogicOp = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetLogicOpEXT");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_LOGIC_OP_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04869");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    VkCommandBufferObj m_commandBuffer(m_device, m_commandPool);
    m_commandBuffer.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLogicOpEXT-None-04867");
    vkCmdSetLogicOpEXT(m_commandBuffer.handle(), VK_LOGIC_OP_AND);
    m_errorMonitor->VerifyFound();

    m_commandBuffer.end();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicState2Enabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 LogicOp VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2, skipping";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT, VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT};

    for (size_t i = 0; i < size(dyn_states); ++i) {
        // Verify duplicates of every dynamic state.
        {
            CreatePipelineHelper pipe(*this);
            pipe.InitInfo();
            auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
            dyn_state_ci.dynamicStateCount = 2;
            VkDynamicState dyn_state_dupes[2] = {dyn_states[i], dyn_states[i]};
            dyn_state_ci.pDynamicStates = dyn_state_dupes;
            pipe.dyn_state_ci_ = dyn_state_ci;
            pipe.InitState();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
            pipe.CreateGraphicsPipeline();
            m_errorMonitor->VerifyFound();
        }

        // Calling draw without setting the dynamic state is an error
        {
            CreatePipelineHelper pipe2(*this);
            pipe2.InitInfo();
            auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
            dyn_state_ci.dynamicStateCount = 1;
            dyn_state_ci.pDynamicStates = &dyn_states[i];
            pipe2.dyn_state_ci_ = dyn_state_ci;
            pipe2.InitState();
            pipe2.CreateGraphicsPipeline();

            VkCommandBufferObj m_commandBuffer(m_device, m_commandPool);
            m_commandBuffer.begin();
            m_commandBuffer.BeginRenderPass(m_renderPassBeginInfo);

            vk::CmdBindPipeline(m_commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);

            if (dyn_states[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04876");
            if (dyn_states[i] == VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT)
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04877");
            if (dyn_states[i] == VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT)
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04879");
            vk::CmdDraw(m_commandBuffer.handle(), 1, 1, 0, 0);
            m_errorMonitor->VerifyFound();
            vk::CmdEndRenderPass(m_commandBuffer.handle());
            m_commandBuffer.end();
        }
    }
}

TEST_F(VkLayerTest, ValidateExtendedDynamicState2PatchControlPointsEnabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 PatchControlPoints VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2PatchControlPoints, skipping";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    auto vkCmdSetPatchControlPointsEXT =
        (PFN_vkCmdSetPatchControlPointsEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetPatchControlPointsEXT");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT};

    // Verify dupes of the dynamic state.
    for (size_t i = 0; i < size(dyn_states); ++i) {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 2;
        VkDynamicState dyn_state_dupes[2] = {dyn_states[i], dyn_states[i]};
        dyn_state_ci.pDynamicStates = dyn_state_dupes;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        VkCommandBufferObj m_commandBuffer(m_device, m_commandPool);
        m_commandBuffer.begin();
        m_commandBuffer.BeginRenderPass(m_renderPassBeginInfo);

        vk::CmdBindPipeline(m_commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        // Calling draw without setting the dynamic state is an error
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-04875");
        vk::CmdDraw(m_commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();

        // setting an invalid value for patchControlpoints is an error
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPatchControlPointsEXT-patchControlPoints-04874");
        vkCmdSetPatchControlPointsEXT(m_commandBuffer.handle(), 0x1000);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(m_commandBuffer.handle());
        m_commandBuffer.end();
    }
}

TEST_F(VkLayerTest, ValidateExtendedDynamicState2LogicOpEnabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 LogicOp VUs");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state2_features);
    if (!extended_dynamic_state2_features.extendedDynamicState2LogicOp) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState2LogicOp, skipping";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_LOGIC_OP_EXT};

    // Verify dupes of the dynamic state.
    for (size_t i = 0; i < size(dyn_states); ++i) {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 2;
        VkDynamicState dyn_state_dupes[2] = {dyn_states[i], dyn_states[i]};
        dyn_state_ci.pDynamicStates = dyn_state_dupes;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        VkCommandBufferObj m_commandBuffer(m_device, m_commandPool);
        m_commandBuffer.begin();
        m_commandBuffer.BeginRenderPass(m_renderPassBeginInfo);

        vk::CmdBindPipeline(m_commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        // Calling draw without setting the dynamic state is an error
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-logicOp-04878");
        vk::CmdDraw(m_commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(m_commandBuffer.handle());
        m_commandBuffer.end();
    }
}

TEST_F(VkLayerTest, ValidateExtendedDynamicState3Disabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state3 VUs");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto extended_dynamic_state3_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);

    // Create a device with all VK_EXT_extended_dynamic_state3 features disabled
    extended_dynamic_state3_features.extendedDynamicState3TessellationDomainOrigin = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3DepthClampEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3PolygonMode = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3RasterizationSamples = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3SampleMask = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3AlphaToCoverageEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3AlphaToOneEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3LogicOpEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3RasterizationStream = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ConservativeRasterizationMode = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ExtraPrimitiveOverestimationSize = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3DepthClipEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3SampleLocationsEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ColorBlendAdvanced = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ProvokingVertexMode = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3LineRasterizationMode = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3LineStippleEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3DepthClipNegativeOneToOne = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ViewportWScalingEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ViewportSwizzle = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3CoverageToColorEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3CoverageToColorLocation = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3CoverageModulationMode = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3CoverageModulationTableEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3CoverageModulationTable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3CoverageReductionMode = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3RepresentativeFragmentTestEnable = VK_FALSE;
    extended_dynamic_state3_features.extendedDynamicState3ShadingRateImageEnable = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Check feature is enabled for each pipeline dynamic states.
    struct DynamicStateEnableTest {
        VkDynamicState dynamicState;
        char const *vuid;
    } const dynamicStateEnableTests[] = {
        {
            VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3TessellationDomainOrigin-07370",
        },
        {
            VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClampEnable-07371",
        },
        {
            VK_DYNAMIC_STATE_POLYGON_MODE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3PolygonMode-07372",
        },
        {
            VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RasterizationSamples-07373",
        },
        {
            VK_DYNAMIC_STATE_SAMPLE_MASK_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3SampleMask-07374",
        },
        {
            VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3AlphaToCoverageEnable-07375",
        },
        {
            VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3AlphaToOneEnable-07376",
        },
        {
            VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LogicOpEnable-07377",
        },
        {
            VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendEnable-07378",
        },
        {
            VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendEquation-07379",
        },
        {
            VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorWriteMask-07380",
        },
        {
            VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RasterizationStream-07381",
        },
        {
            VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ConservativeRasterizationMode-07382",
        },
        {
            VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ExtraPrimitiveOverestimationSize-07383",
        },
        {
            VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClipEnable-07384",
        },
        {
            VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3SampleLocationsEnable-07385",
        },
        {
            VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendAdvanced-07386",
        },
        {
            VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ProvokingVertexMode-07387",
        },
        {
            VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LineRasterizationMode-07388",
        },
        {
            VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LineStippleEnable-07389",
        },
        {
            VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClipNegativeOneToOne-07390",
        },
        {
            VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ViewportWScalingEnable-07391",
        },
        {
            VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ViewportSwizzle-07392",
        },
        {
            VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageToColorEnable-07393",
        },
        {
            VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageToColorLocation-07394",
        },
        {
            VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationMode-07395",
        },
        {
            VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationTableEnable-07396",
        },
        {
            VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationTable-07397",
        },
        {
            VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageReductionMode-07398",
        },
        {
            VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RepresentativeFragmentTestEnable-07399",
        },
        {
            VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV,
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ShadingRateImageEnable-07400",
        },
    };
    for (size_t i = 0; i < size(dynamicStateEnableTests); ++i) {
        DynamicStateEnableTest const &test = dynamicStateEnableTests[i];
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        const VkDynamicState dyn_states[] = {test.dynamicState};
        auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.vuid);
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    // Check feature is enable for each set command.
    VkCommandBufferObj m_commandBuffer(m_device, m_commandPool);
    m_commandBuffer.begin();
    {
        auto vkCmdSetTessellationDomainOriginEXT = (PFN_vkCmdSetTessellationDomainOriginEXT)vk::GetDeviceProcAddr(
            m_device->device(), "vkCmdSetTessellationDomainOriginEXT");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetTessellationDomainOriginEXT-extendedDynamicState3TessellationDomainOrigin-07444");
        vkCmdSetTessellationDomainOriginEXT(m_commandBuffer.handle(), VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetDepthClampEnableEXT =
            (PFN_vkCmdSetDepthClampEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDepthClampEnableEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetDepthClampEnableEXT-extendedDynamicState3DepthClampEnable-07448");
        vkCmdSetDepthClampEnableEXT(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetPolygonModeEXT =
            (PFN_vkCmdSetPolygonModeEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetPolygonModeEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPolygonModeEXT-extendedDynamicState3PolygonMode-07422");
        vkCmdSetPolygonModeEXT(m_commandBuffer.handle(), VK_POLYGON_MODE_FILL);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetRasterizationSamplesEXT =
            (PFN_vkCmdSetRasterizationSamplesEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetRasterizationSamplesEXT");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetRasterizationSamplesEXT-extendedDynamicState3RasterizationSamples-07414");
        vkCmdSetRasterizationSamplesEXT(m_commandBuffer.handle(), VK_SAMPLE_COUNT_1_BIT);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetSampleMaskEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetSampleMaskEXT-extendedDynamicState3SampleMask-07342");
        VkSampleMask sampleMask = 1U;
        vkCmdSetSampleMaskEXT(m_commandBuffer.handle(), VK_SAMPLE_COUNT_1_BIT, &sampleMask);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetAlphaToCoverageEnableEXT =
            (PFN_vkCmdSetAlphaToCoverageEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetAlphaToCoverageEnableEXT");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetAlphaToCoverageEnableEXT-extendedDynamicState3AlphaToCoverageEnable-07343");
        vkCmdSetAlphaToCoverageEnableEXT(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetAlphaToOneEnableEXT =
            (PFN_vkCmdSetAlphaToOneEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetAlphaToOneEnableEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetAlphaToOneEnableEXT-extendedDynamicState3AlphaToOneEnable-07345");
        vkCmdSetAlphaToOneEnableEXT(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetLogicOpEnableEXT =
            (PFN_vkCmdSetLogicOpEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetLogicOpEnableEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLogicOpEnableEXT-extendedDynamicState3LogicOpEnable-07365");
        vkCmdSetLogicOpEnableEXT(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetColorBlendEnableEXT =
            (PFN_vkCmdSetColorBlendEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetColorBlendEnableEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetColorBlendEnableEXT-extendedDynamicState3ColorBlendEnable-07355");
        VkBool32 enable = VK_FALSE;
        vkCmdSetColorBlendEnableEXT(m_commandBuffer.handle(), 0U, 1U, &enable);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetColorBlendEquationEXT =
            (PFN_vkCmdSetColorBlendEquationEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetColorBlendEquationEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetColorBlendEquationEXT-extendedDynamicState3ColorBlendEquation-07356");
        VkColorBlendEquationEXT equation = {
            VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
        };
        vkCmdSetColorBlendEquationEXT(m_commandBuffer.handle(), 0U, 1U, &equation);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetColorWriteMaskEXT =
            (PFN_vkCmdSetColorWriteMaskEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetColorWriteMaskEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetColorWriteMaskEXT-extendedDynamicState3ColorWriteMask-07364");
        VkColorComponentFlags const components = {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                  VK_COLOR_COMPONENT_A_BIT};
        vkCmdSetColorWriteMaskEXT(m_commandBuffer.handle(), 0U, 1U, &components);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetRasterizationStreamEXT =
            (PFN_vkCmdSetRasterizationStreamEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetRasterizationStreamEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetRasterizationStreamEXT-extendedDynamicState3RasterizationStream-07410");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-transformFeedback-07411");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07412");
        vkCmdSetRasterizationStreamEXT(m_commandBuffer.handle(), 0U);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetConservativeRasterizationModeEXT = (PFN_vkCmdSetConservativeRasterizationModeEXT)vk::GetDeviceProcAddr(
            m_device->device(), "vkCmdSetConservativeRasterizationModeEXT");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetConservativeRasterizationModeEXT-extendedDynamicState3ConservativeRasterizationMode-07426");
        vkCmdSetConservativeRasterizationModeEXT(m_commandBuffer.handle(), VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetExtraPrimitiveOverestimationSizeEXT = (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)vk::GetDeviceProcAddr(
            m_device->device(), "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-extendedDynamicState3ExtraPrimitiveOverestimationSize-07427");
        vkCmdSetExtraPrimitiveOverestimationSizeEXT(m_commandBuffer.handle(), 0.0f);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetDepthClipEnableEXT =
            (PFN_vkCmdSetDepthClipEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDepthClipEnableEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetDepthClipEnableEXT-extendedDynamicState3DepthClipEnable-07450");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthClipEnableEXT-depthClipEnable-07451");
        vkCmdSetDepthClipEnableEXT(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetSampleLocationsEnableEXT =
            (PFN_vkCmdSetSampleLocationsEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetSampleLocationsEnableEXT");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetSampleLocationsEnableEXT-extendedDynamicState3SampleLocationsEnable-07415");
        vkCmdSetSampleLocationsEnableEXT(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetColorBlendAdvancedEXT =
            (PFN_vkCmdSetColorBlendAdvancedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetColorBlendAdvancedEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetColorBlendAdvancedEXT-extendedDynamicState3ColorBlendAdvanced-07504");
        VkColorBlendAdvancedEXT const advanced = {VK_BLEND_OP_BLUE_EXT, VK_FALSE, VK_FALSE, VK_BLEND_OVERLAP_UNCORRELATED_EXT,
                                                  VK_FALSE};
        vkCmdSetColorBlendAdvancedEXT(m_commandBuffer.handle(), 0U, 1U, &advanced);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetProvokingVertexModeEXT =
            (PFN_vkCmdSetProvokingVertexModeEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetProvokingVertexModeEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetProvokingVertexModeEXT-extendedDynamicState3ProvokingVertexMode-07446");
        vkCmdSetProvokingVertexModeEXT(m_commandBuffer.handle(), VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetLineRasterizationModeEXT =
            (PFN_vkCmdSetLineRasterizationModeEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetLineRasterizationModeEXT");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetLineRasterizationModeEXT-extendedDynamicState3LineRasterizationMode-07417");
        vkCmdSetLineRasterizationModeEXT(m_commandBuffer.handle(), VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetLineStippleEnableEXT =
            (PFN_vkCmdSetLineStippleEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetLineStippleEnableEXT");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetLineStippleEnableEXT-extendedDynamicState3LineStippleEnable-07421");
        vkCmdSetLineStippleEnableEXT(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetDepthClipNegativeOneToOneEXT = (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)vk::GetDeviceProcAddr(
            m_device->device(), "vkCmdSetDepthClipNegativeOneToOneEXT");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-extendedDynamicState3DepthClipNegativeOneToOne-07452");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-depthClipControl-07453");
        vkCmdSetDepthClipNegativeOneToOneEXT(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetViewportWScalingEnableNV =
            (PFN_vkCmdSetViewportWScalingEnableNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetViewportWScalingEnableNV");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetViewportWScalingEnableNV-extendedDynamicState3ViewportWScalingEnable-07580");
        vkCmdSetViewportWScalingEnableNV(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetViewportSwizzleNV =
            (PFN_vkCmdSetViewportSwizzleNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetViewportSwizzleNV");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdSetViewportSwizzleNV-extendedDynamicState3ViewportSwizzle-07445");
        VkViewportSwizzleNV const swizzle = {
            VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV,
            VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV};
        vkCmdSetViewportSwizzleNV(m_commandBuffer.handle(), 0U, 1U, &swizzle);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetCoverageToColorEnableNV =
            (PFN_vkCmdSetCoverageToColorEnableNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetCoverageToColorEnableNV");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageToColorEnableNV-extendedDynamicState3CoverageToColorEnable-07347");
        vkCmdSetCoverageToColorEnableNV(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetCoverageToColorLocationNV =
            (PFN_vkCmdSetCoverageToColorLocationNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetCoverageToColorLocationNV");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageToColorLocationNV-extendedDynamicState3CoverageToColorLocation-07348");
        vkCmdSetCoverageToColorLocationNV(m_commandBuffer.handle(), 0U);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetCoverageModulationModeNV =
            (PFN_vkCmdSetCoverageModulationModeNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetCoverageModulationModeNV");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageModulationModeNV-extendedDynamicState3CoverageModulationMode-07350");
        vkCmdSetCoverageModulationModeNV(m_commandBuffer.handle(), VK_COVERAGE_MODULATION_MODE_NONE_NV);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetCoverageModulationTableEnableNV = (PFN_vkCmdSetCoverageModulationTableEnableNV)vk::GetDeviceProcAddr(
            m_device->device(), "vkCmdSetCoverageModulationTableEnableNV");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageModulationTableEnableNV-extendedDynamicState3CoverageModulationTableEnable-07351");
        vkCmdSetCoverageModulationTableEnableNV(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetCoverageModulationTableNV =
            (PFN_vkCmdSetCoverageModulationTableNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetCoverageModulationTableNV");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageModulationTableNV-extendedDynamicState3CoverageModulationTable-07352");
        float const modulation = 1.0f;
        vkCmdSetCoverageModulationTableNV(m_commandBuffer.handle(), 1U, &modulation);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetShadingRateImageEnableNV =
            (PFN_vkCmdSetShadingRateImageEnableNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetShadingRateImageEnableNV");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetShadingRateImageEnableNV-extendedDynamicState3ShadingRateImageEnable-07416");
        vkCmdSetShadingRateImageEnableNV(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetRepresentativeFragmentTestEnableNV = (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)vk::GetDeviceProcAddr(
            m_device->device(), "vkCmdSetRepresentativeFragmentTestEnableNV");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "VUID-vkCmdSetRepresentativeFragmentTestEnableNV-extendedDynamicState3RepresentativeFragmentTestEnable-07346");
        vkCmdSetRepresentativeFragmentTestEnableNV(m_commandBuffer.handle(), VK_FALSE);
        m_errorMonitor->VerifyFound();
    }
    {
        auto vkCmdSetCoverageReductionModeNV =
            (PFN_vkCmdSetCoverageReductionModeNV)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetCoverageReductionModeNV");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetCoverageReductionModeNV-extendedDynamicState3CoverageReductionMode-07349");
        vkCmdSetCoverageReductionModeNV(m_commandBuffer.handle(), VK_COVERAGE_REDUCTION_MODE_MERGE_NV);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer.end();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicState3Enabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state3 VUs");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto line_rasterization_feature = LvlInitStruct<VkPhysicalDeviceLineRasterizationFeaturesEXT>();
    auto provoking_vertex_features = LvlInitStruct<VkPhysicalDeviceProvokingVertexFeaturesEXT>(&line_rasterization_feature);
    auto transform_feedback_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(&provoking_vertex_features);
    auto extended_dynamic_state3_features =
        LvlInitStruct<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(&transform_feedback_features);
    auto features2 = GetPhysicalDeviceFeatures2(extended_dynamic_state3_features);
    features2.features.depthClamp = VK_FALSE;
    features2.features.fillModeNonSolid = VK_FALSE;
    features2.features.alphaToOne = VK_FALSE;
    features2.features.logicOp = VK_FALSE;
    features2.features.dualSrcBlend = VK_FALSE;
    transform_feedback_features.transformFeedback = VK_FALSE;
    provoking_vertex_features.provokingVertexLast = VK_FALSE;
    line_rasterization_feature.rectangularLines = VK_FALSE;
    line_rasterization_feature.bresenhamLines = VK_FALSE;
    line_rasterization_feature.smoothLines = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDynamicState const dyn_states[] = {VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT,
                                         VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_POLYGON_MODE_EXT,
                                         VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
                                         VK_DYNAMIC_STATE_SAMPLE_MASK_EXT,
                                         VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
                                         VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
                                         VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT,
                                         VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT,
                                         VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT,
                                         VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT,
                                         VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT,
                                         VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT,
                                         VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT,
                                         VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT,
                                         VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV,
                                         VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV,
                                         VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV,
                                         VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV,
                                         VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV};

    // Verify dupes of every state.
    for (size_t i = 0; i < size(dyn_states); ++i) {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 2;
        VkDynamicState dyn_state_dupes[2] = {dyn_states[i], dyn_states[i]};
        dyn_state_ci.pDynamicStates = dyn_state_dupes;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }


    if (extended_dynamic_state3_features.extendedDynamicState3TessellationDomainOrigin) {
        auto vkCmdSetTessellationDomainOriginEXT = (PFN_vkCmdSetTessellationDomainOriginEXT)vk::GetDeviceProcAddr(
            m_device->device(), "vkCmdSetTessellationDomainOriginEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        VkPipelineTessellationDomainOriginStateCreateInfo tess_domain_ci =
            LvlInitStruct<VkPipelineTessellationDomainOriginStateCreateInfo>();
        tess_domain_ci.domainOrigin = VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT;
        VkPipelineTessellationStateCreateInfo tess_ci = LvlInitStruct<VkPipelineTessellationStateCreateInfo>(&tess_domain_ci);
        pipe.gp_ci_.pTessellationState = &tess_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07619");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        vkCmdSetTessellationDomainOriginEXT(commandBuffer.handle(), VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT);
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (extended_dynamic_state3_features.extendedDynamicState3DepthClampEnable) {
        auto vkCmdSetDepthClampEnableEXT =
            (PFN_vkCmdSetDepthClampEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDepthClampEnableEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07620");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthClampEnableEXT-depthClamp-07449");
        vkCmdSetDepthClampEnableEXT(commandBuffer.handle(), VK_TRUE);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (extended_dynamic_state3_features.extendedDynamicState3PolygonMode) {
        auto vkCmdSetPolygonModeEXT =
            (PFN_vkCmdSetPolygonModeEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetPolygonModeEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_POLYGON_MODE_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07621");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPolygonModeEXT-fillModeNonSolid-07424");
        vkCmdSetPolygonModeEXT(commandBuffer.handle(), VK_POLYGON_MODE_POINT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPolygonModeEXT-polygonMode-07425");
        vkCmdSetPolygonModeEXT(commandBuffer.handle(), VK_POLYGON_MODE_FILL_RECTANGLE_NV);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (extended_dynamic_state3_features.extendedDynamicState3AlphaToOneEnable) {
        auto vkCmdSetAlphaToOneEnableEXT =
            (PFN_vkCmdSetAlphaToOneEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetAlphaToOneEnableEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07625");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetAlphaToOneEnableEXT-alphaToOne-07607");
        vkCmdSetAlphaToOneEnableEXT(commandBuffer.handle(), VK_TRUE);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (extended_dynamic_state3_features.extendedDynamicState3LogicOpEnable) {
        auto vkCmdSetLogicOpEnableEXT =
            (PFN_vkCmdSetLogicOpEnableEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetLogicOpEnableEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07626");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLogicOpEnableEXT-logicOp-07366");
        vkCmdSetLogicOpEnableEXT(commandBuffer.handle(), VK_TRUE);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation) {
        auto vkCmdSetColorBlendEquationEXT =
            (PFN_vkCmdSetColorBlendEquationEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetColorBlendEquationEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07628");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-dualSrcBlend-07357");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-dualSrcBlend-07358");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-dualSrcBlend-07359");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-dualSrcBlend-07360");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendEquationEXT-colorBlendOp-07361");
        VkColorBlendEquationEXT const equation = {VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_OP_ZERO_EXT,
                                                  VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_FACTOR_SRC1_COLOR, VK_BLEND_OP_ZERO_EXT};
        vkCmdSetColorBlendEquationEXT(commandBuffer.handle(), 0U, 1U, &equation);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (IsExtensionsEnabled(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME) &&
        extended_dynamic_state3_features.extendedDynamicState3RasterizationStream) {
        auto transform_feedback_props = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
        GetPhysicalDeviceProperties2(transform_feedback_props);

        auto vkCmdSetRasterizationStreamEXT =
            (PFN_vkCmdSetRasterizationStreamEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetRasterizationStreamEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07630");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-transformFeedback-07411");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07412");
        if (!transform_feedback_props.transformFeedbackRasterizationStreamSelect) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07413");
        }
        vkCmdSetRasterizationStreamEXT(commandBuffer.handle(), transform_feedback_props.maxTransformFeedbackStreams + 1);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (IsExtensionsEnabled(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME) &&
        extended_dynamic_state3_features.extendedDynamicState3ExtraPrimitiveOverestimationSize) {
        auto vkCmdSetExtraPrimitiveOverestimationSizeEXT = (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)vk::GetDeviceProcAddr(
            m_device->device(), "vkCmdSetExtraPrimitiveOverestimationSizeEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07632");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-extraPrimitiveOverestimationSize-07428");
        vkCmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer.handle(), -1.0F);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (IsExtensionsEnabled(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME) &&
        extended_dynamic_state3_features.extendedDynamicState3ColorBlendAdvanced) {
        auto blend_operation_advanced = LvlInitStruct<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT>();
        GetPhysicalDeviceProperties2(blend_operation_advanced);

        auto vkCmdSetColorBlendAdvancedEXT =
            (PFN_vkCmdSetColorBlendAdvancedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetColorBlendAdvancedEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07635");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        if (!blend_operation_advanced.advancedBlendNonPremultipliedSrcColor) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendAdvancedEXT-srcPremultiplied-07505");
        }
        if (!blend_operation_advanced.advancedBlendNonPremultipliedDstColor) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendAdvancedEXT-dstPremultiplied-07506");
        }
        if (!blend_operation_advanced.advancedBlendCorrelatedOverlap) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkColorBlendAdvancedEXT-blendOverlap-07507");
        }
        VkColorBlendAdvancedEXT advanced = {VK_BLEND_OP_ZERO_EXT, VK_TRUE, VK_TRUE, VK_BLEND_OVERLAP_DISJOINT_EXT, VK_FALSE};
        vkCmdSetColorBlendAdvancedEXT(commandBuffer.handle(), 0U, 1U, &advanced);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (IsExtensionsEnabled(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME) &&
        extended_dynamic_state3_features.extendedDynamicState3ProvokingVertexMode) {
        auto vkCmdSetProvokingVertexModeEXT =
            (PFN_vkCmdSetProvokingVertexModeEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetProvokingVertexModeEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07636");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetProvokingVertexModeEXT-provokingVertexMode-07447");
        vkCmdSetProvokingVertexModeEXT(commandBuffer.handle(), VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }

    if (IsExtensionsEnabled(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME) &&
        extended_dynamic_state3_features.extendedDynamicState3LineRasterizationMode) {
        auto vkCmdSetLineRasterizationModeEXT =
            (PFN_vkCmdSetLineRasterizationModeEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetLineRasterizationModeEXT");

        VkCommandBufferObj commandBuffer(m_device, m_commandPool);
        commandBuffer.begin();

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = 1;
        VkDynamicState dyn_state = VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT;
        dyn_state_ci.pDynamicStates = &dyn_state;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

        commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07637");
        vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07418");
        vkCmdSetLineRasterizationModeEXT(commandBuffer.handle(), VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07419");
        vkCmdSetLineRasterizationModeEXT(commandBuffer.handle(), VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07420");
        vkCmdSetLineRasterizationModeEXT(commandBuffer.handle(), VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT);
        m_errorMonitor->VerifyFound();
        vk::CmdEndRenderPass(commandBuffer.handle());

        commandBuffer.end();
    }
}

TEST_F(VkLayerTest, ValidateVertexInputDynamicStateDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_vertex_input_dynamic_state VUs when disabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto vkCmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetVertexInputEXT");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04807
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VERTEX_INPUT_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04807");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // VUID-vkCmdSetVertexInputEXT-None-04790
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-None-04790");
    vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ValidateVertexInputDynamicStateEnabled) {
    TEST_DESCRIPTION("Validate VK_EXT_vertex_input_dynamic_state VUs when enabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto vertex_input_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(vertex_input_dynamic_state_features);
    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) vertexInputDynamicState, skipping";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetVertexInputEXT");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VERTEX_INPUT_EXT, VK_DYNAMIC_STATE_VERTEX_INPUT_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineDynamicStateCreateInfo-pDynamicStates-01442");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // VUID-vkCmdSetVertexInputEXT-vertexBindingDescriptionCount-04791
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        std::vector<VkVertexInputBindingDescription2EXT> bindings(m_device->props.limits.maxVertexInputBindings + 1u, binding);
        for (uint32_t i = 0; i < bindings.size(); ++i) bindings[i].binding = i;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-vertexBindingDescriptionCount-04791");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), m_device->props.limits.maxVertexInputBindings + 1u, bindings.data(), 0,
                               nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-vkCmdSetVertexInputEXT-vertexAttributeDescriptionCount-04792
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0};
        std::vector<VkVertexInputAttributeDescription2EXT> attributes(m_device->props.limits.maxVertexInputAttributes + 1u,
                                                                      attribute);
        for (uint32_t i = 0; i < attributes.size(); ++i) attributes[i].location = i;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-vertexAttributeDescriptionCount-04792");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, m_device->props.limits.maxVertexInputAttributes + 1u,
                               attributes.data());
        m_errorMonitor->VerifyFound();
    }

    // VUID-vkCmdSetVertexInputEXT-binding-04793
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-binding-04793");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
        m_errorMonitor->VerifyFound();
    }

    // VUID-vkCmdSetVertexInputEXT-pVertexBindingDescriptions-04794
    {
        VkVertexInputBindingDescription2EXT bindings[2] = {
            {VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1},
            {VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1}};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-pVertexBindingDescriptions-04794");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 2, bindings, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-vkCmdSetVertexInputEXT-pVertexAttributeDescriptions-04795
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attributes[2] = {
            {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
            {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0}};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetVertexInputEXT-pVertexAttributeDescriptions-04795");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 2, attributes);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-binding-04796
    {
        VkVertexInputBindingDescription2EXT binding = {VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
                                                       nullptr,
                                                       m_device->props.limits.maxVertexInputBindings + 1u,
                                                       0,
                                                       VK_VERTEX_INPUT_RATE_VERTEX,
                                                       1};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-binding-04796");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-stride-04797
    {
        VkVertexInputBindingDescription2EXT binding = {VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
                                                       nullptr,
                                                       0,
                                                       m_device->props.limits.maxVertexInputBindingStride + 1u,
                                                       VK_VERTEX_INPUT_RATE_VERTEX,
                                                       1};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-stride-04797");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-divisor-04798
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_INSTANCE, 0};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-divisor-04798");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-divisor-04799
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_INSTANCE, 2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-divisor-04799");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputAttributeDescription2EXT-location-06228
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                                                           nullptr,
                                                           m_device->props.limits.maxVertexInputAttributes + 1u,
                                                           0,
                                                           VK_FORMAT_R32G32B32A32_SFLOAT,
                                                           0};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputAttributeDescription2EXT-location-06228");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputAttributeDescription2EXT-binding-06229
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                                                           nullptr,
                                                           0,
                                                           m_device->props.limits.maxVertexInputBindings + 1u,
                                                           VK_FORMAT_R32G32B32A32_SFLOAT,
                                                           0};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputAttributeDescription2EXT-binding-06229");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdSetVertexInputEXT-binding-04793");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputAttributeDescription2EXT-offset-06230
    if (m_device->props.limits.maxVertexInputAttributeOffset <
        std::numeric_limits<decltype(m_device->props.limits.maxVertexInputAttributeOffset)>::max()) {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
        VkVertexInputAttributeDescription2EXT attribute = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
            m_device->props.limits.maxVertexInputAttributeOffset + 1u};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputAttributeDescription2EXT-offset-06230");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputAttributeDescription2EXT-format-04805
    {
        const VkFormat format = VK_FORMAT_D16_UNORM;
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(gpu(), format, &format_props);
        if ((format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
            VkVertexInputBindingDescription2EXT binding = {
                VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT, nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1};
            VkVertexInputAttributeDescription2EXT attribute = {
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr, 0, 0, format, 0};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputAttributeDescription2EXT-format-04805");
            vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 1, &attribute);
            m_errorMonitor->VerifyFound();
        }
    }

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ValidateVertexInputDynamicStateDivisor) {
    TEST_DESCRIPTION("Validate VK_EXT_vertex_input_dynamic_state VUs when VK_EXT_vertex_attribute_divisor is enabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto vertex_attribute_divisor_features = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    auto vertex_input_dynamic_state_features =
        LvlInitStruct<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>(&vertex_attribute_divisor_features);
    auto features2 = GetPhysicalDeviceFeatures2(vertex_input_dynamic_state_features);
    if (!vertex_attribute_divisor_features.vertexAttributeInstanceRateDivisor) {
        GTEST_SKIP() << "Test requires (unsupported) vertexAttributeInstanceRateDivisor, skipping";
    }
    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) vertexInputDynamicState, skipping";
    }

    auto vertex_attribute_divisor_properties = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&vertex_attribute_divisor_properties);
    GetPhysicalDeviceProperties2(properties2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetVertexInputEXT");

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    // VUID-VkVertexInputBindingDescription2EXT-divisor-06226
    if (vertex_attribute_divisor_properties.maxVertexAttribDivisor < 0xFFFFFFFFu) {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,       nullptr, 0, 0, VK_VERTEX_INPUT_RATE_INSTANCE,
            vertex_attribute_divisor_properties.maxVertexAttribDivisor + 1u};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-divisor-06226");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // VUID-VkVertexInputBindingDescription2EXT-divisor-06227
    {
        VkVertexInputBindingDescription2EXT binding = {
            VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,  nullptr, 0, 0, VK_VERTEX_INPUT_RATE_VERTEX,
            vertex_attribute_divisor_properties.maxVertexAttribDivisor};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVertexInputBindingDescription2EXT-divisor-06227");
        vkCmdSetVertexInputEXT(m_commandBuffer->handle(), 1, &binding, 0, nullptr);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ValidateViewportStateScissorOverflow) {
    TEST_DESCRIPTION("Validate sum of offset and width of viewport state scissor");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkRect2D scissor_x = {{INT32_MAX / 2, 0}, {INT32_MAX / 2 + 64, 64}};
    VkRect2D scissor_y = {{0, INT32_MAX / 2}, {64, INT32_MAX / 2 + 64}};

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

TEST_F(VkLayerTest, WriteTimeStampInvalidQuery) {
    TEST_DESCRIPTION("Test for invalid query slot in query pool.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    bool sync2 = IsExtensionsEnabled(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2 = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>();
    synchronization2.synchronization2 = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>();
    if (sync2) {
        features2.pNext = &synchronization2;
    }
    InitState(nullptr, &features2);
    sync2 &= (synchronization2.synchronization2 == VK_TRUE);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.\n";
    }

    vk_testing::QueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    query_pool.init(*m_device, query_pool_ci);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-query-04904");
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 1);
    m_errorMonitor->VerifyFound();
    if (sync2) {
        auto vkCmdWriteTimestamp2KHR =
            (PFN_vkCmdWriteTimestamp2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdWriteTimestamp2KHR");

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp2-query-04903");
        vkCmdWriteTimestamp2KHR(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool.handle(), 1);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
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

TEST_F(VkLayerTest, ValidateColorWriteDynamicStateDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_color_write_enable VUs when disabled");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04800
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04800");
    pipe.CreateGraphicsPipeline();
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

TEST_F(VkLayerTest, ExternalMemoryAndExternalMemoryNV) {
    TEST_DESCRIPTION("Test for both external memory and external memory NV in image create pNext chain.");

    AddRequiredExtensions(VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkExternalMemoryImageCreateInfoNV external_mem_nv = LvlInitStruct<VkExternalMemoryImageCreateInfoNV>();
    external_mem_nv.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkExternalMemoryImageCreateInfo external_mem = LvlInitStruct<VkExternalMemoryImageCreateInfo>(&external_mem_nv);
    external_mem.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&external_mem);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-00988");
    VkImage test_image;
    vk::CreateImage(device(), &ici, nullptr, &test_image);
    m_errorMonitor->VerifyFound();
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-physicalDeviceCount-01421");
    VkImage test_image;
    vk::CreateImage(device(), &ici, nullptr, &test_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, QueueSubmitWaitingSameSemaphore) {
    TEST_DESCRIPTION("Submit to queue with waitSemaphore that another queue is already waiting on.");

    AddOptionalExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>();
    auto features2 = GetPhysicalDeviceFeatures2(sync2_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    if (m_device->graphics_queues().size() < 2) {
        GTEST_SKIP() << "2 graphics queues are needed";
    }

    auto sem_info = LvlInitStruct<VkSemaphoreCreateInfo>();

    vk_testing::Semaphore semaphore;
    semaphore.init(*m_device, sem_info);

    VkQueue other = m_device->graphics_queues()[1]->handle();

    {
        auto signal_submit = LvlInitStruct<VkSubmitInfo>();
        signal_submit.signalSemaphoreCount = 1;
        signal_submit.pSignalSemaphores = &semaphore.handle();

        VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        auto wait_submit = LvlInitStruct<VkSubmitInfo>();
        wait_submit.waitSemaphoreCount = 1;
        wait_submit.pWaitSemaphores = &semaphore.handle();
        wait_submit.pWaitDstStageMask = &stage_flags;

        vk::QueueSubmit(m_device->m_queue, 1, &signal_submit, VK_NULL_HANDLE);
        vk::QueueSubmit(m_device->m_queue, 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pWaitSemaphores-00068");
        vk::QueueSubmit(other, 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
        vk::QueueWaitIdle(m_device->m_queue);
        vk::QueueWaitIdle(other);
    }
    if (m_device->queue_props[m_device->m_queue_obj->get_family_index()].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        auto signal_bind = LvlInitStruct<VkBindSparseInfo>();
        signal_bind.signalSemaphoreCount = 1;
        signal_bind.pSignalSemaphores = &semaphore.handle();

        auto wait_bind = LvlInitStruct<VkBindSparseInfo>();
        wait_bind.waitSemaphoreCount = 1;
        wait_bind.pWaitSemaphores = &semaphore.handle();

        vk::QueueBindSparse(m_device->m_queue, 1, &signal_bind, VK_NULL_HANDLE);
        vk::QueueBindSparse(m_device->m_queue, 1, &wait_bind, VK_NULL_HANDLE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueBindSparse-pWaitSemaphores-01116");
        vk::QueueBindSparse(other, 1, &wait_bind, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        vk::QueueWaitIdle(m_device->m_queue);
        vk::QueueWaitIdle(other);
    }
    if (sync2_features.synchronization2) {
        auto vkQueueSubmit2KHR =
            reinterpret_cast<PFN_vkQueueSubmit2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2KHR"));

        auto signal_sem_info = LvlInitStruct<VkSemaphoreSubmitInfo>();
        signal_sem_info.semaphore = semaphore.handle();
        signal_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        auto signal_submit = LvlInitStruct<VkSubmitInfo2>();
        signal_submit.signalSemaphoreInfoCount = 1;
        signal_submit.pSignalSemaphoreInfos = &signal_sem_info;

        auto wait_sem_info = LvlInitStruct<VkSemaphoreSubmitInfo>();
        wait_sem_info.semaphore = semaphore.handle();
        wait_sem_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        auto wait_submit = LvlInitStruct<VkSubmitInfo2>();
        wait_submit.waitSemaphoreInfoCount = 1;
        wait_submit.pWaitSemaphoreInfos = &wait_sem_info;

        vkQueueSubmit2KHR(m_device->m_queue, 1, &signal_submit, VK_NULL_HANDLE);
        vkQueueSubmit2KHR(m_device->m_queue, 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit2-semaphore-03871");
        vkQueueSubmit2KHR(other, 1, &wait_submit, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        vk::QueueWaitIdle(m_device->m_queue);
        vk::QueueWaitIdle(other);
    }
}

TEST_F(VkLayerTest, QueueSubmit2KHRUsedButSynchronizaion2Disabled) {
    TEST_DESCRIPTION("Using QueueSubmit2KHR when synchronization2 is not enabled");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto fpQueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2KHR");
    auto fpQueueSubmit2 = (PFN_vkQueueSubmit2)vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2");

    auto submit_info = LvlInitStruct<VkSubmitInfo2KHR>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit2-synchronization2-03866");
    fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit2-synchronization2-03866");
        fpQueueSubmit2(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, WaitEventsDifferentQueues) {
    TEST_DESCRIPTION("Using CmdWaitEvents with invalid barrier queues");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t no_gfx = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (no_gfx == UINT32_MAX) {
        GTEST_SKIP() << "Required queue families not present (non-graphics non-compute capable required)";
    }

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    VkBufferObj buffer;
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    buffer.init_as_src_and_dst(*m_device, 256, mem_prop);

    VkBufferMemoryBarrier buffer_memory_barrier = LvlInitStruct<VkBufferMemoryBarrier>();
    buffer_memory_barrier.srcAccessMask = 0;
    buffer_memory_barrier.dstAccessMask = 0;
    buffer_memory_barrier.buffer = buffer.handle();
    buffer_memory_barrier.offset = 0;
    buffer_memory_barrier.size = 256;
    buffer_memory_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    buffer_memory_barrier.dstQueueFamilyIndex = no_gfx;

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    VkImageMemoryBarrier image_memory_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    image_memory_barrier.srcAccessMask = 0;
    image_memory_barrier.dstAccessMask = 0;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_memory_barrier.image = image.handle();
    image_memory_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_memory_barrier.dstQueueFamilyIndex = no_gfx;
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.layerCount = 1;
    image_memory_barrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                      nullptr, 1, &buffer_memory_barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                      nullptr, 0, nullptr, 1, &image_memory_barrier);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    vk::DestroyEvent(m_device->handle(), event, nullptr);
}

TEST_F(VkLayerTest, InvalidColorWriteEnableFeature) {
    TEST_DESCRIPTION("Invalid usage of vkCmdSetColorWriteEnableEXT with feature not enabled");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkBool32 color_write_enable[2] = {VK_TRUE, VK_FALSE};

    PFN_vkCmdSetColorWriteEnableEXT vkCmdSetColorWriteEnableEXT =
        (PFN_vkCmdSetColorWriteEnableEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdSetColorWriteEnableEXT");

    CreatePipelineHelper helper(*this);
    helper.InitInfo();
    helper.InitState();
    helper.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetColorWriteEnableEXT-None-04803");
    vkCmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 1, color_write_enable);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidColorWriteEnableAttachmentCount) {
    TEST_DESCRIPTION("Invalid usage of attachmentCount for vkCmdSetColorWriteEnableEXT");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Feature required to be supported for extension
    VkPhysicalDeviceColorWriteEnableFeaturesEXT color_write_features = LvlInitStruct<VkPhysicalDeviceColorWriteEnableFeaturesEXT>();
    color_write_features.colorWriteEnable = VK_TRUE;
    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&color_write_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // need a valid array to index into
    std::vector<VkBool32> color_write_enable(m_device->props.limits.maxColorAttachments + 1, VK_TRUE);

    PFN_vkCmdSetColorWriteEnableEXT vkCmdSetColorWriteEnableEXT =
        (PFN_vkCmdSetColorWriteEnableEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdSetColorWriteEnableEXT");

    CreatePipelineHelper helper(*this);
    helper.InitInfo();
    helper.InitState();
    helper.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);

    // Value can't be zero
    // TODO: The generated code is not use the correct implicit VUID, but at least its still correctly validating
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetColorWriteEnableEXT-attachmentCount-arraylength");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID_Undefined");
    vkCmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 0, color_write_enable.data());
    m_errorMonitor->VerifyFound();

    // over the limit
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetColorWriteEnableEXT-attachmentCount-06656");
    vkCmdSetColorWriteEnableEXT(m_commandBuffer->handle(), m_device->props.limits.maxColorAttachments + 1,
                                color_write_enable.data());
    m_errorMonitor->VerifyFound();

    // mismatch of attachmentCount value is allowed for dynamic
    // see https://gitlab.khronos.org/vulkan/vulkan/-/issues/2868
    vkCmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 2, color_write_enable.data());

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidCmdSetDiscardRectangleEXTRectangleCount) {
    TEST_DESCRIPTION("Test CmdSetDiscardRectangleEXT with invalid offsets in pDiscardRectangles");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties =
        LvlInitStruct<VkPhysicalDeviceDiscardRectanglePropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>();
    phys_dev_props_2.pNext = &discard_rectangle_properties;
    GetPhysicalDeviceProperties2(phys_dev_props_2);

    auto fpCmdSetDiscardRectangleEXT =
        (PFN_vkCmdSetDiscardRectangleEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDiscardRectangleEXT");

    VkRect2D discard_rectangles = {};
    discard_rectangles.offset.x = 0;
    discard_rectangles.offset.y = 0;
    discard_rectangles.extent.width = 64;
    discard_rectangles.extent.height = 64;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-firstDiscardRectangle-00585");
    fpCmdSetDiscardRectangleEXT(m_commandBuffer->handle(), discard_rectangle_properties.maxDiscardRectangles, 1,
                                &discard_rectangles);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdEndQueryIndexedEXTIndex) {
    TEST_DESCRIPTION("Test InvalidCmdEndQueryIndexedEXT with invalid index");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    InitState();

    auto transform_feedback_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    GetPhysicalDeviceProperties2(transform_feedback_properties);
    if (transform_feedback_properties.maxTransformFeedbackStreams < 1) {
        GTEST_SKIP() << "maxTransformFeedbackStreams < 1";
    }

    auto fpCmdBeginQueryIndexedEXT =
        (PFN_vkCmdBeginQueryIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT");
    ASSERT_NE(fpCmdBeginQueryIndexedEXT, nullptr);
    auto fpCmdEndQueryIndexedEXT =
        (PFN_vkCmdEndQueryIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndQueryIndexedEXT");
    ASSERT_NE(fpCmdEndQueryIndexedEXT, nullptr);

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    qpci.queryCount = 1;

    vk_testing::QueryPool tf_query_pool(*m_device, qpci);
    ASSERT_TRUE(tf_query_pool.initialized());

    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    vk_testing::QueryPool query_pool(*m_device, qpci);
    ASSERT_TRUE(query_pool.initialized());

    m_commandBuffer->begin();
    fpCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-02346");
    fpCmdEndQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0,
                            transform_feedback_properties.maxTransformFeedbackStreams);

    fpCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-02347");
    fpCmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-None-02342");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-query-02343");
    fpCmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 1, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdEndQueryIndexedEXTPrimitiveGenerated) {
    TEST_DESCRIPTION("Test InvalidCmdEndQueryIndexedEXT with invalid index");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto primitives_generated_features = LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto transform_feedback_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_properties);
    GetPhysicalDeviceProperties2(phys_dev_props_2);

    auto vkCmdBeginQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdBeginQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT"));
    auto vkCmdEndQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdEndQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndQueryIndexedEXT"));

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryCount = 1;

    vk_testing::QueryPool tf_query_pool;
    qpci.queryType = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    tf_query_pool.init(*m_device, qpci);

    vk_testing::QueryPool pg_query_pool;
    qpci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    pg_query_pool.init(*m_device, qpci);

    vk_testing::QueryPool query_pool;
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool.init(*m_device, qpci);

    m_commandBuffer->begin();

    vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0, 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06694");
    vkCmdEndQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0,
                            transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    if (!primitives_generated_features.primitivesGeneratedQueryWithNonZeroStreams) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06691");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06690");
    vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), pg_query_pool.handle(), 0, 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06695");
    vkCmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
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

TEST_F(VkLayerTest, InvalidCmdSetDiscardRectangleEXTOffsets) {
    TEST_DESCRIPTION("Test CmdSetDiscardRectangleEXT with invalid offsets in pDiscardRectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties =
        LvlInitStruct<VkPhysicalDeviceDiscardRectanglePropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>();
    phys_dev_props_2.pNext = &discard_rectangle_properties;
    GetPhysicalDeviceProperties2(phys_dev_props_2);

    auto fpCmdSetDiscardRectangleEXT =
        (PFN_vkCmdSetDiscardRectangleEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDiscardRectangleEXT");

    if (discard_rectangle_properties.maxDiscardRectangles == 0) {
        GTEST_SKIP() << "Discard rectangles are not supported";
    }

    VkRect2D discard_rectangles = {};
    discard_rectangles.offset.x = -1;
    discard_rectangles.offset.y = 0;
    discard_rectangles.extent.width = 64;
    discard_rectangles.extent.height = 64;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-x-00587");
    fpCmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    m_errorMonitor->VerifyFound();

    discard_rectangles.offset.x = 0;
    discard_rectangles.offset.y = -32;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-x-00587");
    fpCmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BeginQueryTypeTransformFeedbackStream) {
    TEST_DESCRIPTION(
        "Call CmdBeginQuery with query type transform feedback stream when transformFeedbackQueries is not supported.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props =
        LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    VkPhysicalDeviceProperties2 phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_props);

    GetPhysicalDeviceProperties2(phys_dev_props_2);
    if (transform_feedback_props.transformFeedbackQueries) {
        GTEST_SKIP() << "Transform feedback queries are supported";
    }

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-02328");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, CmdSetDiscardRectangleEXTRectangleCountOverflow) {
    TEST_DESCRIPTION("Test CmdSetDiscardRectangleEXT with invalid offsets in pDiscardRectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto vkCmdSetDiscardRectangleEXT = GetDeviceProcAddr<PFN_vkCmdSetDiscardRectangleEXT>("vkCmdSetDiscardRectangleEXT");

    VkRect2D discard_rectangles = {};
    discard_rectangles.offset.x = 1;
    discard_rectangles.offset.y = 0;
    discard_rectangles.extent.width = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());
    discard_rectangles.extent.height = 64;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-offset-00588");
    vkCmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    m_errorMonitor->VerifyFound();

    discard_rectangles.offset.x = 0;
    discard_rectangles.offset.y = std::numeric_limits<int32_t>::max();
    discard_rectangles.extent.width = 64;
    discard_rectangles.extent.height = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDiscardRectangleEXT-offset-00589");
    vkCmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 1, &discard_rectangles);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, GetQueryPoolResultsFlags) {
    TEST_DESCRIPTION("Test GetQueryPoolResults with invalid pData and stride");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    vk_testing::QueryPool query_pool;
    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 1;
    query_pool.init(*m_device, qpci);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];

    VkQueryResultFlags flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-04811");
    vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, out_data_size, data + 1, 4, flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, QueryPoolResultStatusOnly) {
    TEST_DESCRIPTION("Request result status only query result.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR;
    query_pool_ci.queryCount = 1;
    VkResult err = vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);
    if (err != VK_SUCCESS) {
        GTEST_SKIP() << "Required query not supported";
    }

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-04810");
    vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 1, out_data_size, &data, sizeof(uint32_t), 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerTest, DestroyActiveQueryPool) {
    TEST_DESCRIPTION("Destroy query pool after GetQueryPoolResults() without VK_QUERY_RESULT_PARTIAL_BIT returns VK_SUCCESS");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        GTEST_SKIP() << "Device graphic queue has timestampValidBits of 0, skipping.";
    }

    VkQueryPoolCreateInfo query_pool_create_info = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &query_pool_create_info, nullptr, &query_pool);

    auto cmd_begin = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    m_commandBuffer->begin(&cmd_begin);
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool, 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    VkResult res;
    do {
        res = vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 1, out_data_size, &data, 4, 0);
    } while (res != VK_SUCCESS);

    // Submit the command buffer again, making query pool in use and invalid to destroy
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyQueryPool-queryPool-00793");
    vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
    vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
}

TEST_F(VkLayerTest, ValidateExternalMemoryImageLayout) {
    TEST_DESCRIPTION("Validate layout of image with external memory");

#ifdef _WIN32
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    VkExternalMemoryImageCreateInfo external_mem = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    external_mem.handleTypes = handle_type;

    VkExternalMemoryImageCreateInfoNV external_mem_nv = LvlInitStruct<VkExternalMemoryImageCreateInfoNV>();
    external_mem_nv.handleTypes = handle_type;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&external_mem);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {32, 32, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImage test_image;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-01443");
    vk::CreateImage(device(), &ici, nullptr, &test_image);
    m_errorMonitor->VerifyFound();

    ici.pNext = &external_mem_nv;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-01443");
    vk::CreateImage(device(), &ici, nullptr, &test_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateCreateSamplerWithBorderColorSwizzle) {
    TEST_DESCRIPTION("Validate vkCreateSampler with VkSamplerBorderColorComponentMappingCreateInfoEXT");

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSamplerBorderColorComponentMappingCreateInfoEXT border_color_component_mapping =
        LvlInitStruct<VkSamplerBorderColorComponentMappingCreateInfoEXT>();
    border_color_component_mapping.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                 VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};

    VkSamplerCreateInfo sampler_create_info = SafeSaneSamplerCreateInfo();
    sampler_create_info.pNext = &border_color_component_mapping;

    VkSampler sampler = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSamplerBorderColorComponentMappingCreateInfoEXT-borderColorSwizzle-06437");
    vk::CreateSampler(device(), &sampler_create_info, nullptr, &sampler);
    m_errorMonitor->VerifyFound();

    vk::DestroySampler(device(), sampler, nullptr);
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

TEST_F(VkLayerTest, BeginQueryWithMultiview) {
    TEST_DESCRIPTION("Test CmdBeginQuery in subpass with multiview");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto features_1_1 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    auto features2 = GetPhysicalDeviceFeatures2(features_1_1);
    if (!features_1_1.multiview) {
        GTEST_SKIP() << "Test requires VkPhysicalDeviceVulkan11Features::multiview feature.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference color_att = {};
    color_att.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_att;

    uint32_t viewMasks[] = {0x3u};
    uint32_t correlationMasks[] = {0x1u};
    auto rpmv_ci = LvlInitStruct<VkRenderPassMultiviewCreateInfo>();
    rpmv_ci.subpassCount = 1;
    rpmv_ci.pViewMasks = viewMasks;
    rpmv_ci.correlationMaskCount = 1;
    rpmv_ci.pCorrelationMasks = correlationMasks;

    auto rp_ci = LvlInitStruct<VkRenderPassCreateInfo>(&rpmv_ci);
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attach;
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, rp_ci);

    VkImageObj image(m_device);
    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 4;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image.init(&image_ci);
    ASSERT_TRUE(image.initialized());

    vk_testing::ImageView image_view;
    VkImageViewCreateInfo iv_ci = LvlInitStruct<VkImageViewCreateInfo>();
    iv_ci.image = image.handle();
    iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    iv_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    iv_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv_ci.subresourceRange.baseMipLevel = 0;
    iv_ci.subresourceRange.levelCount = 1;
    iv_ci.subresourceRange.baseArrayLayer = 0;
    iv_ci.subresourceRange.layerCount = 4;
    image_view.init(*m_device, iv_ci);

    VkImageView image_view_handle = image_view.handle();

    auto fb_ci = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_ci.renderPass = render_pass.handle();
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = &image_view_handle;
    fb_ci.width = 64;
    fb_ci.height = 64;
    fb_ci.layers = 1;

    vk_testing::Framebuffer framebuffer;
    framebuffer.init(*m_device, fb_ci);

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 2;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, qpci);

    auto rp_begin = LvlInitStruct<VkRenderPassBeginInfo>();
    rp_begin.renderPass = render_pass.handle();
    rp_begin.framebuffer = framebuffer.handle();
    rp_begin.renderArea.extent = {64, 64};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_begin);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-query-00808");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 1, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, PipelineStatisticsQuery) {
    TEST_DESCRIPTION("Test unsupported pipeline statistics queries");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t graphics_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT);
    uint32_t compute_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (graphics_queue_family_index == std::numeric_limits<uint32_t>::max() &&
        compute_queue_family_index == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "required queue families not found";
    }

    if (graphics_queue_family_index != std::numeric_limits<uint32_t>::max()) {
        VkCommandPoolObj command_pool(m_device, graphics_queue_family_index);

        VkCommandBufferObj command_buffer(m_device, &command_pool);
        command_buffer.begin();

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_ci.queryCount = 1;
        query_pool_ci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

        vk_testing::QueryPool query_pool;
        query_pool.init(*m_device, query_pool_ci);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00805");
        vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        command_buffer.end();
    }

    if (compute_queue_family_index != std::numeric_limits<uint32_t>::max()) {
        VkCommandPoolObj command_pool(m_device, compute_queue_family_index);

        VkCommandBufferObj command_buffer(m_device, &command_pool);
        command_buffer.begin();

        VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
        query_pool_ci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_ci.queryCount = 1;
        query_pool_ci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;

        vk_testing::QueryPool query_pool;
        query_pool.init(*m_device, query_pool_ci);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00804");
        vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
        m_errorMonitor->VerifyFound();

        command_buffer.end();
    }
}

TEST_F(VkLayerTest, TestGetQueryPoolResultsDataAndStride) {
    TEST_DESCRIPTION("Test pData and stride multiple in GetQueryPoolResults");

    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    vk_testing::QueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    query_pool.init(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-02828");
    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, out_data_size, &data, 3, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDeviceQueueFamilyIndex) {
    TEST_DESCRIPTION("Create device queue with invalid queue family index.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    uint32_t queue_family_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, queue_props.data());

    uint32_t queue_family_index = queue_family_count;


    float priority = 1.0f;
    auto device_queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    device_queue_ci.queueFamilyIndex = queue_family_index;
    device_queue_ci.queueCount = 1;
    device_queue_ci.pQueuePriorities = &priority;

    auto device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &device_queue_ci;
    device_ci.enabledLayerCount = 0;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381");
    VkDevice device;
    vk::CreateDevice(gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateColorWriteDynamicStateNotSet) {
    TEST_DESCRIPTION("Validate dynamic state color write enable was set before draw command");

    AddRequiredExtensions(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto color_write_enable_features = LvlInitStruct<VkPhysicalDeviceColorWriteEnableFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(color_write_enable_features);

    if (color_write_enable_features.colorWriteEnable == VK_FALSE) {
        GTEST_SKIP() << "colorWriteEnable feature is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(2));

    auto vkCmdSetColorWriteEnableEXT =
        reinterpret_cast<PFN_vkCmdSetColorWriteEnableEXT>(vk::GetDeviceProcAddr(m_device->handle(), "vkCmdSetColorWriteEnableEXT"));

    VkPipelineColorBlendAttachmentState color_blend[2] = {};
    color_blend[0].blendEnable = VK_TRUE;
    color_blend[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    color_blend[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    color_blend[0].colorBlendOp = VK_BLEND_OP_ADD;
    color_blend[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend[0].alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend[1].blendEnable = VK_TRUE;
    color_blend[1].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    color_blend[1].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    color_blend[1].colorBlendOp = VK_BLEND_OP_ADD;
    color_blend[1].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend[1].alphaBlendOp = VK_BLEND_OP_ADD;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cb_ci_.attachmentCount = 2;
    pipe.cb_ci_.pAttachments = color_blend;
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07749");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBool32 color_write_enable[] = {VK_TRUE, VK_FALSE};
    vkCmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 1, color_write_enable);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-attachmentCount-07750");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vkCmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 2, color_write_enable);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ValidateDiscardRectanglesDynamicStateNotSet) {
    TEST_DESCRIPTION("Validate dynamic state for VK_EXT_discard_rectangles");

    AddRequiredExtensions(VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdSetDiscardRectangleEXT =
        (PFN_vkCmdSetDiscardRectangleEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDiscardRectangleEXT");

    auto discard_rect_ci = LvlInitStruct<VkPipelineDiscardRectangleStateCreateInfoEXT>();
    discard_rect_ci.discardRectangleMode = VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT;
    discard_rect_ci.discardRectangleCount = 4;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &discard_rect_ci;
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT};
    auto dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = size(dyn_states);
    dyn_state_ci.pDynamicStates = dyn_states;
    pipe.dyn_state_ci_ = dyn_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07751");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // only fill in [0, 1, 3] index
    VkRect2D discard_rectangles[2] = {{{0, 0}, {16, 16}}, {{0, 0}, {16, 16}}};
    vkCmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 0, 2, discard_rectangles);
    vkCmdSetDiscardRectangleEXT(m_commandBuffer->handle(), 3, 1, discard_rectangles);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-07751");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InstanceCreateEnumeratePortability) {
    TEST_DESCRIPTION("Validate creating instances with VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR.");

    auto ici = GetInstanceCreateInfo();
    ici.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VkInstance local_instance;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkInstanceCreateInfo-flags-06559");
    vk::CreateInstance(&ici, nullptr, &local_instance);
    m_errorMonitor->VerifyFound();

    if (InstanceExtensionSupported(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        std::vector<const char *> enabled_extensions;
        for (uint32_t i = 0; i < ici.enabledExtensionCount; ++i) {
            enabled_extensions.push_back(ici.ppEnabledExtensionNames[i]);
        }
        enabled_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        ici.enabledExtensionCount++;
        ici.ppEnabledExtensionNames = enabled_extensions.data();

        vk::CreateInstance(&ici, nullptr, &local_instance);
    }
}

TEST_F(VkLayerTest, MismatchedDeviceQueueGlobalPriority) {
    TEST_DESCRIPTION("Create multiple device queues with same queue family index but different global priorty.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    uint32_t queue_family_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props(queue_family_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, queue_props.data());

    uint32_t queue_family_index = queue_family_count;

    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if (queue_props[i].queueCount > 1) {
            queue_family_index = i;
            break;
        }
    }
    if (queue_family_index == queue_family_count) {
        GTEST_SKIP() << "Multiple queues from same queue family are required to run this test";
    }

    VkDeviceQueueGlobalPriorityCreateInfoKHR queue_global_priority_ci[2] = {};
    queue_global_priority_ci[0] = LvlInitStruct<VkDeviceQueueGlobalPriorityCreateInfoKHR>();
    queue_global_priority_ci[0].globalPriority = VK_QUEUE_GLOBAL_PRIORITY_LOW_KHR;
    queue_global_priority_ci[1] = LvlInitStruct<VkDeviceQueueGlobalPriorityCreateInfoKHR>();
    queue_global_priority_ci[1].globalPriority = VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR;

    float priorities[] = {1.0f, 1.0f};
    VkDeviceQueueCreateInfo device_queue_ci[2] = {};
    device_queue_ci[0] = LvlInitStruct<VkDeviceQueueCreateInfo>(&queue_global_priority_ci[0]);
    device_queue_ci[0].queueFamilyIndex = queue_family_index;
    device_queue_ci[0].queueCount = 1;
    device_queue_ci[0].pQueuePriorities = &priorities[0];

    device_queue_ci[1] = LvlInitStruct<VkDeviceQueueCreateInfo>(&queue_global_priority_ci[1]);
    device_queue_ci[1].queueFamilyIndex = queue_family_index;
    device_queue_ci[1].queueCount = 1;
    device_queue_ci[1].pQueuePriorities = &priorities[1];

    auto device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = 2;
    device_ci.pQueueCreateInfos = device_queue_ci;
    device_ci.enabledLayerCount = 0;
    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-queueFamilyIndex-02802");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pQueueCreateInfos-06654");
    VkDevice device;
    vk::CreateDevice(gpu(), &device_ci, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PrimitivesGeneratedQuery) {
    TEST_DESCRIPTION("Test unsupported primitives generated queries");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto primitives_generated_features = LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported";
    }
    primitives_generated_features.primitivesGeneratedQueryWithNonZeroStreams = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    auto transform_feedback_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_properties);
    GetPhysicalDeviceProperties2(phys_dev_props_2);

    uint32_t compute_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (compute_queue_family_index == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "required queue family not found, skipping test";
    }
    VkCommandPoolObj command_pool(m_device, compute_queue_family_index);

    VkCommandBufferObj command_buffer(m_device, &command_pool);
    command_buffer.begin();

    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    query_pool_ci.queryCount = 1;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, query_pool_ci);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-06687");
    vk::CmdBeginQuery(command_buffer.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    auto fpvkCmdBeginQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdBeginQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT"));
    auto fpvkCmdEndQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdEndQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndQueryIndexedEXT"));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06689");
    fpvkCmdBeginQueryIndexedEXT(command_buffer.handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06690");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06691");
    fpvkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0,
                                transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;

    vk_testing::QueryPool occlusion_query_pool;
    occlusion_query_pool.init(*m_device, query_pool_ci);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06692");
    fpvkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), occlusion_query_pool.handle(), 0, 0, 1);
    m_errorMonitor->VerifyFound();

    fpvkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06696");
    fpvkCmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();

}

TEST_F(VkLayerTest, PrimitivesGeneratedQueryFeature) {
    TEST_DESCRIPTION("Test missing primitives generated query feature");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto primitives_generated_features = LvlInitStruct<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>();
    GetPhysicalDeviceFeatures2(primitives_generated_features);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        GTEST_SKIP() << "primitivesGeneratedQuery feature is not supported";
    }
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    query_pool_ci.queryCount = 1;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, query_pool_ci);
    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-06688");
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    auto fpvkCmdBeginQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdBeginQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT"));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06693");
    fpvkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, IncompatibleRenderPass) {
    TEST_DESCRIPTION("Validate if attachments in render pass and descriptor set use the same image subresources");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t width = 32;
    const uint32_t height = 32;
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

    VkSubpassDependency dependency = {0,
                                      0,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                      VK_DEPENDENCY_BY_REGION_BIT};

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;

    vk_testing::RenderPass render_pass1(*m_device, rpci);
    rpci.dependencyCount = 0;
    vk_testing::RenderPass render_pass2(*m_device, rpci);
    rpci.dependencyCount = 1;
    dependency.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    vk_testing::RenderPass render_pass3(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(width, height, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto fb_ci = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_ci.renderPass = render_pass1.handle();
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = &imageView;
    fb_ci.width = width;
    fb_ci.height = height;
    fb_ci.layers = 1;
    vk_testing::Framebuffer framebuffer;
    framebuffer.init(*m_device, fb_ci);

    VkClearValue clear_values[2] = {};
    clear_values[0].color = {{0, 0, 0, 0}};
    clear_values[1].color = {{0, 0, 0, 0}};

    VkRenderPassBeginInfo rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderPass = render_pass2.handle();
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;
    rpbi.clearValueCount = 2;
    rpbi.pClearValues = clear_values;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderPass-00904");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    rpbi.renderPass = render_pass3.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderPass-00904");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderPass-00904");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, IncompatibleRenderPass2) {
    TEST_DESCRIPTION("Validate if attachments in render pass and descriptor set use the same image subresources");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference2 attach_ref = LvlInitStruct<VkAttachmentReference2>();
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;
    subpass.viewMask = 0x1;

    VkAttachmentDescription2 attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDependency2 dependency = {VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
                                       nullptr,
                                       0,
                                       0,
                                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                       VK_DEPENDENCY_BY_REGION_BIT};

    uint32_t correlated_view_mask = 0x1;
    VkRenderPassCreateInfo2 rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;
    rpci.correlatedViewMaskCount = 1;
    rpci.pCorrelatedViewMasks = &correlated_view_mask;

    vk_testing::RenderPass render_pass1(*m_device, rpci);
    rpci.correlatedViewMaskCount = 0;
    vk_testing::RenderPass render_pass2(*m_device, rpci);
    rpci.correlatedViewMaskCount = 1;
    correlated_view_mask = 0x2;
    vk_testing::RenderPass render_pass3(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(width, height, 1, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto fb_ci = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_ci.renderPass = render_pass1.handle();
    fb_ci.attachmentCount = 1;
    fb_ci.pAttachments = &imageView;
    fb_ci.width = width;
    fb_ci.height = height;
    fb_ci.layers = 1;
    vk_testing::Framebuffer framebuffer;
    framebuffer.init(*m_device, fb_ci);

    VkClearValue clear_values[2] = {};
    clear_values[0].color = {{0, 0, 0, 0}};
    clear_values[1].color = {{0, 0, 0, 0}};

    VkRenderPassBeginInfo rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderPass = render_pass2.handle();
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;
    rpbi.clearValueCount = 2;
    rpbi.pClearValues = clear_values;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderPass-00904");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    rpbi.renderPass = render_pass3.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderPass-00904");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
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

    PFN_vkExportMetalObjectsEXT vkExportMetalObjectsEXT =
        reinterpret_cast<PFN_vkExportMetalObjectsEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkExportMetalObjectsEXT"));
    ASSERT_TRUE(vkExportMetalObjectsEXT != nullptr);

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
    VkImage image;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-06783");
    vk::CreateImage(device(), &ici, NULL, &image);
    m_errorMonitor->VerifyFound();

    auto import_metal_texture_info = LvlInitStruct<VkImportMetalTextureInfoEXT>();
    import_metal_texture_info.plane = VK_IMAGE_ASPECT_COLOR_BIT;
    ici.pNext = &import_metal_texture_info;
    ici.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-06784");
    vk::CreateImage(device(), &ici, NULL, &image);
    m_errorMonitor->VerifyFound();

    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    import_metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_1_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-06785");
    vk::CreateImage(device(), &ici, NULL, &image);
    m_errorMonitor->VerifyFound();

    ici.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    import_metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_2_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-06786");
    vk::CreateImage(device(), &ici, NULL, &image);
    m_errorMonitor->VerifyFound();

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
    VkBufferView buffer_view;
    buff_view_ci.pNext = &metal_object_create_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferViewCreateInfo-pNext-06782");
    vk::CreateBufferView(device(), &buff_view_ci, NULL, &buffer_view);
    m_errorMonitor->VerifyFound();

    VkImageObj image_obj(m_device);
    image_obj.Init(256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView image_view;
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image_obj.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.pNext = &metal_object_create_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-06787");
    vk::CreateImageView(m_device->device(), &ivci, nullptr, &image_view);
    m_errorMonitor->VerifyFound();

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
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    export_metal_objects_info.pNext = &metal_command_queue_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06792");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    alloc_info.pNext = nullptr;
    VkResult err = vk::AllocateMemory(device(), &alloc_info, nullptr, &memory);
    ASSERT_VK_SUCCESS(err);
    auto metal_buffer_info = LvlInitStruct<VkExportMetalBufferInfoEXT>();
    metal_buffer_info.memory = memory;
    export_metal_objects_info.pNext = &metal_buffer_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06793");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
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
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    // Image not created with struct in pNext
    metal_texture_info.bufferView = VK_NULL_HANDLE;
    metal_texture_info.image = image_obj.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06795");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    metal_texture_info.image = VK_NULL_HANDLE;
    vk_testing::ImageView image_view_no_struct;
    auto image_view_ci = image_obj.TargetViewCI(VK_FORMAT_B8G8R8A8_UNORM);
    image_view_ci.image = image_obj.handle();
    image_view_no_struct.init(*m_device, image_view_ci);
    metal_texture_info.imageView = image_view_no_struct.handle();
    // ImageView not created with struct in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06796");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    buff_view_ci.pNext = nullptr;
    vk_testing::BufferView buffer_view_no_struct;
    buffer_view_no_struct.init(*m_device, buff_view_ci);
    metal_texture_info.imageView = VK_NULL_HANDLE;
    metal_texture_info.bufferView = buffer_view_no_struct.handle();
    // BufferView not created with struct in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06797");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    metal_texture_info.bufferView = VK_NULL_HANDLE;
    metal_texture_info.image = export_image_obj.handle();
    metal_texture_info.plane = VK_IMAGE_ASPECT_COLOR_BIT;
    // metal_texture_info.plane not plane 0, 1 or 2
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06798");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageObj single_plane_export_image_obj(m_device);
    single_plane_export_image_obj.Init(ici);
    metal_texture_info.plane = VK_IMAGE_ASPECT_PLANE_1_BIT;
    metal_texture_info.image = single_plane_export_image_obj.handle();
    // metal_texture_info.plane not plane_0 for single plane image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06799");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    image_view_ci.pNext = &export_metal_object_create_info;
    export_metal_object_create_info.exportObjectType = VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT;
    vk_testing::ImageView single_plane_export_image_view;
    single_plane_export_image_view.init(*m_device, image_view_ci);
    metal_texture_info.image = VK_NULL_HANDLE;
    metal_texture_info.imageView = single_plane_export_image_view.handle();
    // metal_texture_info.plane not plane_0 for single plane imageView
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06801");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    auto metal_iosurface_info = LvlInitStruct<VkExportMetalIOSurfaceInfoEXT>();
    metal_iosurface_info.image = image_obj.handle();
    export_metal_objects_info.pNext = &metal_iosurface_info;
    // metal_iosurface_info.image not created with struct in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06803");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    auto metal_shared_event_info = LvlInitStruct<VkExportMetalSharedEventInfoEXT>();
    export_metal_objects_info.pNext = &metal_shared_event_info;
    // metal_shared_event_info event and semaphore both VK_NULL_HANDLE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06804");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    sem_info.pNext = nullptr;
    vk_testing::Semaphore semaphore_no_struct;
    semaphore_no_struct.init(*m_device, sem_info);
    metal_shared_event_info.semaphore = semaphore_no_struct.handle();
    export_metal_objects_info.pNext = &metal_shared_event_info;
    // Semaphore not created with struct in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06805");
    vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
    m_errorMonitor->VerifyFound();

    if (portability_features.events) {
        event_info.pNext = nullptr;
        vk_testing::Event event_no_struct;
        event_no_struct.init(*m_device, event_info);
        metal_shared_event_info.event = event_no_struct.handle();
        metal_shared_event_info.semaphore = VK_NULL_HANDLE;
        // Event not created with struct in pNext
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExportMetalObjectsInfoEXT-pNext-06806");
        vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
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
        vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
        m_errorMonitor->VerifyFound();

        if (ycbcr_conversion_extension) {
            PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionKHR =
                reinterpret_cast<PFN_vkCreateSamplerYcbcrConversionKHR>(
                    vk::GetDeviceProcAddr(m_device->device(), "vkCreateSamplerYcbcrConversionKHR"));
            PFN_vkDestroySamplerYcbcrConversionKHR vkDestroySamplerYcbcrConversionKHR =
                reinterpret_cast<PFN_vkDestroySamplerYcbcrConversionKHR>(
                    vk::GetDeviceProcAddr(m_device->device(), "vkDestroySamplerYcbcrConversionKHR"));
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
            err = vkCreateSamplerYcbcrConversionKHR(m_device->device(), &ycbcr_create_info, nullptr, &conversion);
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
            vkExportMetalObjectsEXT(m_device->handle(), &export_metal_objects_info);
            m_errorMonitor->VerifyFound();
            vkDestroySamplerYcbcrConversionKHR(m_device->device(), conversion, nullptr);
        }
    }
}
#endif  // VK_USE_PLATFORM_METAL_EXT
