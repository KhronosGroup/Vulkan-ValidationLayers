/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
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
        filter_setting = {static_cast<VkStructureType>(VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT), nullptr, 1,
                          &filter_setting_val};
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
        custom_stype_setting = {static_cast<VkStructureType>(VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT), nullptr, 1,
                                &custom_stype_setting_val};
    }

    CustomStypeList(const std::vector<uint32_t> &stype_id_array) {
        local_vector = stype_id_array;
        custom_stype_value.arrayInt32.pInt32Array = local_vector.data();
        custom_stype_value.arrayInt32.count = local_vector.size();

        strncpy(custom_stype_setting_val.name, "custom_stype_list", sizeof(custom_stype_setting_val.name));
        custom_stype_setting_val.type = VK_LAYER_SETTING_VALUE_TYPE_UINT32_ARRAY_EXT;
        custom_stype_setting_val.data = custom_stype_value;
        custom_stype_setting = {static_cast<VkStructureType>(VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT), nullptr, 1,
                                &custom_stype_setting_val};
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
        limit_setting = {static_cast<VkStructureType>(VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT), nullptr, 1,
                         &limit_setting_val};
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

    PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2 =
        (PFN_vkGetPhysicalDeviceProperties2)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2");
    assert(vkGetPhysicalDeviceProperties2);

    VkPhysicalDeviceProperties2 phys_dev_props_2{};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-API-Version-Violation");
    vkGetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnsupportedPnextApiVersion) {
    TEST_DESCRIPTION("Validate that newer pnext structs are not valid for old Vulkan versions.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    ASSERT_NO_FATAL_FAILURE(Init());
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
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
            m_errorMonitor->ExpectSuccess();
            vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
            m_errorMonitor->VerifyNotFound();
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

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto private_data_features = LvlInitStruct<VkPhysicalDevicePrivateDataFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&private_data_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (private_data_features.privateData == VK_FALSE) {
        printf("%s privateData feature is not supported.\n", kSkipPrefix);
        return;
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
        printf("%s Failed to create private data slot, VkResult %d.\n", kSkipPrefix, err);
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
        printf("%s Failed to set private data. VkResult = %d\n", kSkipPrefix, err);
    }
    m_errorMonitor->ExpectSuccess();
    uint64_t data;
    pfn_vkGetPrivateDataEXT(m_device->handle(), VK_OBJECT_TYPE_SAMPLER, (uint64_t)sampler, data_slot, &data);
    if (data != data_value) {
        m_errorMonitor->SetError("Got unexpected private data, %s.\n");
    }
    pfn_vkDestroyPrivateDataSlotEXT(m_device->handle(), data_slot, NULL);
    vk::DestroySampler(m_device->handle(), sampler, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, PrivateDataFeature) {
    TEST_DESCRIPTION("Test privateData feature not being enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_PRIVATE_DATA_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_PRIVATE_DATA_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_PRIVATE_DATA_EXTENSION_NAME);
        return;
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

    m_errorMonitor->ExpectSuccess(kErrorBit);
    vk::CreateBufferView(m_device->device(), &bvci, NULL, &buffer_view);
    m_errorMonitor->VerifyNotFound();

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
    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();

    // Limit should prevent the message from coming through a fourth time
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, MessageIdFilterString) {
    TEST_DESCRIPTION("Validate that message id string filtering is working");

    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.
    auto filter_setting = MessageIdFilter("VUID-VkRenderPassCreateInfo-pNext-01963");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, filter_setting.pnext));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        return;
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

    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.
    auto filter_setting = MessageIdFilter("0xa19880e3");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, filter_setting.pnext));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        return;
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

    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.
    auto filter_setting = MessageIdFilter("2711126243");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, filter_setting.pnext));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        return;
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
    m_errorMonitor->ExpectSuccess();
    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &local_instance));
    m_errorMonitor->VerifyNotFound();
    vk::DestroyInstance(local_instance, nullptr);

#ifndef NDEBUG
    // Create an instance, error if layer DEBUG_BUILD warning message not found
    callback_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    m_errorMonitor->ExpectSuccess();
    ASSERT_VK_SUCCESS(vk::CreateInstance(&ici, nullptr, &local_instance));
    m_errorMonitor->VerifyNotFound();
    vk::DestroyInstance(local_instance, nullptr);
#endif
}

TEST_F(VkLayerTest, RequiredParameter) {
    TEST_DESCRIPTION("Specify VK_NULL_HANDLE, NULL, and 0 for required handle, pointer, array, and array count parameters");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter pFeatures specified as NULL");
    // Specify NULL for a pointer to a handle
    // Expected to trigger an error with
    // parameter_validation::validate_required_pointer
    vk::GetPhysicalDeviceFeatures(gpu(), NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter pQueueFamilyPropertyCount specified as NULL");
    // Specify NULL for pointer to array count
    // Expected to trigger an error with parameter_validation::validate_array
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), NULL, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewport-viewportCount-arraylength");
    // Specify 0 for a required array count
    // Expected to trigger an error with parameter_validation::validate_array
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
    // Expected to trigger an error with parameter_validation::validate_array
    m_commandBuffer->SetViewport(0, 1, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter memory specified as VK_NULL_HANDLE");
    // Specify VK_NULL_HANDLE for a required handle
    // Expected to trigger an error with
    // parameter_validation::validate_required_handle
    vk::UnmapMemory(device(), VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter pFences[0] specified as VK_NULL_HANDLE");
    // Specify VK_NULL_HANDLE for a required handle array entry
    // Expected to trigger an error with
    // parameter_validation::validate_required_handle_array
    VkFence fence = VK_NULL_HANDLE;
    vk::ResetFences(device(), 1, &fence);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "required parameter pAllocateInfo specified as NULL");
    // Specify NULL for a required struct pointer
    // Expected to trigger an error with
    // parameter_validation::validate_struct_type
    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), NULL, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "value of faceMask must not be 0");
    // Specify 0 for a required VkFlags parameter
    // Expected to trigger an error with parameter_validation::validate_flags
    m_commandBuffer->SetStencilReference(0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "value of pSubmits[0].pWaitDstStageMask[0] must not be 0");
    // Specify 0 for a required VkFlags array entry
    // Expected to trigger an error with
    // parameter_validation::validate_flags_array
    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkPipelineStageFlags stageFlags = 0;
    VkSubmitInfo submitInfo = LvlInitStruct<VkSubmitInfo>();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.pWaitDstStageMask = &stageFlags;
    vk::QueueSubmit(m_device->m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-sType-sType");
    stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    // Set a bogus sType and see what happens
    submitInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.pWaitDstStageMask = &stageFlags;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

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
    bool ycbcr_support = (DeviceExtensionEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) ||
                          (DeviceValidationVersion() >= VK_API_VERSION_1_1));
    bool maintenance2_support =
        (DeviceExtensionEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));

    if (!((m_device->format_properties(VK_FORMAT_R8_UINT).optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) &&
          (ycbcr_support ^ maintenance2_support))) {
        printf("%s Device does not support format and extensions required, skipping test case\n", kSkipPrefix);
        return;
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
        printf("%s Device does not support tessellation shaders; skipped.\n", kSkipPrefix);
        return;
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pNext-pNext");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineTessellationStateCreateInfo-pNext-pNext");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PnextOnlyStructValidation) {
    TEST_DESCRIPTION("See if checks occur on structs ONLY used in pnext chains.");

    if (!(CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names, m_device_extension_names, NULL,
                                                         m_errorMonitor))) {
        printf("Descriptor indexing or one of its dependencies not supported, skipping tests\n");
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device passing in a bad PdevFeatures2 value
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
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
    // parameter_validation::validate_reserved_flags
    VkSemaphore sem_handle = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo sem_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    sem_info.flags = 1;
    vk::CreateSemaphore(device(), &sem_info, NULL, &sem_handle);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DebugMarkerNameTest) {
    TEST_DESCRIPTION("Ensure debug marker object names are printed in debug report output");

    if (InstanceExtensionSupported(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), kValidationLayerName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    } else {
        printf("%s Debug Marker Extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkDebugMarkerSetObjectNameEXT fpvkDebugMarkerSetObjectNameEXT =
        (PFN_vkDebugMarkerSetObjectNameEXT)vk::GetInstanceProcAddr(instance(), "vkDebugMarkerSetObjectNameEXT");
    if (!(fpvkDebugMarkerSetObjectNameEXT)) {
        printf("%s Can't find fpvkDebugMarkerSetObjectNameEXT; skipped.\n", kSkipPrefix);
        return;
    }

    if (DeviceSimulation()) {
        printf("%sSkipping object naming test.\n", kSkipPrefix);
        return;
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

    // Skip test if extension not supported
    if (InstanceExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    } else {
        printf("%s Debug Utils Extension not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkSetDebugUtilsObjectNameEXT fpvkSetDebugUtilsObjectNameEXT =
        (PFN_vkSetDebugUtilsObjectNameEXT)vk::GetInstanceProcAddr(instance(), "vkSetDebugUtilsObjectNameEXT");
    ASSERT_TRUE(fpvkSetDebugUtilsObjectNameEXT);  // Must be extant if extension is enabled
    PFN_vkCreateDebugUtilsMessengerEXT fpvkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vk::GetInstanceProcAddr(instance(), "vkCreateDebugUtilsMessengerEXT");
    ASSERT_TRUE(fpvkCreateDebugUtilsMessengerEXT);  // Must be extant if extension is enabled
    PFN_vkDestroyDebugUtilsMessengerEXT fpvkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vk::GetInstanceProcAddr(instance(), "vkDestroyDebugUtilsMessengerEXT");
    ASSERT_TRUE(fpvkDestroyDebugUtilsMessengerEXT);  // Must be extant if extension is enabled
    PFN_vkCmdInsertDebugUtilsLabelEXT fpvkCmdInsertDebugUtilsLabelEXT =
        (PFN_vkCmdInsertDebugUtilsLabelEXT)vk::GetInstanceProcAddr(instance(), "vkCmdInsertDebugUtilsLabelEXT");
    ASSERT_TRUE(fpvkCmdInsertDebugUtilsLabelEXT);  // Must be extant if extension is enabled

    if (DeviceSimulation()) {
        printf("%sSkipping object naming test.\n", kSkipPrefix);
        return;
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
    fpvkCreateDebugUtilsMessengerEXT(instance(), &callback_create_info, nullptr, &my_messenger);

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
    fpvkSetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    // Pass in 'unknown' object type and see if parameter validation catches it
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDebugUtilsObjectNameInfoEXT-objectType-02589");
    name_info.objectHandle = (uint64_t)memory_2;
    name_info.objectType = VK_OBJECT_TYPE_UNKNOWN;
    fpvkSetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    name_info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
    fpvkSetDebugUtilsObjectNameEXT(device(), &name_info);

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
    fpvkSetDebugUtilsObjectNameEXT(device(), &name_info);

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

    fpvkCmdInsertDebugUtilsLabelEXT(commandBuffer, &command_label);
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
    fpvkDestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(VkLayerTest, InvalidStructSType) {
    TEST_DESCRIPTION("Specify an invalid VkStructureType for a Vulkan structure's sType field");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "parameter pAllocateInfo->sType must be");
    // Zero struct memory, effectively setting sType to
    // VK_STRUCTURE_TYPE_APPLICATION_INFO
    // Expected to trigger an error with
    // parameter_validation::validate_struct_type
    VkMemoryAllocateInfo alloc_info = {};
    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &alloc_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "parameter pSubmits[0].sType must be");
    // Zero struct memory, effectively setting sType to
    // VK_STRUCTURE_TYPE_APPLICATION_INFO
    // Expected to trigger an error with
    // parameter_validation::validate_struct_type_array
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

    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "value of pCreateInfo->pNext must be NULL");
    // Set VkCommandPoolCreateInfo::pNext to a non-NULL value, when pNext must be NULL.
    // Need to pick a function that has no allowed pNext structure types.
    // Expected to trigger an error with parameter_validation::validate_struct_pnext
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
    // Expected to trigger an error with parameter_validation::validate_struct_pnext
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
    // parameter_validation::validate_ranged_enum
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), static_cast<VkFormat>(8000), &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadMask) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "contains flag bits that are not recognized members of");
    // Specify an invalid VkFlags bitmask value
    // Expected to trigger an error with parameter_validation::validate_flags
    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               static_cast<VkImageUsageFlags>(1 << 25), 0, &image_format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadFlag) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "contains flag bits that are not recognized members of");
    // Specify an invalid VkFlags array entry
    // Expected to trigger an error with parameter_validation::validate_flags_array
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    } else {
        printf("%s VK_KHR_sampler_mirror_clamp_to_edge extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Specify an invalid VkBool32 value, expecting a warning with parameter_validation::validate_bool32
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
        printf("%s Device only has one queue family; skipped.\n", kSkipPrefix);
        return;
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
#ifdef _WIN32
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    // Check for external semaphore instance extensions
    if (InstanceExtensionSupported(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s External semaphore extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Check for external semaphore device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, extension_name)) {
        m_device_extension_names.push_back(extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    } else {
        printf("%s External semaphore extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfoKHR esi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR, nullptr,
                                                    handle_type};
    VkExternalSemaphorePropertiesKHR esp = {VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR, nullptr};
    auto vkGetPhysicalDeviceExternalSemaphorePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        printf("%s External semaphore does not support importing and exporting, skipping test\n", kSkipPrefix);
        return;
    }

    VkResult err;

    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfoKHR esci = {VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR, nullptr, handle_type};
    VkSemaphoreCreateInfo sci = LvlInitStruct<VkSemaphoreCreateInfo>(&esci);
    sci.flags = 0;

    VkSemaphore export_semaphore;
    err = vk::CreateSemaphore(m_device->device(), &sci, nullptr, &export_semaphore);
    ASSERT_VK_SUCCESS(err);

    // Create a semaphore to import payload into
    sci.pNext = nullptr;
    VkSemaphore import_semaphore;
    err = vk::CreateSemaphore(m_device->device(), &sci, nullptr, &import_semaphore);
    ASSERT_VK_SUCCESS(err);

#ifdef _WIN32
    // Export semaphore payload to an opaque handle
    HANDLE handle = nullptr;
    VkSemaphoreGetWin32HandleInfoKHR ghi = {VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR, nullptr, export_semaphore,
                                            handle_type};
    auto vkGetSemaphoreWin32HandleKHR =
        (PFN_vkGetSemaphoreWin32HandleKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetSemaphoreWin32HandleKHR");
    err = vkGetSemaphoreWin32HandleKHR(m_device->device(), &ghi, &handle);
    ASSERT_VK_SUCCESS(err);

    // Import opaque handle exported above *temporarily*
    VkImportSemaphoreWin32HandleInfoKHR ihi = {VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR,
                                               nullptr,
                                               import_semaphore,
                                               VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR,
                                               handle_type,
                                               handle,
                                               nullptr};
    auto vkImportSemaphoreWin32HandleKHR =
        (PFN_vkImportSemaphoreWin32HandleKHR)vk::GetDeviceProcAddr(m_device->device(), "vkImportSemaphoreWin32HandleKHR");
    err = vkImportSemaphoreWin32HandleKHR(m_device->device(), &ihi);
    ASSERT_VK_SUCCESS(err);
#else
    // Export semaphore payload to an opaque handle
    int fd = 0;
    VkSemaphoreGetFdInfoKHR ghi = {VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR, nullptr, export_semaphore, handle_type};
    auto vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetSemaphoreFdKHR");
    err = vkGetSemaphoreFdKHR(m_device->device(), &ghi, &fd);
    ASSERT_VK_SUCCESS(err);

    // Import opaque handle exported above *temporarily*
    VkImportSemaphoreFdInfoKHR ihi = {VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR, nullptr,     import_semaphore,
                                      VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR,          handle_type, fd};
    auto vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)vk::GetDeviceProcAddr(m_device->device(), "vkImportSemaphoreFdKHR");
    err = vkImportSemaphoreFdKHR(m_device->device(), &ihi);
    ASSERT_VK_SUCCESS(err);
#endif

    // Wait on the imported semaphore twice in vk::QueueSubmit, the second wait should be an error
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo si[] = {
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, &flags, 0, nullptr, 1, &export_semaphore},
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &import_semaphore, &flags, 0, nullptr, 0, nullptr},
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, &flags, 0, nullptr, 1, &export_semaphore},
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &import_semaphore, &flags, 0, nullptr, 0, nullptr},
    };
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "has no way to be signaled");
    vk::QueueSubmit(m_device->m_queue, 4, si, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    auto index = m_device->graphics_queue_node_index_;
    if (m_device->queue_props[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
        // Wait on the imported semaphore twice in vk::QueueBindSparse, the second wait should be an error
        VkBindSparseInfo bi[] = {
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 1, &export_semaphore},
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 1, &import_semaphore, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr},
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 1, &export_semaphore},
            {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 1, &import_semaphore, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr},
        };
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "has no way to be signaled");
        vk::QueueBindSparse(m_device->m_queue, 4, bi, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Cleanup
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    vk::DestroySemaphore(m_device->device(), export_semaphore, nullptr);
    vk::DestroySemaphore(m_device->device(), import_semaphore, nullptr);
}

TEST_F(VkLayerTest, TemporaryExternalFence) {
#ifdef _WIN32
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    // Check for external fence instance extensions
    if (InstanceExtensionSupported(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s External fence extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Check for external fence device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, extension_name)) {
        m_device_extension_names.push_back(extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    } else {
        printf("%s External fence extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfoKHR efi = LvlInitStruct<VkPhysicalDeviceExternalFenceInfoKHR>();
    efi.handleType = handle_type;
    VkExternalFencePropertiesKHR efp = LvlInitStruct<VkExternalFencePropertiesKHR>();
    auto vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vk::GetInstanceProcAddr(
        instance(), "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    vkGetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        printf("%s External fence does not support importing and exporting, skipping test\n", kSkipPrefix);
        return;
    }

    VkResult err;

    // Create a fence to export payload from
    VkFence export_fence;
    {
        VkExportFenceCreateInfoKHR efci = {VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO_KHR, nullptr, handle_type};
        VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, &efci, 0};
        err = vk::CreateFence(m_device->device(), &fci, nullptr, &export_fence);
        ASSERT_VK_SUCCESS(err);
    }

    // Create a fence to import payload into
    VkFence import_fence;
    {
        VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
        err = vk::CreateFence(m_device->device(), &fci, nullptr, &import_fence);
        ASSERT_VK_SUCCESS(err);
    }

#ifdef _WIN32
    // Export fence payload to an opaque handle
    HANDLE handle = nullptr;
    {
        VkFenceGetWin32HandleInfoKHR ghi = {VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR, nullptr, export_fence, handle_type};
        auto vkGetFenceWin32HandleKHR =
            (PFN_vkGetFenceWin32HandleKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetFenceWin32HandleKHR");
        err = vkGetFenceWin32HandleKHR(m_device->device(), &ghi, &handle);
        ASSERT_VK_SUCCESS(err);
    }

    // Import opaque handle exported above
    {
        VkImportFenceWin32HandleInfoKHR ifi = {VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR,
                                               nullptr,
                                               import_fence,
                                               VK_FENCE_IMPORT_TEMPORARY_BIT_KHR,
                                               handle_type,
                                               handle,
                                               nullptr};
        auto vkImportFenceWin32HandleKHR =
            (PFN_vkImportFenceWin32HandleKHR)vk::GetDeviceProcAddr(m_device->device(), "vkImportFenceWin32HandleKHR");
        err = vkImportFenceWin32HandleKHR(m_device->device(), &ifi);
        ASSERT_VK_SUCCESS(err);
    }
#else
    // Export fence payload to an opaque handle
    int fd = 0;
    {
        VkFenceGetFdInfoKHR gfi = {VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR, nullptr, export_fence, handle_type};
        auto vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)vk::GetDeviceProcAddr(m_device->device(), "vkGetFenceFdKHR");
        err = vkGetFenceFdKHR(m_device->device(), &gfi, &fd);
        ASSERT_VK_SUCCESS(err);
    }

    // Import opaque handle exported above
    {
        VkImportFenceFdInfoKHR ifi = {VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR, nullptr,     import_fence,
                                      VK_FENCE_IMPORT_TEMPORARY_BIT_KHR,          handle_type, fd};
        auto vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR)vk::GetDeviceProcAddr(m_device->device(), "vkImportFenceFdKHR");
        err = vkImportFenceFdKHR(m_device->device(), &ifi);
        ASSERT_VK_SUCCESS(err);
    }
#endif

    // Undo the temporary import
    vk::ResetFences(m_device->device(), 1, &import_fence);

    // Signal the previously imported fence twice, the second signal should produce a validation error
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, import_fence);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "is already in use by another submission.");
    vk::QueueSubmit(m_device->m_queue, 0, nullptr, import_fence);
    m_errorMonitor->VerifyFound();

    // Cleanup
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    vk::DestroyFence(m_device->device(), export_fence, nullptr);
    vk::DestroyFence(m_device->device(), import_fence, nullptr);
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::vector<const char *> device_extensions;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME) &&
        DeviceExtensionSupported(gpu(), nullptr, VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME)) {
        device_extensions.push_back(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    } else {
        printf("%s VariablePointer Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables variablePointers but not variablePointersStorageBuffer
    auto variable_features = LvlInitStruct<VkPhysicalDeviceVariablePointersFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&variable_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (variable_features.variablePointers == VK_FALSE) {
        printf("%s variablePointer feature not supported, skipping tests\n", kSkipPrefix);
        return;
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::vector<const char *> device_extensions;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        device_extensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    } else {
        printf("%s Multiview Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&multiview_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

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
        printf("%s multiviewGeometryShader and  multiviewTessellationShader feature not supported, skipping tests\n", kSkipPrefix);
        return;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), kValidationLayerName, VK_EXT_VALIDATION_CACHE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
    } else {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Load extension functions
    auto fpCreateValidationCache =
        (PFN_vkCreateValidationCacheEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCreateValidationCacheEXT");
    auto fpDestroyValidationCache =
        (PFN_vkDestroyValidationCacheEXT)vk::GetDeviceProcAddr(m_device->device(), "vkDestroyValidationCacheEXT");
    auto fpMergeValidationCaches =
        (PFN_vkMergeValidationCachesEXT)vk::GetDeviceProcAddr(m_device->device(), "vkMergeValidationCachesEXT");
    if (!fpCreateValidationCache || !fpDestroyValidationCache || !fpMergeValidationCaches) {
        printf("%s Failed to load function pointers for %s\n", kSkipPrefix, VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
        return;
    }

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
        printf("%s Multiple queue families are required to run this test.\n", kSkipPrefix);
        return;
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
    m_errorMonitor->ExpectSuccess();
    vk::CreateBuffer(second_device, &buffCI, NULL, &buffer);
    m_errorMonitor->VerifyNotFound();
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
        printf("%s This test should not run on Pixel 2 XL\n", kSkipPrefix);
        return;
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
    m_errorMonitor->ExpectSuccess();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), occlusion_query_pool, 0, query_pool_size);
    m_errorMonitor->VerifyNotFound();

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
        printf("%s Test requires unsupported pipelineStatisticsQuery feature. Skipped.\n", kSkipPrefix);
        return;
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

    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-01149");
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent-stageMask-01153");
    vk::CmdResetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_HOST_BIT;
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();

    // Signal the semaphore so the next test can wait on it.
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore;
    submit_info.pWaitDstStageMask = &stage_flags;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitDstStageMask-00078");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DestroyEvent(m_device->device(), event, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
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

    m_errorMonitor->ExpectSuccess();

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

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_test.GetBuffer(), 0, 1024, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
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
    m_errorMonitor->Reset();  // resume logmsg processing

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
    m_errorMonitor->ExpectSuccess();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();
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
    m_errorMonitor->ExpectSuccess();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();
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
        printf("%s Device graphic queue has timestampValidBits of 0, skipping.\n", kSkipPrefix);
        return;
    }

    VkBufferObj buffer;
    buffer.init(*m_device, 128, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    // Use setup as a positive test...
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-00827");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 0, 8, VK_QUERY_RESULT_PARTIAL_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->end();

    // Submit cmd buffer and wait for it.
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->VerifyNotFound();

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

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME);
    } else {
        printf("%s Intel Performance Query Extension not supported, skipping tests\n", kSkipPrefix);
        return;
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
        printf("%s Unable to compile shader, skipping.\n", kSkipPrefix);
        return;
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

    // Test takes magnitude of time longer for devsim and slows down testing
    if (IsPlatform(kMockICD) || DeviceSimulation()) {
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
    bool bailout = false;
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
    bool bailout = false;
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
    m_errorMonitor->ExpectSuccess();

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables descriptorBindingStorageBufferUpdateAfterBind
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (VK_FALSE == indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        printf("%s Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind, skipping\n", kSkipPrefix);
        return;
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
    bool bailout = false;
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

    m_errorMonitor->VerifyNotFound();
}
#endif  // GTEST_IS_THREADSAFE

TEST_F(VkLayerTest, ExecuteUnrecordedCB) {
    TEST_DESCRIPTION("Attempt vkQueueSubmit with a CB in the initial state");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());
    // never record m_commandBuffer
    m_errorMonitor->VerifyNotFound();

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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!((DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) &&
          (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME)))) {
        printf("%s Maintenance1 and AMD_negative viewport height extensions not supported, skipping test\n", kSkipPrefix);
        return;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!(DeviceExtensionSupported(gpu(), nullptr, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME))) {
        printf("%s AMD_negative viewport height extensions not supported, skipping test\n", kSkipPrefix);
        return;
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
            m_errorMonitor->ExpectSuccess();
            m_errorMonitor->SetError("No ProcAddr for 1.2 core vkResetQueryPool");
            m_errorMonitor->VerifyNotFound();
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

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables shading_rate_image but disables multiViewport
    auto shading_rate_image_features = LvlInitStruct<VkPhysicalDeviceShadingRateImageFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&shading_rate_image_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

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
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-tiling-02084");

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
    m_errorMonitor->VerifyNotFound();

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
    m_errorMonitor->VerifyNotFound();

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
                m_errorMonitor->VerifyNotFound();
            }
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetCoarseSampleOrderNV-sampleOrderType-02081");
        vkCmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV, 1, &sampOrdGood);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();

    vk::DestroyImageView(m_device->device(), view, NULL);
}

#include "android_ndk_types.h"
#ifdef AHB_VALIDATION_SUPPORT

TEST_F(VkLayerTest, AndroidHardwareBufferImageCreate) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer image create info.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkImage img = VK_NULL_HANDLE;
    auto reset_img = [&img, dev]() {
        if (VK_NULL_HANDLE != img) vk::DestroyImage(dev, img, NULL);
        img = VK_NULL_HANDLE;
    };

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    // undefined format
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-01975");
    // Various extra errors for having VK_FORMAT_UNDEFINED without VkExternalFormatANDROID
    m_errorMonitor->SetUnexpectedError("VUID_Undefined");
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();

    // also undefined format
    VkExternalFormatANDROID efa = LvlInitStruct<VkExternalFormatANDROID>();
    efa.externalFormat = 0;
    ici.pNext = &efa;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-01975");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();

    // undefined format with an unknown external format
    efa.externalFormat = 0xBADC0DE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkExternalFormatANDROID-externalFormat-01894");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();

    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    // Allocate an AHardwareBuffer
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Retrieve it's properties to make it's external format 'known' (AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM)
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);
    pfn_GetAHBProps(dev, ahb, &ahb_props);

    // a defined image format with a non-zero external format
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    efa.externalFormat = ahb_fmt_props.externalFormat;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-01974");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
    ici.format = VK_FORMAT_UNDEFINED;

    // external format while MUTABLE
    ici.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02396");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
    ici.flags = 0;

    // external format while usage other than SAMPLED
    ici.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02397");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    // external format while tiline other than OPTIMAL
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02398");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;

    // imageType
    VkExternalMemoryImageCreateInfo emici = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    ici.pNext = &emici;  // remove efa from chain, insert emici
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.imageType = VK_IMAGE_TYPE_3D;
    ici.extent = {64, 64, 64};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02393");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();

    // wrong mipLevels
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.extent = {64, 64, 1};
    ici.mipLevels = 6;  // should be 7
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-02394");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
}

TEST_F(VkLayerTest, AndroidHardwareBufferFetchUnboundImageInfo) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer retreive image properties while memory unbound.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkImage img = VK_NULL_HANDLE;
    auto reset_img = [&img, dev]() {
        if (VK_NULL_HANDLE != img) vk::DestroyImage(dev, img, NULL);
        img = VK_NULL_HANDLE;
    };

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkExternalMemoryImageCreateInfo emici = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    ici.pNext = &emici;

    m_errorMonitor->ExpectSuccess();
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyNotFound();

    // attempt to fetch layout from unbound image
    VkImageSubresource sub_rsrc = {};
    sub_rsrc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSubresourceLayout sub_layout = {};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-image-01895");
    vk::GetImageSubresourceLayout(dev, img, &sub_rsrc, &sub_layout);
    m_errorMonitor->VerifyFound();

    // attempt to get memory reqs from unbound image
    VkImageMemoryRequirementsInfo2 imri = LvlInitStruct<VkImageMemoryRequirementsInfo2>();
    imri.image = img;
    VkMemoryRequirements2 mem_reqs = LvlInitStruct<VkMemoryRequirements2>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-01897");
    vk::GetImageMemoryRequirements2(dev, &imri, &mem_reqs);
    m_errorMonitor->VerifyFound();

    reset_img();
}

TEST_F(VkLayerTest, AndroidHardwareBufferMemoryAllocation) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer memory allocation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkImage img = VK_NULL_HANDLE;
    auto reset_img = [&img, dev]() {
        if (VK_NULL_HANDLE != img) vk::DestroyImage(dev, img, NULL);
        img = VK_NULL_HANDLE;
    };
    VkDeviceMemory mem_handle = VK_NULL_HANDLE;
    auto reset_mem = [&mem_handle, dev]() {
        if (VK_NULL_HANDLE != mem_handle) vk::FreeMemory(dev, mem_handle, NULL);
        mem_handle = VK_NULL_HANDLE;
    };

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    // AHB structs
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    VkImportAndroidHardwareBufferInfoANDROID iahbi = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();

    // destroy and re-acquire an AHB, and fetch it's properties
    auto recreate_ahb = [&ahb, &iahbi, &ahb_desc, &ahb_props, dev, pfn_GetAHBProps]() {
        if (ahb) AHardwareBuffer_release(ahb);
        ahb = nullptr;
        AHardwareBuffer_allocate(&ahb_desc, &ahb);
        if (ahb) {
            pfn_GetAHBProps(dev, ahb, &ahb_props);
            iahbi.buffer = ahb;
        }
    };

    // Allocate an AHardwareBuffer
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    recreate_ahb();

    // Create an image w/ external format
    VkExternalFormatANDROID efa = LvlInitStruct<VkExternalFormatANDROID>();
    efa.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&efa);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkResult res = vk::CreateImage(dev, &ici, NULL, &img);
    ASSERT_VK_SUCCESS(res);

    // Chained import struct
    VkMemoryAllocateInfo mai = LvlInitStruct<VkMemoryAllocateInfo>(&iahbi);
    mai.allocationSize = ahb_props.allocationSize;
    mai.memoryTypeIndex = 32;
    // Set index to match one of the bits in ahb_props
    for (int i = 0; i < 32; i++) {
        if (ahb_props.memoryTypeBits & (1 << i)) {
            mai.memoryTypeIndex = i;
            break;
        }
    }
    ASSERT_NE(32, mai.memoryTypeIndex);

    // Import w/ non-dedicated memory allocation

    // Import requires format AHB_FMT_BLOB and usage AHB_USAGE_GPU_DATA_BUFFER
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02384");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    // Allocation size mismatch
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    ahb_desc.height = 1;
    recreate_ahb();
    mai.allocationSize = ahb_props.allocationSize + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-allocationSize-02383");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    mai.allocationSize = ahb_props.allocationSize;
    reset_mem();

    // memoryTypeIndex mismatch
    mai.memoryTypeIndex++;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    mai.memoryTypeIndex--;
    reset_mem();

    // Insert dedicated image memory allocation to mai chain
    VkMemoryDedicatedAllocateInfo mdai = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    mdai.image = img;
    mdai.buffer = VK_NULL_HANDLE;
    mdai.pNext = mai.pNext;
    mai.pNext = &mdai;

    // Dedicated allocation with unmatched usage bits for Color
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;
    ahb_desc.height = 64;
    recreate_ahb();
    mai.allocationSize = ahb_props.allocationSize;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02390");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    // Dedicated allocation with unmatched usage bits for Depth/Stencil
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_S8_UINT;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;
    ahb_desc.height = 64;
    recreate_ahb();
    mai.allocationSize = ahb_props.allocationSize;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02390");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    // Dedicated allocation with incomplete mip chain
    reset_img();
    ici.mipLevels = 2;
    vk::CreateImage(dev, &ici, NULL, &img);
    mdai.image = img;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE;
    recreate_ahb();

    if (ahb) {
        mai.allocationSize = ahb_props.allocationSize;
        for (int i = 0; i < 32; i++) {
            if (ahb_props.memoryTypeBits & (1 << i)) {
                mai.memoryTypeIndex = i;
                break;
            }
        }
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02389");
        vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
        m_errorMonitor->VerifyFound();
        reset_mem();
    } else {
        // ERROR: AHardwareBuffer_allocate() with MIPMAP_COMPLETE fails. It returns -12, NO_MEMORY.
        // The problem seems to happen in Pixel 2, not Pixel 3.
        printf("%s AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    // Dedicated allocation with mis-matched dimension
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.height = 32;
    ahb_desc.width = 128;
    recreate_ahb();
    mai.allocationSize = ahb_props.allocationSize;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02388");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    // Dedicated allocation with mis-matched VkFormat
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.height = 64;
    ahb_desc.width = 64;
    recreate_ahb();
    mai.allocationSize = ahb_props.allocationSize;
    ici.mipLevels = 1;
    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    ici.pNext = NULL;
    VkImage img2;
    vk::CreateImage(dev, &ici, NULL, &img2);
    mdai.image = img2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02387");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    vk::DestroyImage(dev, img2, NULL);
    mdai.image = img;
    reset_mem();

    // Missing required ahb usage
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");
    recreate_ahb();
    m_errorMonitor->VerifyFound();

    // Dedicated allocation with missing usage bits
    // Setting up this test also triggers a slew of others
    mai.allocationSize = ahb_props.allocationSize + 1;
    mai.memoryTypeIndex = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02390");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-allocationSize-02383");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02386");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    // Non-import allocation - replace import struct in chain with export struct
    VkExportMemoryAllocateInfo emai = LvlInitStruct<VkExportMemoryAllocateInfo>();
    emai.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    mai.pNext = &emai;
    emai.pNext = &mdai;  // still dedicated
    mdai.pNext = nullptr;

    // Export with allocation size non-zero
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    recreate_ahb();
    mai.allocationSize = ahb_props.allocationSize;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryDedicatedAllocateInfo-image-02964");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-01874");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    AHardwareBuffer_release(ahb);
    reset_mem();
    reset_img();
}

TEST_F(VkLayerTest, AndroidHardwareBufferCreateYCbCrSampler) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer YCbCr sampler creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Enable Ycbcr Conversion Features
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = LvlInitStruct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
    ycbcr_features.samplerYcbcrConversion = VK_TRUE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ycbcr_features));
    VkDevice dev = m_device->device();

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-format-04061");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    VkExternalFormatANDROID efa = LvlInitStruct<VkExternalFormatANDROID>();
    efa.externalFormat = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    sycci.format = VK_FORMAT_R8G8B8A8_UNORM;
    sycci.pNext = &efa;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01904");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    efa.externalFormat = AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420;
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    // Spec says if we use VkExternalFormatANDROID value of components is ignored.
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    vk::CreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyNotFound();
    vk::DestroySamplerYcbcrConversion(dev, ycbcr_conv, nullptr);
}

TEST_F(VkLayerTest, AndroidHardwareBufferPhysDevImageFormatProp2) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer GetPhysicalDeviceImageFormatProperties.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    if ((DeviceValidationVersion() < VK_API_VERSION_1_1) &&
        !InstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    VkImageFormatProperties2 ifp = LvlInitStruct<VkImageFormatProperties2>();
    VkPhysicalDeviceImageFormatInfo2 pdifi = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>();
    pdifi.format = VK_FORMAT_R8G8B8A8_UNORM;
    pdifi.tiling = VK_IMAGE_TILING_OPTIMAL;
    pdifi.type = VK_IMAGE_TYPE_2D;
    pdifi.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkAndroidHardwareBufferUsageANDROID ahbu = LvlInitStruct<VkAndroidHardwareBufferUsageANDROID>();
    ahbu.androidHardwareBufferUsage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ifp.pNext = &ahbu;

    // AHB_usage chained to input without a matching external image format struc chained to output
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868");
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &pdifi, &ifp);
    m_errorMonitor->VerifyFound();

    // output struct chained, but does not include VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID usage
    VkPhysicalDeviceExternalImageFormatInfo pdeifi = LvlInitStruct<VkPhysicalDeviceExternalImageFormatInfo>();
    pdeifi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    pdifi.pNext = &pdeifi;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868");
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &pdifi, &ifp);
    m_errorMonitor->VerifyFound();
}

#if DISABLEUNTILAHBWORKS
TEST_F(VkLayerTest, AndroidHardwareBufferCreateImageView) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer image view creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    // Allocate an AHB and fetch its properties
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Retrieve AHB properties to make it's external format 'known'
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props);
    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);
    pfn_GetAHBProps(dev, ahb, &ahb_props);
    AHardwareBuffer_release(ahb);

    VkExternalMemoryImageCreateInfo emici = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    // Give image an external format
    VkExternalFormatANDROID efa = LvlInitStruct<VkExternalFormatANDROID>(&emici);
    efa.externalFormat = ahb_fmt_props.externalFormat;

    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Create another VkExternalFormatANDROID for test VUID-VkImageViewCreateInfo-image-02400
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props_Ycbcr =
        LvlInitStruct<VkAndroidHardwareBufferFormatPropertiesANDROID>();
    VkAndroidHardwareBufferPropertiesANDROID ahb_props_Ycbcr =
        LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>(&ahb_fmt_props_Ycbcr);
    pfn_GetAHBProps(dev, ahb, &ahb_props_Ycbcr);
    AHardwareBuffer_release(ahb);

    VkExternalFormatANDROID efa_Ycbcr = LvlInitStruct<VkExternalFormatANDROID>();
    efa_Ycbcr.externalFormat = ahb_fmt_props_Ycbcr.externalFormat;

    // Need to make sure format has sample bit needed for image usage
    if ((ahb_fmt_props_Ycbcr.formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0) {
        printf("%s VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT feature bit not supported for format %" PRIu64 ".\n", kSkipPrefix,
               ahb_fmt_props_Ycbcr.externalFormat);
        return;
    }

    // Create the image
    VkImage img = VK_NULL_HANDLE;
    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>(&efa);
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vk::CreateImage(dev, &ici, NULL, &img);

    // Set up memory allocation
    VkDeviceMemory img_mem = VK_NULL_HANDLE;
    VkMemoryAllocateInfo mai = LvlInitStruct<VkMemoryAllocateInfo>();
    mai.allocationSize = 64 * 64 * 4;
    mai.memoryTypeIndex = 0;
    vk::AllocateMemory(dev, &mai, NULL, &img_mem);

    // It shouldn't use vk::GetImageMemoryRequirements for imported AndroidHardwareBuffer when memory isn't bound yet
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageMemoryRequirements-image-04004");
    VkMemoryRequirements img_mem_reqs = {};
    vk::GetImageMemoryRequirements(m_device->device(), img, &img_mem_reqs);
    m_errorMonitor->VerifyFound();
    vk::BindImageMemory(dev, img, img_mem, 0);

    // Bind image to memory
    vk::DestroyImage(dev, img, NULL);
    vk::FreeMemory(dev, img_mem, NULL);
    vk::CreateImage(dev, &ici, NULL, &img);
    vk::AllocateMemory(dev, &mai, NULL, &img_mem);
    vk::BindImageMemory(dev, img, img_mem, 0);

    // Create a YCbCr conversion, with different external format, chain to view
    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>(&efa_Ycbcr);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    vk::CreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    VkSamplerYcbcrConversionInfo syci = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
    syci.conversion = ycbcr_conv;

    // Create a view
    VkImageView image_view = VK_NULL_HANDLE;
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>(&syci);
    ivci.image = img;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    auto reset_view = [&image_view, dev]() {
        if (VK_NULL_HANDLE != image_view) vk::DestroyImageView(dev, image_view, NULL);
        image_view = VK_NULL_HANDLE;
    };

    // Up to this point, no errors expected
    m_errorMonitor->VerifyNotFound();

    // Chained ycbcr conversion has different (external) format than image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02400");
    // Also causes "unsupported format" - should be removed in future spec update
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-None-02273");
    vk::CreateImageView(dev, &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    vk::DestroySamplerYcbcrConversion(dev, ycbcr_conv, NULL);
    sycci.pNext = &efa;
    vk::CreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    syci.conversion = ycbcr_conv;

    // View component swizzle not IDENTITY
    ivci.components.r = VK_COMPONENT_SWIZZLE_B;
    ivci.components.b = VK_COMPONENT_SWIZZLE_R;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02401");
    // Also causes "unsupported format" - should be removed in future spec update
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-None-02273");
    vk::CreateImageView(dev, &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;

    // View with external format, when format is not UNDEFINED
    ivci.format = VK_FORMAT_R5G6B5_UNORM_PACK16;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02399");
    // Also causes "view format different from image format"
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-01762");
    vk::CreateImageView(dev, &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    vk::DestroySamplerYcbcrConversion(dev, ycbcr_conv, NULL);
    vk::DestroyImageView(dev, image_view, NULL);
    vk::DestroyImage(dev, img, NULL);
    vk::FreeMemory(dev, img_mem, NULL);
}
#endif  // DISABLEUNTILAHBWORKS

TEST_F(VkLayerTest, AndroidHardwareBufferImportBuffer) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer import as buffer.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkDeviceMemory mem_handle = VK_NULL_HANDLE;
    auto reset_mem = [&mem_handle, dev]() {
        if (VK_NULL_HANDLE != mem_handle) vk::FreeMemory(dev, mem_handle, NULL);
        mem_handle = VK_NULL_HANDLE;
    };

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    // AHB structs
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    VkImportAndroidHardwareBufferInfoANDROID iahbi = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();

    // Allocate an AHardwareBuffer
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_SENSOR_DIRECT_DATA;  // non USAGE_GPU_*
    ahb_desc.width = 512;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);
    m_errorMonitor->SetUnexpectedError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");
    pfn_GetAHBProps(dev, ahb, &ahb_props);
    iahbi.buffer = ahb;

    // Create export and import buffers
    VkExternalMemoryBufferCreateInfo ext_buf_info = LvlInitStruct<VkExternalMemoryBufferCreateInfo>();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;

    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>(&ext_buf_info);
    bci.size = ahb_props.allocationSize;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkBuffer buf = VK_NULL_HANDLE;
    vk::CreateBuffer(dev, &bci, NULL, &buf);
    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(dev, buf, &mem_reqs);

    // Allocation info
    VkMemoryAllocateInfo mai = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, mem_reqs, 0);
    mai.pNext = &iahbi;  // Chained import struct
    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);
    unsigned int i;
    for (i = 0; i < memory_info.memoryTypeCount; i++) {
        if ((ahb_props.memoryTypeBits & (1 << i))) {
            mai.memoryTypeIndex = i;
            break;
        }
    }
    if (i >= memory_info.memoryTypeCount) {
        printf("%s No invalid memory type index could be found; skipped.\n", kSkipPrefix);
        AHardwareBuffer_release(ahb);
        reset_mem();
        vk::DestroyBuffer(dev, buf, NULL);
        return;
    }

    // Import as buffer requires usage AHB_USAGE_GPU_DATA_BUFFER
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01881");
    // Also causes "non-dedicated allocation format/usage" error
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-02384");
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
    reset_mem();
    vk::DestroyBuffer(dev, buf, NULL);
}

TEST_F(VkLayerTest, AndroidHardwareBufferExporttBuffer) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer export memory as AHB.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkDeviceMemory mem_handle = VK_NULL_HANDLE;

    // Allocate device memory, no linked export struct indicating AHB handle type
    VkMemoryAllocateInfo mai = LvlInitStruct<VkMemoryAllocateInfo>();
    mai.allocationSize = 65536;
    mai.memoryTypeIndex = 0;
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);

    PFN_vkGetMemoryAndroidHardwareBufferANDROID pfn_GetMemAHB =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)vk::GetDeviceProcAddr(dev, "vkGetMemoryAndroidHardwareBufferANDROID");
    ASSERT_TRUE(pfn_GetMemAHB != nullptr);

    VkMemoryGetAndroidHardwareBufferInfoANDROID mgahbi = LvlInitStruct<VkMemoryGetAndroidHardwareBufferInfoANDROID>();
    mgahbi.memory = mem_handle;
    AHardwareBuffer *ahb = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-handleTypes-01882");
    pfn_GetMemAHB(dev, &mgahbi, &ahb);
    m_errorMonitor->VerifyFound();

    if (ahb) AHardwareBuffer_release(ahb);
    ahb = nullptr;
    if (VK_NULL_HANDLE != mem_handle) vk::FreeMemory(dev, mem_handle, NULL);
    mem_handle = VK_NULL_HANDLE;

    // Add an export struct with AHB handle type to allocation info
    VkExportMemoryAllocateInfo emai = LvlInitStruct<VkExportMemoryAllocateInfo>();
    emai.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    mai.pNext = &emai;

    // Create an image, do not bind memory
    VkImage img = VK_NULL_HANDLE;
    VkImageCreateInfo ici = LvlInitStruct<VkImageCreateInfo>();
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {128, 128, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vk::CreateImage(dev, &ici, NULL, &img);
    ASSERT_TRUE(VK_NULL_HANDLE != img);

    // Add image to allocation chain as dedicated info, re-allocate
    VkMemoryDedicatedAllocateInfo mdai = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    mdai.image = img;
    emai.pNext = &mdai;
    mai.allocationSize = 0;
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);
    mgahbi.memory = mem_handle;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-pNext-01883");
    pfn_GetMemAHB(dev, &mgahbi, &ahb);
    m_errorMonitor->VerifyFound();

    if (ahb) AHardwareBuffer_release(ahb);
    if (VK_NULL_HANDLE != mem_handle) vk::FreeMemory(dev, mem_handle, NULL);
    vk::DestroyImage(dev, img, NULL);
}

TEST_F(VkLayerTest, AndroidHardwareBufferInvalidBindBufferMemory) {
    TEST_DESCRIPTION("Validate binding AndroidHardwareBuffer VkBuffer act same as non-AHB buffers.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // Allocate an AHardwareBuffer
    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    VkExternalMemoryBufferCreateInfo ext_buf_info = LvlInitStruct<VkExternalMemoryBufferCreateInfo>();
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>(&ext_buf_info);
    buffer_create_info.size = 1 << 20;  // 1 MB
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkBuffer buffer = VK_NULL_HANDLE;
    vk::CreateBuffer(m_device->device(), &buffer_create_info, nullptr, &buffer);

    // Try to get memory requirements prior to binding memory
    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    memory_info.allocationSize = mem_reqs.size + mem_reqs.alignment;  // save room for offset
    bool has_memtype = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &memory_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (!has_memtype) {
        printf("%s No invalid memory type index could be found; skipped.\n", kSkipPrefix);
        AHardwareBuffer_release(ahb);
        vk::DestroyBuffer(m_device->device(), buffer, nullptr);
        return;
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkResult result = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &memory);
    if ((memory == VK_NULL_HANDLE) || (result != VK_SUCCESS)) {
        printf("%s This test failed to allocate memory for importing\n", kSkipPrefix);
        return;
    }

    if (mem_reqs.alignment > 1) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memoryOffset-01036");
        vk::BindBufferMemory(device(), buffer, memory, 1);
        m_errorMonitor->VerifyFound();
    }

    VkDeviceSize buffer_offset = (mem_reqs.size - 1) & ~(mem_reqs.alignment - 1);
    if (buffer_offset > 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-size-01037");
        vk::BindBufferMemory(device(), buffer, memory, buffer_offset);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroyBuffer(m_device->device(), buffer, nullptr);
    vk::FreeMemory(m_device->device(), memory, nullptr);
}

TEST_F(VkLayerTest, AndroidHardwareBufferImportBufferHandleType) {
    TEST_DESCRIPTION("Don't use proper resource handleType for import buffer");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(m_device->device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    PFN_vkBindBufferMemory2KHR vkBindBufferMemory2Function =
        (PFN_vkBindBufferMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindBufferMemory2KHR");

    m_errorMonitor->ExpectSuccess();

    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Create buffer without VkExternalMemoryBufferCreateInfo
    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 512;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vk::CreateBuffer(m_device->device(), &buffer_create_info, nullptr, &buffer);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>();
    import_ahb_Info.buffer = ahb;

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    memory_allocate_info.allocationSize = ahb_props.allocationSize;
    // driver won't expose correct memoryType since resource was not created as an import operation
    // so just need any valid memory type returned from GetAHBInfo
    for (int i = 0; i < 32; i++) {
        if (ahb_props.memoryTypeBits & (1 << i)) {
            memory_allocate_info.memoryTypeIndex = i;
            break;
        }
    }

    VkDeviceMemory memory;
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, nullptr, &memory);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-02986");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindBufferMemory-memory-01035");
    vk::BindBufferMemory(m_device->device(), buffer, memory, 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = LvlInitStruct<VkBindBufferMemoryInfo>();
    bind_buffer_info.buffer = buffer;
    bind_buffer_info.memory = memory;
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryInfo-memory-02986");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindBufferMemoryInfo-memory-01035");
    vkBindBufferMemory2Function(m_device->device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(m_device->device(), buffer, nullptr);
    vk::FreeMemory(m_device->device(), memory, nullptr);
}

TEST_F(VkLayerTest, AndroidHardwareBufferImportImageHandleType) {
    TEST_DESCRIPTION("Don't use proper resource handleType for import image");

    AddRequiredExtensions(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(m_device->device(),
                                                                               "vkGetAndroidHardwareBufferPropertiesANDROID");
    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function =
        (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");

    m_errorMonitor->ExpectSuccess();

    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    ahb_desc.stride = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Create buffer without VkExternalMemoryImageCreateInfo
    VkImage image = VK_NULL_HANDLE;
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vk::CreateImage(m_device->device(), &image_create_info, nullptr, &image);

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    memory_dedicated_info.image = image;
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info =
        LvlInitStruct<VkImportAndroidHardwareBufferInfoANDROID>(&memory_dedicated_info);
    import_ahb_Info.buffer = ahb;

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = LvlInitStruct<VkAndroidHardwareBufferPropertiesANDROID>();
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&import_ahb_Info);
    memory_allocate_info.allocationSize = ahb_props.allocationSize;
    // driver won't expose correct memoryType since resource was not created as an import operation
    // so just need any valid memory type returned from GetAHBInfo
    for (int i = 0; i < 32; i++) {
        if (ahb_props.memoryTypeBits & (1 << i)) {
            memory_allocate_info.memoryTypeIndex = i;
            break;
        }
    }

    VkDeviceMemory memory;
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, nullptr, &memory);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-02990");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-memory-01047");
    m_errorMonitor->SetUnexpectedError("VUID-vkBindImageMemory-size-01049");
    vk::BindImageMemory(m_device->device(), image, memory, 0);
    m_errorMonitor->VerifyFound();

    VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>();
    bind_image_info.image = image;
    bind_image_info.memory = memory;
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-memory-02990");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01617");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01615");
    vkBindImageMemory2Function(m_device->device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(m_device->device(), image, nullptr);
    vk::FreeMemory(m_device->device(), memory, nullptr);
}

#endif  // AHB_VALIDATION_SUPPORT

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
    m_errorMonitor->ExpectSuccess();
    vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space4), &data_space4, 4, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyNotFound();

    char data_space8[8] = "";
    m_errorMonitor->ExpectSuccess();
    vk::GetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space8), &data_space8, 8,
                            (VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT));
    m_errorMonitor->VerifyNotFound();

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

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 4, 4, 0);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 8, 8, VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyNotFound();

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

        m_errorMonitor->ExpectSuccess();
        vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 2, 24);
        m_errorMonitor->VerifyNotFound();

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

        m_errorMonitor->ExpectSuccess();
        vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 2, 24);
        m_errorMonitor->VerifyNotFound();

        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

    } else {
        printf("%s Test requires unsupported multiDrawIndirect feature. Skipped.\n", kSkipPrefix);
    }
    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerTest, ValidateGeometryNV) {
    TEST_DESCRIPTION("Validate acceleration structure geometries.");
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkBufferObj vbo;
    vbo.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
             VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    VkBufferObj ibo;
    ibo.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
             VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    VkBufferObj tbo;
    tbo.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
             VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    VkBufferObj aabbbo;
    aabbbo.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

    VkBufferCreateInfo unbound_buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    unbound_buffer_ci.size = 1024;
    unbound_buffer_ci.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    VkBufferObj unbound_buffer;
    unbound_buffer.init_no_mem(*m_device, unbound_buffer_ci);

    const std::vector<float> vertices = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    const std::vector<uint32_t> indicies = {0, 1, 2};
    const std::vector<float> aabbs = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    const std::vector<float> transforms = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    uint8_t *mapped_vbo_buffer_data = (uint8_t *)vbo.memory().map();
    std::memcpy(mapped_vbo_buffer_data, (uint8_t *)vertices.data(), sizeof(float) * vertices.size());
    vbo.memory().unmap();

    uint8_t *mapped_ibo_buffer_data = (uint8_t *)ibo.memory().map();
    std::memcpy(mapped_ibo_buffer_data, (uint8_t *)indicies.data(), sizeof(uint32_t) * indicies.size());
    ibo.memory().unmap();

    uint8_t *mapped_tbo_buffer_data = (uint8_t *)tbo.memory().map();
    std::memcpy(mapped_tbo_buffer_data, (uint8_t *)transforms.data(), sizeof(float) * transforms.size());
    tbo.memory().unmap();

    uint8_t *mapped_aabbbo_buffer_data = (uint8_t *)aabbbo.memory().map();
    std::memcpy(mapped_aabbbo_buffer_data, (uint8_t *)aabbs.data(), sizeof(float) * aabbs.size());
    aabbbo.memory().unmap();

    VkGeometryNV valid_geometry_triangles = LvlInitStruct<VkGeometryNV>();
    valid_geometry_triangles.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    valid_geometry_triangles.geometry.triangles = LvlInitStruct<VkGeometryTrianglesNV>();
    valid_geometry_triangles.geometry.triangles.vertexData = vbo.handle();
    valid_geometry_triangles.geometry.triangles.vertexOffset = 0;
    valid_geometry_triangles.geometry.triangles.vertexCount = 3;
    valid_geometry_triangles.geometry.triangles.vertexStride = 12;
    valid_geometry_triangles.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    valid_geometry_triangles.geometry.triangles.indexData = ibo.handle();
    valid_geometry_triangles.geometry.triangles.indexOffset = 0;
    valid_geometry_triangles.geometry.triangles.indexCount = 3;
    valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    valid_geometry_triangles.geometry.triangles.transformData = tbo.handle();
    valid_geometry_triangles.geometry.triangles.transformOffset = 0;
    valid_geometry_triangles.geometry.aabbs = LvlInitStruct<VkGeometryAABBNV>();

    VkGeometryNV valid_geometry_aabbs = LvlInitStruct<VkGeometryNV>();
    valid_geometry_aabbs.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    valid_geometry_aabbs.geometry.triangles = LvlInitStruct<VkGeometryTrianglesNV>();
    valid_geometry_aabbs.geometry.aabbs = LvlInitStruct<VkGeometryAABBNV>();
    valid_geometry_aabbs.geometry.aabbs.aabbData = aabbbo.handle();
    valid_geometry_aabbs.geometry.aabbs.numAABBs = 1;
    valid_geometry_aabbs.geometry.aabbs.offset = 0;
    valid_geometry_aabbs.geometry.aabbs.stride = 24;

    PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV = reinterpret_cast<PFN_vkCreateAccelerationStructureNV>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateAccelerationStructureNV"));
    assert(vkCreateAccelerationStructureNV != nullptr);

    const auto GetCreateInfo = [](const VkGeometryNV &geometry) {
        VkAccelerationStructureCreateInfoNV as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
        as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
        as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        as_create_info.info.instanceCount = 0;
        as_create_info.info.geometryCount = 1;
        as_create_info.info.pGeometries = &geometry;
        return as_create_info;
    };

    VkAccelerationStructureNV as;

    // Invalid vertex format.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R64_UINT;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexFormat-02430");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - not multiple of component size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02429");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexOffset = 12 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02428");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid vertex buffer - no such buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexData = VkBuffer(123456789);

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexData-parameter");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
#if 0
    // XXX Subtest disabled because this is the wrong VUID.
    // No VUIDs currently exist to require memory is bound (spec bug).
    // Invalid vertex buffer - no memory bound.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.vertexData = unbound_buffer.handle();

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-vertexOffset-02428");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
#endif

    // Invalid index offset - not multiple of index size.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexOffset-02432");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexOffset-02431");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index count - must be 0 if type is VK_INDEX_TYPE_NONE_NV.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        geometry.geometry.triangles.indexData = VK_NULL_HANDLE;
        geometry.geometry.triangles.indexCount = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexCount-02436");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid index data - must be VK_NULL_HANDLE if type is VK_INDEX_TYPE_NONE_NV.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        geometry.geometry.triangles.indexData = ibo.handle();
        geometry.geometry.triangles.indexCount = 0;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-indexData-02434");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Invalid transform offset - not multiple of 16.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-transformOffset-02438");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid transform offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_triangles;
        geometry.geometry.triangles.transformOffset = 2048;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryTrianglesNV-transformOffset-02437");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Invalid aabb offset - not multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-offset-02440");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb offset - bigger than buffer.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.offset = 8 * 1024;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-offset-02439");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // Invalid aabb stride - not multiple of 8.
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.stride = 1;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryAABBNV-stride-02441");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // geometryType must be VK_GEOMETRY_TYPE_TRIANGLES_NV or VK_GEOMETRY_TYPE_AABBS_NV
    {
        VkGeometryNV geometry = valid_geometry_aabbs;
        geometry.geometry.aabbs.stride = 1;
        geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

        VkAccelerationStructureCreateInfoNV as_create_info = GetCreateInfo(geometry);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGeometryNV-geometryType-03503");
        m_errorMonitor->SetUnexpectedError("VUID-VkGeometryNV-geometryType-parameter");
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateCreateAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV = reinterpret_cast<PFN_vkCreateAccelerationStructureNV>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateAccelerationStructureNV"));
    assert(vkCreateAccelerationStructureNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();

    VkAccelerationStructureNV as = VK_NULL_HANDLE;

    // Top level can not have geometry
    {
        VkAccelerationStructureCreateInfoNV bad_top_level_create_info = as_create_info;
        bad_top_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
        bad_top_level_create_info.info.instanceCount = 0;
        bad_top_level_create_info.info.geometryCount = 1;
        bad_top_level_create_info.info.pGeometries = &geometry;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-type-02425");
        vkCreateAccelerationStructureNV(m_device->handle(), &bad_top_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Bot level can not have instances
    {
        VkAccelerationStructureCreateInfoNV bad_bot_level_create_info = as_create_info;
        bad_bot_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_bot_level_create_info.info.instanceCount = 1;
        bad_bot_level_create_info.info.geometryCount = 0;
        bad_bot_level_create_info.info.pGeometries = nullptr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-type-02426");
        vkCreateAccelerationStructureNV(m_device->handle(), &bad_bot_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Type must not be VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR
    {
        VkAccelerationStructureCreateInfoNV bad_bot_level_create_info = as_create_info;
        bad_bot_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR;
        bad_bot_level_create_info.info.instanceCount = 0;
        bad_bot_level_create_info.info.geometryCount = 0;
        bad_bot_level_create_info.info.pGeometries = nullptr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-type-04623");
        vkCreateAccelerationStructureNV(m_device->handle(), &bad_bot_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not prefer both fast trace and fast build
    {
        VkAccelerationStructureCreateInfoNV bad_flags_level_create_info = as_create_info;
        bad_flags_level_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_flags_level_create_info.info.instanceCount = 0;
        bad_flags_level_create_info.info.geometryCount = 1;
        bad_flags_level_create_info.info.pGeometries = &geometry;
        bad_flags_level_create_info.info.flags =
            VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-flags-02592");
        vkCreateAccelerationStructureNV(m_device->handle(), &bad_flags_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not have geometry or instance for compacting
    {
        VkAccelerationStructureCreateInfoNV bad_compacting_as_create_info = as_create_info;
        bad_compacting_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        bad_compacting_as_create_info.info.instanceCount = 0;
        bad_compacting_as_create_info.info.geometryCount = 1;
        bad_compacting_as_create_info.info.pGeometries = &geometry;
        bad_compacting_as_create_info.info.flags = 0;
        bad_compacting_as_create_info.compactedSize = 1024;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureCreateInfoNV-compactedSize-02421");
        vkCreateAccelerationStructureNV(m_device->handle(), &bad_compacting_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not mix different geometry types into single bottom level acceleration structure
    {
        VkGeometryNV aabb_geometry = LvlInitStruct<VkGeometryNV>();
        aabb_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
        aabb_geometry.geometry.triangles = LvlInitStruct<VkGeometryTrianglesNV>();
        aabb_geometry.geometry.aabbs = LvlInitStruct<VkGeometryAABBNV>();
        // Buffer contents do not matter for this test.
        aabb_geometry.geometry.aabbs.aabbData = geometry.geometry.triangles.vertexData;
        aabb_geometry.geometry.aabbs.numAABBs = 1;
        aabb_geometry.geometry.aabbs.offset = 0;
        aabb_geometry.geometry.aabbs.stride = 24;

        std::vector<VkGeometryNV> geometries = {geometry, aabb_geometry};

        VkAccelerationStructureCreateInfoNV mix_geometry_types_as_create_info = as_create_info;
        mix_geometry_types_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        mix_geometry_types_as_create_info.info.instanceCount = 0;
        mix_geometry_types_as_create_info.info.geometryCount = static_cast<uint32_t>(geometries.size());
        mix_geometry_types_as_create_info.info.pGeometries = geometries.data();
        mix_geometry_types_as_create_info.info.flags = 0;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkAccelerationStructureInfoNV-type-02786");
        vkCreateAccelerationStructureNV(m_device->handle(), &mix_geometry_types_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateCreateAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>(&ray_tracing_features);
    auto acc_struct_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_query_features);
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&acc_struct_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, false, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    if (ray_query_features.rayQuery == VK_FALSE && ray_tracing_features.rayTracingPipeline == VK_FALSE) {
        printf("%s Both of the required features rayQuery and rayTracing are not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateAccelerationStructureKHR"));
    assert(vkCreateAccelerationStructureKHR != nullptr);

    VkAccelerationStructureKHR as;
    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");

    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, buffer.handle()};
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &device_address_info);
    // invalid buffer;
    {
        VkBufferObj invalid_buffer;
        invalid_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR);
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.buffer = invalid_buffer.handle();
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-buffer-03614");
        vkCreateAccelerationStructureKHR(m_device->handle(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid deviceAddress and flag;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.deviceAddress = device_address;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-deviceAddress-03612");
        vkCreateAccelerationStructureKHR(m_device->handle(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();

        invalid_as_create_info.deviceAddress = 0;
        invalid_as_create_info.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_FLAG_BITS_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-parameter");
        vkCreateAccelerationStructureKHR(m_device->handle(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid size and offset;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.size = 4097;  // buffer size is 4096
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-offset-03616");
        vkCreateAccelerationStructureKHR(m_device->handle(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid sType;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-sType-sType");
        vkCreateAccelerationStructureKHR(m_device->handle(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // invalid type;
    {
        VkAccelerationStructureCreateInfoKHR invalid_as_create_info = as_create_info;
        invalid_as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-type-parameter");
        vkCreateAccelerationStructureKHR(m_device->handle(), &invalid_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateCreateAccelerationStructureKHRReplayFeature) {
    TEST_DESCRIPTION("Validate acceleration structure creation replay feature.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto acc_struct_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&acc_struct_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, false, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    acc_struct_features.accelerationStructureCaptureReplay = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateAccelerationStructureKHR"));
    assert(vkCreateAccelerationStructureKHR != nullptr);
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");
    assert(vkGetBufferDeviceAddressKHR != nullptr);

    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);

    VkBufferDeviceAddressInfo device_address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    device_address_info.buffer = buffer.handle();
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &device_address_info);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = device_address;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-03613");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateAccelerationStructureKHR-deviceAddress-03488");
    vkCreateAccelerationStructureKHR(m_device->handle(), &as_create_info, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateBindAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure binding.");
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkBindAccelerationStructureMemoryNV vkBindAccelerationStructureMemoryNV =
        reinterpret_cast<PFN_vkBindAccelerationStructureMemoryNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkBindAccelerationStructureMemoryNV"));
    assert(vkBindAccelerationStructureMemoryNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;
    as_create_info.info.instanceCount = 0;

    VkAccelerationStructureObj as(*m_device, as_create_info, false);
    m_errorMonitor->VerifyNotFound();

    VkMemoryRequirements as_memory_requirements = as.memory_requirements().memoryRequirements;

    VkBindAccelerationStructureMemoryInfoNV as_bind_info = LvlInitStruct<VkBindAccelerationStructureMemoryInfoNV>();
    as_bind_info.accelerationStructure = as.handle();

    VkMemoryAllocateInfo as_memory_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    as_memory_alloc.allocationSize = as_memory_requirements.size;
    ASSERT_TRUE(m_device->phy().set_memory_type(as_memory_requirements.memoryTypeBits, &as_memory_alloc, 0));

    // Can not bind already freed memory
    {
        VkDeviceMemory as_memory_freed = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_freed));
        vk::FreeMemory(device(), as_memory_freed, NULL);

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_freed = as_bind_info;
        as_bind_info_freed.memory = as_memory_freed;
        as_bind_info_freed.memoryOffset = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-parameter");
        (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_freed);
        m_errorMonitor->VerifyFound();
    }

    // Can not bind with bad alignment
    if (as_memory_requirements.alignment > 1) {
        VkMemoryAllocateInfo as_memory_alloc_bad_alignment = as_memory_alloc;
        as_memory_alloc_bad_alignment.allocationSize += 1;

        VkDeviceMemory as_memory_bad_alignment = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc_bad_alignment, NULL, &as_memory_bad_alignment));

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_alignment = as_bind_info;
        as_bind_info_bad_alignment.memory = as_memory_bad_alignment;
        as_bind_info_bad_alignment.memoryOffset = 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03623");
        (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_alignment);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_bad_alignment, NULL);
    }

    // Can not bind with offset outside the allocation
    {
        VkDeviceMemory as_memory_bad_offset = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

        VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_offset = as_bind_info;
        as_bind_info_bad_offset.memory = as_memory_bad_offset;
        as_bind_info_bad_offset.memoryOffset =
            (as_memory_alloc.allocationSize + as_memory_requirements.alignment) & ~(as_memory_requirements.alignment - 1);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03621");
        (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_bad_offset, NULL);
    }

    // Can not bind with offset that doesn't leave enough size
    {
        VkDeviceSize offset = (as_memory_requirements.size - 1) & ~(as_memory_requirements.alignment - 1);
        if (offset > 0 && (as_memory_requirements.size < (as_memory_alloc.allocationSize - as_memory_requirements.alignment))) {
            VkDeviceMemory as_memory_bad_offset = VK_NULL_HANDLE;
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_bad_offset));

            VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_offset = as_bind_info;
            as_bind_info_bad_offset.memory = as_memory_bad_offset;
            as_bind_info_bad_offset.memoryOffset = offset;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-size-03624");
            (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_offset);
            m_errorMonitor->VerifyFound();

            vk::FreeMemory(device(), as_memory_bad_offset, NULL);
        }
    }

    // Can not bind with memory that has unsupported memory type
    {
        VkPhysicalDeviceMemoryProperties memory_properties = {};
        vk::GetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memory_properties);

        uint32_t supported_memory_type_bits = as_memory_requirements.memoryTypeBits;
        uint32_t unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~supported_memory_type_bits;
        if (unsupported_mem_type_bits != 0) {
            VkMemoryAllocateInfo as_memory_alloc_bad_type = as_memory_alloc;
            ASSERT_TRUE(m_device->phy().set_memory_type(unsupported_mem_type_bits, &as_memory_alloc_bad_type, 0));

            VkDeviceMemory as_memory_bad_type = VK_NULL_HANDLE;
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc_bad_type, NULL, &as_memory_bad_type));

            VkBindAccelerationStructureMemoryInfoNV as_bind_info_bad_type = as_bind_info;
            as_bind_info_bad_type.memory = as_memory_bad_type;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-memory-03622");
            (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_bad_type);
            m_errorMonitor->VerifyFound();

            vk::FreeMemory(device(), as_memory_bad_type, NULL);
        }
    }

    // Can not bind memory twice
    {
        VkAccelerationStructureObj as_twice(*m_device, as_create_info, false);

        VkDeviceMemory as_memory_twice_1 = VK_NULL_HANDLE;
        VkDeviceMemory as_memory_twice_2 = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_1));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &as_memory_alloc, NULL, &as_memory_twice_2));
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_1 = as_bind_info;
        VkBindAccelerationStructureMemoryInfoNV as_bind_info_twice_2 = as_bind_info;
        as_bind_info_twice_1.accelerationStructure = as_twice.handle();
        as_bind_info_twice_2.accelerationStructure = as_twice.handle();
        as_bind_info_twice_1.memory = as_memory_twice_1;
        as_bind_info_twice_2.memory = as_memory_twice_2;

        ASSERT_VK_SUCCESS(vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_1));
        m_errorMonitor->VerifyNotFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoNV-accelerationStructure-03620");
        (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_2);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_twice_1, NULL);
        vk::FreeMemory(device(), as_memory_twice_2, NULL);
    }
}

TEST_F(VkLayerTest, ValidateWriteDescriptorSetAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
                           });

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    VkWriteDescriptorSetAccelerationStructureNV acc = LvlInitStruct<VkWriteDescriptorSetAccelerationStructureNV>();
    acc.accelerationStructureCount = 1;
    VkAccelerationStructureCreateInfoNV top_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    top_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    VkAccelerationStructureObj top_level_as(*m_device, top_level_as_create_info);

    acc.pAccelerationStructures = &top_level_as.handle();
    descriptor_write.pNext = &acc;
    m_errorMonitor->ExpectSuccess();
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, ValidateCmdBuildAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure building.");
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBuildAccelerationStructureNV"));
    assert(vkCmdBuildAccelerationStructureNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    bot_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info);
    m_errorMonitor->VerifyNotFound();

    VkBufferObj bot_level_as_scratch;
    bot_level_as.create_scratch_buffer(*m_device, &bot_level_as_scratch);

    // Command buffer must be in recording state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-recording");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // Incompatible type
    VkAccelerationStructureInfoNV as_build_info_with_incompatible_type = bot_level_as_create_info.info;
    as_build_info_with_incompatible_type.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    as_build_info_with_incompatible_type.instanceCount = 1;
    as_build_info_with_incompatible_type.geometryCount = 0;

    // This is duplicated since it triggers one error for different types and one error for lower instance count - the
    // build info is incompatible but still needs to be valid to get past the stateless checks.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_type, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible flags
    VkAccelerationStructureInfoNV as_build_info_with_incompatible_flags = bot_level_as_create_info.info;
    as_build_info_with_incompatible_flags.flags = VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_NV;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_flags, VK_NULL_HANDLE, 0,
                                      VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Incompatible build size
    VkGeometryNV geometry_with_more_vertices = geometry;
    geometry_with_more_vertices.geometry.triangles.vertexCount += 1;

    VkAccelerationStructureInfoNV as_build_info_with_incompatible_geometry = bot_level_as_create_info.info;
    as_build_info_with_incompatible_geometry.pGeometries = &geometry_with_more_vertices;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-dst-02488");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &as_build_info_with_incompatible_geometry, VK_NULL_HANDLE, 0,
                                      VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer too small
    VkBufferCreateInfo too_small_scratch_buffer_info = LvlInitStruct<VkBufferCreateInfo>();
    too_small_scratch_buffer_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    too_small_scratch_buffer_info.size = 1;
    VkBufferObj too_small_scratch_buffer(*m_device, too_small_scratch_buffer_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, too_small_scratch_buffer.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Scratch buffer with offset too small
    VkDeviceSize scratch_buffer_offset = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02491");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), scratch_buffer_offset);
    m_errorMonitor->VerifyFound();

    // Src must have been built before
    VkAccelerationStructureObj bot_level_as_updated(*m_device, bot_level_as_create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02489");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                      bot_level_as_updated.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Src must have been built before with the VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV flag
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyNotFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02490");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                      bot_level_as_updated.handle(), bot_level_as.handle(), bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid scratch buff
    VkBufferObj bot_level_as_invalid_scratch;
    VkBufferCreateInfo create_info = LvlInitStruct<VkBufferCreateInfo>();
    // invalid usage
    create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    bot_level_as.create_scratch_buffer(*m_device, &bot_level_as_invalid_scratch, &create_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-scratch-02781");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_invalid_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid instance data.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureInfoNV-instanceData-02782");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info,
                                      bot_level_as_invalid_scratch.handle(), 0, VK_FALSE, bot_level_as.handle(), VK_NULL_HANDLE,
                                      bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // must be called outside renderpass
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-renderpass");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ObjInUseCmdBuildAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto vkCmdBuildAccelerationStructureNV = reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBuildAccelerationStructureNV"));
    assert(vkCmdBuildAccelerationStructureNV != nullptr);
    auto vkDestroyAccelerationStructureNV = reinterpret_cast<PFN_vkDestroyAccelerationStructureNV>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkDestroyAccelerationStructureNV"));
    assert(vkCmdBuildAccelerationStructureNV != nullptr);

    auto vbo = layer_data::make_unique<VkBufferObj>();
    auto ibo = layer_data::make_unique<VkBufferObj>();
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, vbo.get(), ibo.get(), &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    bot_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    auto bot_level_as = layer_data::make_unique<VkAccelerationStructureObj>(*m_device, bot_level_as_create_info);

    auto bot_level_as_scratch = layer_data::make_unique<VkBufferObj>();
    bot_level_as->create_scratch_buffer(*m_device, bot_level_as_scratch.get());

    m_commandBuffer->begin();
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as->handle(), VK_NULL_HANDLE, bot_level_as_scratch->handle(), 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), ibo->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), vbo->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), bot_level_as_scratch->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyAccelerationStructureNV-accelerationStructure-03752");
    vkDestroyAccelerationStructureNV(m_device->handle(), bot_level_as->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::QueueWaitIdle(m_device->m_queue);

    ibo.reset();
    vbo.reset();
    bot_level_as_scratch.reset();
    bot_level_as.reset();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, ValidateGetAccelerationStructureHandleNV) {
    TEST_DESCRIPTION("Validate acceleration structure handle querying.");
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV =
        reinterpret_cast<PFN_vkGetAccelerationStructureHandleNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkGetAccelerationStructureHandleNV"));
    assert(vkGetAccelerationStructureHandleNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    bot_level_as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    bot_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    bot_level_as_create_info.info.instanceCount = 0;
    bot_level_as_create_info.info.geometryCount = 1;
    bot_level_as_create_info.info.pGeometries = &geometry;

    // Not enough space for the handle
    {
        VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info);
        m_errorMonitor->VerifyNotFound();

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAccelerationStructureHandleNV-dataSize-02240");
        vkGetAccelerationStructureHandleNV(m_device->handle(), bot_level_as.handle(), sizeof(uint8_t), &handle);
        m_errorMonitor->VerifyFound();
    }

    // No memory bound to acceleration structure
    {
        VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info, /*init_memory=*/false);
        m_errorMonitor->VerifyNotFound();

        uint64_t handle = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-vkGetAccelerationStructureHandleNV-accelerationStructure-XXXX");
        vkGetAccelerationStructureHandleNV(m_device->handle(), bot_level_as.handle(), sizeof(uint64_t), &handle);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateCmdCopyAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure copying.");
    if (!InitFrameworkForRayTracingTest(this, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdCopyAccelerationStructureNV vkCmdCopyAccelerationStructureNV = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureNV>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCmdCopyAccelerationStructureNV"));
    assert(vkCmdCopyAccelerationStructureNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoNV>();
    as_create_info.info = LvlInitStruct<VkAccelerationStructureInfoNV>();
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.instanceCount = 0;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;

    VkAccelerationStructureObj src_as(*m_device, as_create_info);
    VkAccelerationStructureObj dst_as(*m_device, as_create_info);
    VkAccelerationStructureObj dst_as_without_mem(*m_device, as_create_info, false);
    m_errorMonitor->VerifyNotFound();

    // Command buffer must be in recording state
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-recording");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();

    // Src must have been created with allow compaction flag
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-src-03411");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_NV);
    m_errorMonitor->VerifyFound();

    // Dst must have been bound with memory
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkAccelerationStructureNV");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as_without_mem.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);

    m_errorMonitor->VerifyFound();

    // mode must be VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR or VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-mode-03410");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR);
    m_errorMonitor->VerifyFound();

    // mode must be a valid VkCopyAccelerationStructureModeKHR value
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-mode-parameter");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_MAX_ENUM_KHR);
    m_errorMonitor->VerifyFound();

    // This command must only be called outside of a render pass instance
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureNV-renderpass");
    vkCmdCopyAccelerationStructureNV(m_commandBuffer->handle(), dst_as.handle(), src_as.handle(),
                                     VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_NV);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, QueueSubmitNoTimelineSemaphoreInfo) {
    TEST_DESCRIPTION("Submit a queue with a timeline semaphore but not a VkTimelineSemaphoreSubmitInfoKHR.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
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
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto timelineproperties = LvlInitStruct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = LvlInitStruct<VkTimelineSemaphoreSubmitInfoKHR>();
    uint64_t signalValue = 1;
    uint64_t waitValue = 3;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info[2] = {};
    submit_info[0] = LvlInitStruct<VkSubmitInfo>();
    submit_info[0].pNext = &timeline_semaphore_submit_info;
    submit_info[0].pWaitDstStageMask = &stageFlags;
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore;

    submit_info[1] = LvlInitStruct<VkSubmitInfo>();
    submit_info[1].pNext = &timeline_semaphore_submit_info;
    submit_info[1].pWaitDstStageMask = &stageFlags;
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &semaphore;

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-03241");
    vk::QueueSubmit(m_device->m_queue, 1, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pNext-03240");
    vk::QueueSubmit(m_device->m_queue, 2, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);

    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    semaphore_type_create_info.initialValue = 5;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03242");
    vk::QueueSubmit(m_device->m_queue, 1, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < UINT64_MAX) {
        semaphore_type_create_info.initialValue = 0;

        ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

        signalValue = timelineproperties.maxTimelineSemaphoreValueDifference + 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pSignalSemaphores-03244");
        vk::QueueSubmit(m_device->m_queue, 1, submit_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();

        if (signalValue < UINT64_MAX) {
            waitValue = signalValue + 1;
            signalValue = 1;

            timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
            timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitSemaphores-03243");
            vk::QueueSubmit(m_device->m_queue, 2, submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
        }

        vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    }
}

TEST_F(VkLayerTest, QueueSubmitBinarySemaphoreNotSignaled) {
    TEST_DESCRIPTION("Submit a queue with a waiting binary semaphore not previously signaled.");

    bool timelineSemaphoresExtensionSupported = true;

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        timelineSemaphoresExtensionSupported = false;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (timelineSemaphoresExtensionSupported &&
        DeviceExtensionSupported(gpu(), nullptr, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    } else {
        timelineSemaphoresExtensionSupported = false;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();

    VkSemaphore semaphore[3];
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[0]));
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[1]));
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[2]));

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info[3] = {};
    submit_info[0] = LvlInitStruct<VkSubmitInfo>();
    submit_info[0].pWaitDstStageMask = &stageFlags;
    submit_info[0].waitSemaphoreCount = 1;
    submit_info[0].pWaitSemaphores = &(semaphore[0]);
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &(semaphore[1]);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, timelineSemaphoresExtensionSupported
                                                        ? "VUID-vkQueueSubmit-pWaitSemaphores-03238"
                                                        : "VUID-vkQueueSubmit-pWaitSemaphores-00069");
    vk::QueueSubmit(m_device->m_queue, 1, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    submit_info[1] = LvlInitStruct<VkSubmitInfo>();
    submit_info[1].pWaitDstStageMask = &stageFlags;
    submit_info[1].waitSemaphoreCount = 1;
    submit_info[1].pWaitSemaphores = &(semaphore[1]);
    submit_info[1].signalSemaphoreCount = 1;
    submit_info[1].pSignalSemaphores = &(semaphore[2]);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, timelineSemaphoresExtensionSupported
                                                        ? "VUID-vkQueueSubmit-pWaitSemaphores-03238"
                                                        : "VUID-vkQueueSubmit-pWaitSemaphores-00069");
    vk::QueueSubmit(m_device->m_queue, 2, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    submit_info[2] = LvlInitStruct<VkSubmitInfo>();
    submit_info[2].signalSemaphoreCount = 1;
    submit_info[2].pSignalSemaphores = &(semaphore[0]);

    ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &(submit_info[2]), VK_NULL_HANDLE));
    ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 2, submit_info, VK_NULL_HANDLE));

    ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    vk::DestroySemaphore(m_device->device(), semaphore[0], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[1], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[2], nullptr);
}

TEST_F(VkLayerTest, QueueSubmitTimelineSemaphoreOutOfOrder) {
    TEST_DESCRIPTION("Submit out-of-order timeline semaphores.");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
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
            printf("%s Multiple queues are required to run this test. .\n", kSkipPrefix);
            return;
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
#ifdef _WIN32
    printf("%s Test doesn't currently support Win32 semaphore, skipping test\n", kSkipPrefix);
    return;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;

    // Check for external semaphore instance extensions
    if (InstanceExtensionSupported(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s External semaphore extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Check for external semaphore device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, extension_name)) {
        m_device_extension_names.push_back(extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    } else {
        printf("%s External semaphore extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Create a semaphore fpr importing
    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    semaphore_create_info.flags = 0;
    VkSemaphore import_semaphore;
    VkResult err = vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &import_semaphore);
    ASSERT_VK_SUCCESS(err);

    int fd = 0;
    VkImportSemaphoreFdInfoKHR import_semaphore_fd_info = LvlInitStruct<VkImportSemaphoreFdInfoKHR>();
    import_semaphore_fd_info.semaphore = import_semaphore;
    import_semaphore_fd_info.flags = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR;
    import_semaphore_fd_info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;
    import_semaphore_fd_info.fd = fd;
    auto vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)vk::GetDeviceProcAddr(m_device->device(), "vkImportSemaphoreFdKHR");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImportSemaphoreFdInfoKHR-handleType-01143");
    vkImportSemaphoreFdKHR(device(), &import_semaphore_fd_info);
    m_errorMonitor->VerifyFound();

    // Cleanup
    vk::DestroySemaphore(device(), import_semaphore, nullptr);
#endif
}

TEST_F(VkLayerTest, InvalidWaitSemaphoresType) {
    TEST_DESCRIPTION("Wait for a non Timeline Semaphore");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
        return;
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
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

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    auto timelinefeatures = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&timelinefeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!timelinefeatures.timelineSemaphore) {
        printf("%s Timeline semaphores are not supported.\n", kSkipPrefix);
        return;
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
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
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

            m_errorMonitor->ExpectSuccess();

            vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

            semaphore_signal_info.semaphore = timeline_sem;
            semaphore_signal_info.value = 1;
            vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);

            m_errorMonitor->VerifyNotFound();

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
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&sync2_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!sync2_features.synchronization2) {
        GTEST_SKIP() << "VkPhysicalDeviceSynchronization2FeaturesKHR::synchronization2 required";
    }
    if (!vk12_features.timelineSemaphore) {
        GTEST_SKIP() << "VkPhysicalDeviceVulkan12Features::timelineSemaphore required";
    }
    InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    auto fpQueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2KHR");

    auto timelineproperties = LvlInitStruct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    vk::GetPhysicalDeviceProperties2(gpu(), &prop2);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info = LvlInitStruct<VkSemaphoreTypeCreateInfoKHR>();
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_create_info);

    VkSemaphore semaphore[2];
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[0]));
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[1]));

    VkSemaphoreSignalInfo semaphore_signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
    semaphore_signal_info.semaphore = semaphore[0];
    semaphore_signal_info.value = 10;
    ASSERT_VK_SUCCESS(vk::SignalSemaphore(m_device->device(), &semaphore_signal_info));

    auto signal_info = lvl_init_struct<VkSemaphoreSubmitInfoKHR>();
    signal_info.value = 20;
    signal_info.semaphore = semaphore[0];

    auto wait_info = lvl_init_struct<VkSemaphoreSubmitInfoKHR>();
    wait_info.value = 10;
    wait_info.semaphore = semaphore[1];
    wait_info.stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    auto submit_info = lvl_init_struct<VkSubmitInfo2KHR>();
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_info;
    submit_info.waitSemaphoreInfoCount = 1;
    submit_info.pWaitSemaphoreInfos = &wait_info;

    ASSERT_VK_SUCCESS(fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));

    semaphore_signal_info.value = 25;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03259");
    vk::SignalSemaphore(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    semaphore_signal_info.value = 15;
    ASSERT_VK_SUCCESS(vk::SignalSemaphore(m_device->device(), &semaphore_signal_info));
    semaphore_signal_info.semaphore = semaphore[1];
    ASSERT_VK_SUCCESS(vk::SignalSemaphore(m_device->device(), &semaphore_signal_info));

    // Check if we can test violations of maxTimelineSemaphoreValueDifference
    if (timelineproperties.maxTimelineSemaphoreValueDifference < UINT64_MAX) {
        // Regression test for value difference validations ran against binary semaphores
        {
            VkSemaphore timeline_sem;
            VkSemaphore binary_sem;

            semaphore_type_create_info.initialValue = 0;
            ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &timeline_sem));

            VkSemaphoreCreateInfo binary_semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
            ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &binary_semaphore_create_info, nullptr, &binary_sem));

            wait_info.semaphore = timeline_sem;
            wait_info.value = 1;

            signal_info.semaphore = binary_sem;
            signal_info.value = timelineproperties.maxTimelineSemaphoreValueDifference + 1;

            m_errorMonitor->ExpectSuccess();

            fpQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

            semaphore_signal_info.semaphore = timeline_sem;
            semaphore_signal_info.value = 1;
            vk::SignalSemaphore(m_device->device(), &semaphore_signal_info);

            m_errorMonitor->VerifyNotFound();

            vk::DestroySemaphore(m_device->device(), binary_sem, nullptr);
            vk::DestroySemaphore(m_device->device(), timeline_sem, nullptr);
        }
    }

    ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    vk::DestroySemaphore(m_device->device(), semaphore[0], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[1], nullptr);
}

TEST_F(VkLayerTest, InvalidSemaphoreCounterType) {
    TEST_DESCRIPTION("Get payload from a non Timeline Semaphore");

    AddRequiredExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    auto timelinefeatures = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&timelinefeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!timelinefeatures.timelineSemaphore) {
        printf("%s Timeline semaphores are not supported.\n", kSkipPrefix);
        return;
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
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    bool idfm_extensions = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    idfm_extensions = idfm_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    idfm_extensions = idfm_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    idfm_extensions = idfm_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    idfm_extensions = idfm_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    idfm_extensions = idfm_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

    if (idfm_extensions) {
        m_device_extension_names.push_back(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
        return;
    }

    PFN_vkGetImageDrmFormatModifierPropertiesEXT vkGetImageDrmFormatModifierPropertiesEXT =
        (PFN_vkGetImageDrmFormatModifierPropertiesEXT)vk::GetInstanceProcAddr(instance(),
                                                                              "vkGetImageDrmFormatModifierPropertiesEXT");
    ASSERT_TRUE(vkGetImageDrmFormatModifierPropertiesEXT != nullptr);

    ASSERT_NO_FATAL_FAILURE(InitState());

    const uint64_t dummy_modifiers[2] = {0, 1};

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
    drm_format_mod_list.drmFormatModifierCount = 2;
    drm_format_mod_list.pDrmFormatModifiers = dummy_modifiers;

    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_mod_explicit =
        LvlInitStruct<VkImageDrmFormatModifierExplicitCreateInfoEXT>();
    drm_format_mod_explicit.drmFormatModifierPlaneCount = 1;
    drm_format_mod_explicit.pPlaneLayouts = &dummyPlaneLayout;

    VkImage image = VK_NULL_HANDLE;

    // No pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-tiling-02261");
    vk::CreateImage(device(), &image_info, nullptr, &image);
    m_errorMonitor->VerifyFound();

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
    m_errorMonitor->ExpectSuccess();
    vk::CreateImage(device(), &image_info, nullptr, &image);
    vk::DestroyImage(device(), image, nullptr);
    m_errorMonitor->VerifyNotFound();

    image_info.pNext = (void *)&drm_format_mod_explicit;
    m_errorMonitor->ExpectSuccess();
    vk::CreateImage(device(), &image_info, nullptr, &image);
    vk::DestroyImage(device(), image, nullptr);
    m_errorMonitor->VerifyNotFound();

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
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
    } else {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix,
               VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
        return;
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

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

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

    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&protect_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (protect_features.protectedMemory == VK_TRUE) {
        result = vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
        if (result != VK_SUCCESS) {
            printf("%s CreateDevice returned back %s, skipping rest of tests\n", kSkipPrefix, string_VkResult(result));
            return;
        }

        // TODO: Re-enable test when Vulkan-Loader MR #581 is resolved and upstream into next SDK - tested ToT
        // Try using GetDeviceQueue with a queue that has as flag
        // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-flags-01841");
        // vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);
        // m_errorMonitor->VerifyFound();

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
        printf("%s CreateDevice returned back %s, skipping rest of tests\n", kSkipPrefix, string_VkResult(result));
        return;
    }

    // TODO: Re-enable test when Vulkan-Loader MR #581 is resolved and upstream into next SDK - tested ToT
    // if (queue_properties.queueCount > 1) {
    //     // Set queueIndex 1 over size of queueCount used to create device
    //     m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-queueIndex-00385");
    //     vk::GetDeviceQueue(test_device, queue_family_index, 1, &test_queue);
    //     m_errorMonitor->VerifyFound();
    // }

    // Use an unknown queue family index
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-queueFamilyIndex-00384");
    // vk::GetDeviceQueue(test_device, queue_family_index + 1, 0, &test_queue);
    // m_errorMonitor->VerifyFound();

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
    m_errorMonitor->ExpectSuccess();
    vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);
    m_errorMonitor->VerifyNotFound();

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
        printf("%s test requires queue family with 2 queues, not available.  Skipping.\n", kSkipPrefix);
        return;
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
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto protected_features = LvlInitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&protected_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (protected_features.protectedMemory == VK_FALSE) {
        printf("%s test requires protectedMemory, not available.  Skipping.\n", kSkipPrefix);
        return;
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
        printf("%s test requires queue with VK_QUEUE_PROTECTED_BIT with 2 queues, not available.  Skipping.\n", kSkipPrefix);
        return;
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
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto protected_features = LvlInitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&protected_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (protected_features.protectedMemory == VK_FALSE) {
        printf("%s test requires protectedMemory, not available. Skipping.\n", kSkipPrefix);
        return;
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
        printf("%s test requires queue without VK_QUEUE_PROTECTED_BIT. Skipping.\n", kSkipPrefix);
        return;
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto protected_memory_features = LvlInitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&protected_memory_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        printf("%s protectedMemory feature not supported, skipped.\n", kSkipPrefix);
        return;
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
    m_errorMonitor->ExpectSuccess();
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer_protected);
    buffer_create_info.flags = 0;
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer_unprotected);
    m_errorMonitor->VerifyNotFound();

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
    m_errorMonitor->ExpectSuccess();
    image_create_info.flags = VK_IMAGE_CREATE_PROTECTED_BIT;
    vk::CreateImage(device(), &image_create_info, nullptr, &image_protected);
    image_create_info.flags = 0;
    vk::CreateImage(device(), &image_create_info, nullptr, &image_unprotected);
    m_errorMonitor->VerifyNotFound();

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
        printf("%s No valid memory type index could be found; skipped.\n", kSkipPrefix);
        vk::DestroyImage(device(), image_protected, nullptr);
        vk::DestroyImage(device(), image_unprotected, nullptr);
        vk::DestroyBuffer(device(), buffer_protected, nullptr);
        vk::DestroyBuffer(device(), buffer_unprotected, nullptr);
        return;
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

TEST_F(VkLayerTest, ValidateCmdTraceRaysKHR) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, false, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkBuffer buffer;
    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags);
    alloc_info.allocationSize = 4096;
    VkDeviceMemory mem;
    err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);
    vk::BindBufferMemory(device(), buffer, mem, 0);

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto ray_tracing_properties = LvlInitStruct<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)vk::GetInstanceProcAddr(instance(), "vkCmdTraceRaysKHR");
    ASSERT_TRUE(vkCmdTraceRaysKHR != nullptr);
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");

    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, buffer};
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &device_address_info);

    VkStridedDeviceAddressRegionKHR stridebufregion = {};
    stridebufregion.deviceAddress = device_address;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleAlignment;
    stridebufregion.size = stridebufregion.stride;

    m_commandBuffer->begin();
    // Invalid stride multiplier
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = (stridebufregion.size + 1) % stridebufregion.size;
        if (invalid_stride.stride > 0) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03694");
            vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                              100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03690");
            vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                              100, 1);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-03686");
            vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                              100, 1);
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride, greater than maxShaderGroupStride
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        uint32_t align = ray_tracing_properties.shaderGroupHandleSize;
        invalid_stride.stride =
            ray_tracing_properties.maxShaderGroupStride + (align - (ray_tracing_properties.maxShaderGroupStride % align));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04041");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04035");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04029");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
    vk::DestroyBuffer(device(), buffer, nullptr);
    vk::FreeMemory(device(), mem, nullptr);
}

TEST_F(VkLayerTest, ValidateCmdTraceRaysIndirectKHR) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirectKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&ray_tracing_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, false, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
 
    if (ray_tracing_features.rayTracingPipelineTraceRaysIndirect == VK_FALSE) {
        printf("%s rayTracingIndirectTraceRays not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    VkBuffer buffer;
    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags);
    alloc_info.allocationSize = 4096;
    VkDeviceMemory mem;
    err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);
    vk::BindBufferMemory(device(), buffer, mem, 0);

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto ray_tracing_properties = LvlInitStruct<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    PFN_vkCmdTraceRaysIndirectKHR vkCmdTraceRaysIndirectKHR =
        (PFN_vkCmdTraceRaysIndirectKHR)vk::GetInstanceProcAddr(instance(), "vkCmdTraceRaysIndirectKHR");
    ASSERT_TRUE(vkCmdTraceRaysIndirectKHR != nullptr);

    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");

    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, buffer};
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &device_address_info);

    VkStridedDeviceAddressRegionKHR stridebufregion = {};
    stridebufregion.deviceAddress = device_address;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleAlignment;
    stridebufregion.size = stridebufregion.stride;

    m_commandBuffer->begin();
    // Invalid stride multiplier
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = (stridebufregion.size + 1) % stridebufregion.size;
        if (invalid_stride.stride > 0) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03694");
            vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion,
                                      &invalid_stride, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03690");
            vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride,
                                      &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-03686");
            vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion,
                                      &stridebufregion, device_address);
            m_errorMonitor->VerifyFound();
        }
    }
    // Invalid stride, greater than maxShaderGroupStride
    {
        VkStridedDeviceAddressRegionKHR invalid_stride = stridebufregion;
        uint32_t align = ray_tracing_properties.shaderGroupHandleSize;
        invalid_stride.stride =
            ray_tracing_properties.maxShaderGroupStride + (align - (ray_tracing_properties.maxShaderGroupStride % align));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04041");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride,
                                  device_address);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04035");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion,
                                  device_address);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04029");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion,
                                  device_address);
        m_errorMonitor->VerifyFound();
    }
    m_commandBuffer->end();
    vk::DestroyBuffer(device(), buffer, nullptr);
    vk::FreeMemory(device(), mem, nullptr);
}

TEST_F(VkLayerTest, ValidateVkAccelerationStructureVersionInfoKHR) {
    TEST_DESCRIPTION("Validate VkAccelerationStructureVersionInfoKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (!InitFrameworkForRayTracingTest(this, true, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (ray_tracing_features.rayTracingPipeline == VK_FALSE) {
        printf("%s rayTracing not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ray_tracing_features));
    PFN_vkGetDeviceAccelerationStructureCompatibilityKHR vkGetDeviceAccelerationStructureCompatibilityKHR =
        (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetDeviceAccelerationStructureCompatibilityKHR");
    ASSERT_TRUE(vkGetDeviceAccelerationStructureCompatibilityKHR != nullptr);
    VkAccelerationStructureVersionInfoKHR valid_version = LvlInitStruct<VkAccelerationStructureVersionInfoKHR>();
    VkAccelerationStructureCompatibilityKHR compatablity;
    uint8_t mode[] = {VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR, VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR};
    valid_version.pVersionData = mode;
    {
        VkAccelerationStructureVersionInfoKHR invalid_version = valid_version;
        invalid_version.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionInfoKHR-sType-sType");
        vkGetDeviceAccelerationStructureCompatibilityKHR(m_device->handle(), &invalid_version, &compatablity);
        m_errorMonitor->VerifyFound();
    }

    {
        VkAccelerationStructureVersionInfoKHR invalid_version = valid_version;
        invalid_version.pVersionData = NULL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionInfoKHR-pVersionData-parameter");
        vkGetDeviceAccelerationStructureCompatibilityKHR(m_device->handle(), &invalid_version, &compatablity);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateCmdBuildAccelerationStructuresKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    accel_features.accelerationStructureIndirectBuild = VK_TRUE;
    accel_features.accelerationStructureHostCommands = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, false, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR =
        (PFN_vkCmdBuildAccelerationStructuresKHR)vk::GetDeviceProcAddr(device(), "vkCmdBuildAccelerationStructuresKHR");
    assert(vkCmdBuildAccelerationStructuresKHR != nullptr);

    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");

    auto vkCmdBuildAccelerationStructuresIndirectKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresIndirectKHR>(
        vk::GetDeviceProcAddr(device(), "vkCmdBuildAccelerationStructuresIndirectKHR"));
    assert(vkCmdBuildAccelerationStructuresKHR);

    auto vkBuildAccelerationStructuresKHR =
        reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vk::GetDeviceProcAddr(device(), "vkBuildAccelerationStructuresKHR"));
    assert(vkBuildAccelerationStructuresKHR);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometryNV;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometryNV, 0, true);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj bot_level_as(*m_device, as_create_info);

    VkBufferDeviceAddressInfo vertexAddressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                   geometryNV.geometry.triangles.vertexData};
    VkDeviceAddress vertexAddress = vkGetBufferDeviceAddressKHR(m_device->handle(), &vertexAddressInfo);

    VkBufferDeviceAddressInfo indexAddressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                  geometryNV.geometry.triangles.indexData};
    VkDeviceAddress indexAddress = vkGetBufferDeviceAddressKHR(m_device->handle(), &indexAddressInfo);
    VkAccelerationStructureGeometryKHR valid_geometry_triangles = LvlInitStruct<VkAccelerationStructureGeometryKHR>();
    valid_geometry_triangles.geometryType = geometryNV.geometryType;
    valid_geometry_triangles.geometry.triangles = LvlInitStruct<VkAccelerationStructureGeometryTrianglesDataKHR>();
    valid_geometry_triangles.geometry.triangles.vertexFormat = geometryNV.geometry.triangles.vertexFormat;
    valid_geometry_triangles.geometry.triangles.vertexData.deviceAddress = vertexAddress;
    valid_geometry_triangles.geometry.triangles.vertexStride = 8;
    valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    valid_geometry_triangles.geometry.triangles.indexData.deviceAddress = indexAddress;
    valid_geometry_triangles.geometry.triangles.transformData.deviceAddress = 0;
    valid_geometry_triangles.geometry.triangles.maxVertex = 1;
    valid_geometry_triangles.flags = 0;
    VkAccelerationStructureGeometryKHR *pGeometry = &valid_geometry_triangles;

    VkAccelerationStructureBuildGeometryInfoKHR build_info_khr = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto acc_struct_properties = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&acc_struct_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    VkBufferObj bot_level_as_scratch;
    VkBufferCreateInfo create_info = LvlInitStruct<VkBufferCreateInfo>();
    create_info.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    create_info.size = acc_struct_properties.minAccelerationStructureScratchOffsetAlignment;
    bot_level_as.create_scratch_buffer(*m_device, &bot_level_as_scratch, &create_info, true);
    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                     bot_level_as_scratch.handle()};
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &device_address_info);
    build_info_khr.flags = 0;
    build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info_khr.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info_khr.dstAccelerationStructure = bot_level_as.handle();
    build_info_khr.geometryCount = 1;
    build_info_khr.pGeometries = pGeometry;
    build_info_khr.ppGeometries = NULL;
    build_info_khr.scratchData.deviceAddress = device_address;

    VkAccelerationStructureBuildGeometryInfoKHR build_info_ppGeometries_khr = build_info_khr;
    build_info_ppGeometries_khr.pGeometries = NULL;
    build_info_ppGeometries_khr.ppGeometries = &pGeometry;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info;
    build_range_info.firstVertex = 0;
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 3;
    build_range_info.transformOffset = 0;
    VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos = &build_range_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-recording");
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
    m_errorMonitor->VerifyFound();

    {  // dstAccelerationStructure == VK_NULL_HANDLE
        auto build_info_null_dst_handle = build_info_khr;
        build_info_null_dst_handle.dstAccelerationStructure = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_null_dst_handle, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();

        if (accel_features.accelerationStructureIndirectBuild == VK_TRUE) {
            VkDeviceAddress range_ptrs{};
            uint32_t indirect_strides = sizeof(VkAccelerationStructureBuildRangeInfoKHR);
            uint32_t max_prim_counts[1] = {1};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03800");
            vkCmdBuildAccelerationStructuresIndirectKHR(m_commandBuffer->handle(), 1, &build_info_null_dst_handle, &range_ptrs,
                                                        &indirect_strides, reinterpret_cast<uint32_t **>(&max_prim_counts));
            m_errorMonitor->VerifyFound();
        }

        if (accel_features.accelerationStructureHostCommands == VK_TRUE) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03800");
            vkBuildAccelerationStructuresKHR(m_device->handle(), VK_NULL_HANDLE, 1, &build_info_null_dst_handle, &pBuildRangeInfos);
            m_errorMonitor->VerifyFound();
        }
    }

    m_commandBuffer->begin();

    m_errorMonitor->ExpectSuccess(kErrorBit);
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
    m_errorMonitor->VerifyNotFound();
    m_errorMonitor->ExpectSuccess(kErrorBit);
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_ppGeometries_khr, &pBuildRangeInfos);
    m_errorMonitor->VerifyNotFound();
    // Invalid info count
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-infoCount-arraylength");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 0, &build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }

    // Invalid pInfos
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-parameter");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, NULL, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }

    // Invalid ppBuildRangeInfos
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-ppBuildRangeInfos-parameter");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, NULL);
        m_errorMonitor->VerifyFound();
    }

    // must be called outside renderpass
    {
        ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-renderpass");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
        m_commandBuffer->EndRenderPass();
        m_errorMonitor->VerifyFound();
    }
    // Invalid flags
    {
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.flags = VK_BUILD_ACCELERATION_STRUCTURE_FLAG_BITS_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-flags-parameter");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
    // Invalid dst buffer
    {
        auto buffer_ci =
            VkBufferObj::create_info(4096, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
        VkBufferObj invalid_buffer;
        invalid_buffer.init_no_mem(*m_device, buffer_ci);
        as_create_info.buffer = invalid_buffer.handle();
        as_create_info.createFlags = 0;
        as_create_info.offset = 0;
        as_create_info.size = 0;
        as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        as_create_info.deviceAddress = 0;
        VkAccelerationStructurekhrObj invalid_bot_level_as(*m_device, as_create_info);

        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.dstAccelerationStructure = invalid_bot_level_as.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03707");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
    // Invalid sType
    {
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-sType-sType");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
    // Invalid Type
    {
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-03654");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();

        invalid_build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-type-parameter");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }

    // Invalid VetexStride and Indexdata
    {
        VkAccelerationStructureGeometryKHR invalid_geometry_triangles = valid_geometry_triangles;
        invalid_geometry_triangles.geometry.triangles.vertexStride = UINT32_MAX + 1ull;
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.pGeometries = &invalid_geometry_triangles;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-vertexStride-03819");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();

        invalid_geometry_triangles = valid_geometry_triangles;
        invalid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT8_EXT;
        invalid_build_info_khr.pGeometries = &invalid_geometry_triangles;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureGeometryTrianglesDataKHR-indexType-03798");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }

    // ppGeometries and pGeometries both valid pointer
    {
        VkAccelerationStructureBuildGeometryInfoKHR invalid_build_info_khr = build_info_khr;
        invalid_build_info_khr.ppGeometries = &invalid_build_info_khr.pGeometries;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-pGeometries-03788");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &invalid_build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }

    // Buffer is missing VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR usage flag
    {
        VkBufferObj bad_usage_buffer;
        auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        bad_usage_buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);
        VkBufferDeviceAddressInfo addr_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, bad_usage_buffer.handle()};
        auto addr = vkGetBufferDeviceAddressKHR(m_device->handle(), &addr_info);
        pGeometry[0].geometry.triangles.vertexData.deviceAddress = addr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-geometry-03673");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
        pGeometry[0] = valid_geometry_triangles;  // reset to valid
    }

    // Scratch data buffer is missing VK_BUFFER_USAGE_STORAGE_BUFFER_BIT usage flag
    {
        VkBufferObj bad_scratch;
        VkBufferCreateInfo bad_create_info = LvlInitStruct<VkBufferCreateInfo>();
        bad_create_info.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        bad_create_info.size = acc_struct_properties.minAccelerationStructureScratchOffsetAlignment;
        bot_level_as.create_scratch_buffer(*m_device, &bad_scratch, &bad_create_info, true);
        VkBufferDeviceAddressInfo bad_device_address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
        bad_device_address_info.buffer = bad_scratch.handle();
        VkDeviceAddress bad_device_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &bad_device_address_info);

        build_info_khr.scratchData.deviceAddress = bad_device_address;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03674");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
    // Scratch data buffer is 0
    {
        build_info_khr.scratchData.deviceAddress = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructuresKHR-pInfos-03802");
        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info_khr, &pBuildRangeInfos);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ObjInUseCmdBuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building tracks the objects used.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    accel_features.accelerationStructureIndirectBuild = VK_TRUE;
    accel_features.accelerationStructureHostCommands = VK_TRUE;
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, false, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
        vk::GetDeviceProcAddr(device(), "vkCmdBuildAccelerationStructuresKHR"));
    assert(vkCmdBuildAccelerationStructuresKHR != nullptr);

    auto vkGetBufferDeviceAddressKHR =
        reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR"));

    auto vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(device(), "vkDestroyAccelerationStructureKHR"));
    assert(vkDestroyAccelerationStructureKHR);

    auto vbo = layer_data::make_unique<VkBufferObj>();
    auto ibo = layer_data::make_unique<VkBufferObj>();
    VkGeometryNV geometryNV;
    const VkDeviceSize kBufferOffset = 256;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, vbo.get(), ibo.get(), &geometryNV, kBufferOffset, true);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    auto bot_level_as = layer_data::make_unique<VkAccelerationStructurekhrObj>(*m_device, as_create_info);

    auto address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    address_info.buffer = geometryNV.geometry.triangles.vertexData;
    VkDeviceAddress vertexAddress = vkGetBufferDeviceAddressKHR(m_device->handle(), &address_info);

    address_info.buffer = geometryNV.geometry.triangles.indexData;
    VkDeviceAddress indexAddress = vkGetBufferDeviceAddressKHR(m_device->handle(), &address_info);

    auto valid_geometry_triangles = LvlInitStruct<VkAccelerationStructureGeometryKHR>();
    valid_geometry_triangles.geometryType = geometryNV.geometryType;
    valid_geometry_triangles.geometry.triangles = LvlInitStruct<VkAccelerationStructureGeometryTrianglesDataKHR>();
    valid_geometry_triangles.geometry.triangles.vertexFormat = geometryNV.geometry.triangles.vertexFormat;
    valid_geometry_triangles.geometry.triangles.vertexData.deviceAddress = vertexAddress + kBufferOffset;
    valid_geometry_triangles.geometry.triangles.vertexStride = 8;
    valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    valid_geometry_triangles.geometry.triangles.indexData.deviceAddress = indexAddress + kBufferOffset;
    valid_geometry_triangles.geometry.triangles.transformData.deviceAddress = 0;
    valid_geometry_triangles.geometry.triangles.maxVertex = 1;
    valid_geometry_triangles.flags = 0;

    auto vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto acc_struct_properties = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&acc_struct_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    auto bot_level_as_scratch = layer_data::make_unique<VkBufferObj>();
    VkBufferCreateInfo create_info = LvlInitStruct<VkBufferCreateInfo>();
    create_info.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    create_info.size = std::max<VkDeviceSize>(acc_struct_properties.minAccelerationStructureScratchOffsetAlignment, kBufferOffset) * 2;
    bot_level_as->create_scratch_buffer(*m_device, bot_level_as_scratch.get(), &create_info, true);

    address_info.buffer = bot_level_as_scratch->handle();
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &address_info);

    auto build_info = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    build_info.flags = 0;
    build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = bot_level_as->handle();
    build_info.geometryCount = 1;
    build_info.pGeometries = &valid_geometry_triangles;
    build_info.ppGeometries = NULL;
    build_info.scratchData.deviceAddress = device_address + kBufferOffset;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info{};
    build_range_info.firstVertex = 0;
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 3;
    build_range_info.transformOffset = 0;

    VkAccelerationStructureBuildRangeInfoKHR *range_infos[1];
    range_infos[0] = &build_range_info;

    m_commandBuffer->begin();
    vkCmdBuildAccelerationStructuresKHR(m_commandBuffer->handle(), 1, &build_info, range_infos);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), ibo->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), vbo->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-00922");
    vk::DestroyBuffer(m_device->handle(), bot_level_as_scratch->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyAccelerationStructureKHR-accelerationStructure-02442");
    vkDestroyAccelerationStructureKHR(m_device->handle(), bot_level_as->handle(), nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::QueueWaitIdle(m_device->m_queue);

    ibo.reset();
    vbo.reset();
    bot_level_as_scratch.reset();
    bot_level_as.reset();
    m_errorMonitor->VerifyNotFound();
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

    // Check for external memory instance extensions
    std::vector<const char *> reqd_instance_extensions = {
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME}};
    for (auto extension_name : reqd_instance_extensions) {
        if (InstanceExtensionSupported(extension_name)) {
            m_instance_extension_names.push_back(extension_name);
        } else {
            printf("%s Required instance extension %s not supported, skipping test\n", kSkipPrefix, extension_name);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    auto vkGetPhysicalDeviceExternalBufferPropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceExternalBufferPropertiesKHR");

    // Check for import/export capability
    // export used to feed memory to test import
    VkPhysicalDeviceExternalBufferInfoKHR ebi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO_KHR, nullptr, 0,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, handle_type};
    VkExternalBufferPropertiesKHR ebp = {VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES_KHR, nullptr, {0, 0, 0}};
    ASSERT_TRUE(vkGetPhysicalDeviceExternalBufferPropertiesKHR != nullptr);
    vkGetPhysicalDeviceExternalBufferPropertiesKHR(gpu(), &ebi, &ebp);
    if (!(ebp.externalMemoryProperties.compatibleHandleTypes & handle_type) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(ebp.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT_KHR)) {
        printf("%s External buffer does not support importing and exporting, skipping test\n", kSkipPrefix);
        return;
    }

    // Always use dedicated allocation
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s Dedicated allocation extension not supported, skipping test\n", kSkipPrefix);
        return;
    }

    // Check for external memory device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, ext_mem_extension_name)) {
        m_device_extension_names.push_back(ext_mem_extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    } else {
        printf("%s External memory extension not supported, skipping test\n", kSkipPrefix);
        return;
    }

    // Check for bind memory 2
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    } else {
        printf("%s bind memory 2 extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkBindBufferMemory2KHR vkBindBufferMemory2Function =
        (PFN_vkBindBufferMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindBufferMemory2KHR");
    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function =
        (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");

    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize buffer_size = 1024;

    // Create export and import buffers
    VkExternalMemoryBufferCreateInfoKHR external_buffer_info = {VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR, nullptr,
                                                                handle_type};
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
    VkMemoryDedicatedAllocateInfoKHR dedicated_info = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR, nullptr,
                                                       VK_NULL_HANDLE, buffer_export.handle()};
    VkExportMemoryAllocateInfoKHR export_info = {VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR, &dedicated_info, handle_type};
    alloc_info.pNext = &export_info;

    // Allocate memory to be exported
    vk_testing::DeviceMemory memory_buffer_export;
    memory_buffer_export.init(*m_device, alloc_info);

    // Bind exported memory
    buffer_export.bind_memory(memory_buffer_export, 0);

    VkExternalMemoryImageCreateInfoKHR external_image_info = {VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO, nullptr,
                                                              handle_type};
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
    auto vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vk::GetInstanceProcAddr(instance(), "vkGetMemoryFdKHR");
    ASSERT_TRUE(vkGetMemoryFdKHR != nullptr);
    VkMemoryGetFdInfoKHR mgfi_buffer = {VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR, nullptr, memory_buffer_export.handle(),
                                        handle_type};
    VkMemoryGetFdInfoKHR mgfi_image = {VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR, nullptr, memory_image_export.handle(),
                                       handle_type};
    int fd_buffer;
    int fd_image;
    ASSERT_VK_SUCCESS(vkGetMemoryFdKHR(m_device->device(), &mgfi_buffer, &fd_buffer));
    ASSERT_VK_SUCCESS(vkGetMemoryFdKHR(m_device->device(), &mgfi_image, &fd_image));

    VkImportMemoryFdInfoKHR import_info_buffer = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR, nullptr, handle_type, fd_buffer};
    VkImportMemoryFdInfoKHR import_info_image = {VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR, nullptr, handle_type, fd_image};
#endif

    // Import memory
    VkMemoryRequirements buffer_import_reqs = buffer_import.memory_requirements();
    if (buffer_import_reqs.memoryTypeBits == 0) {
        printf("%s no suitable memory found, skipping test\n", kSkipPrefix);
        return;
    }
    alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer_import_reqs, mem_flags);
    alloc_info.pNext = &import_info_buffer;
    vk_testing::DeviceMemory memory_buffer_import;
    memory_buffer_import.init(*m_device, alloc_info);

    VkMemoryRequirements image_import_reqs = image_import.memory_requirements();
    if (image_import_reqs.memoryTypeBits == 0) {
        printf("%s no suitable memory found, skipping test\n", kSkipPrefix);
        return;
    }
    alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image_import_reqs, mem_flags);
    alloc_info.pNext = &import_info_image;
    vk_testing::DeviceMemory memory_image_import;
    memory_image_import.init(*m_device, alloc_info);
    m_errorMonitor->VerifyNotFound();

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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
        return;
    }

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        printf("%s Test requires (unsupported) extendedDynamicState, skipping\n", kSkipPrefix);
        return;
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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_3);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
        return;
    }

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        printf("%s Test requires (unsupported) extendedDynamicState, skipping\n", kSkipPrefix);
        return;
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineViewportStateCreateInfo-scissorCount-04136");
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

    commandBuffer.end();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicStateEnabledNoMultiview) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state VUs");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
        return;
    }

    auto extended_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        printf("%s Test requires (unsupported) extendedDynamicState, skipping\n", kSkipPrefix);
        return;
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

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    bool sri_extension = DeviceExtensionSupported(gpu(), nullptr, VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    if (sri_extension) {
        m_device_extension_names.push_back(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    }

    bool fdm_extension = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    if (fdm_extension) {
        m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    }

    if (!fdm_extension && !sri_extension) {
        printf("%s requires VK_NV_shading_rate_image or VK_EXT_fragment_density_map.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
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
    VkPhysicalDeviceFeatures2KHR query_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_query_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &query_features2);

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

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        GTEST_SKIP() << "Test not supported by MockICD, DevSim doesn't support Vulkan 1.1+";
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
        printf("%s Failed to compile shader\n", kSkipPrefix);
        return;
    }
    const VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kVUID_Core_Shader_InvalidExtension);
    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CmdCopyAccelerationStructureToMemoryKHR) {
    TEST_DESCRIPTION("Validate CmdCopyAccelerationStructureToMemoryKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    if (!InitFrameworkForRayTracingTest(this, true, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto ray_query_features = LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>(&ray_tracing_features);
    auto acc_struct_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&ray_query_features);
    features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&acc_struct_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (ray_query_features.rayQuery == VK_FALSE && ray_tracing_features.rayTracingPipeline == VK_FALSE) {
        printf("%s Both of the required features rayQuery and rayTracing are not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &acc_struct_features));

    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateAccelerationStructureKHR"));
    assert(vkCreateAccelerationStructureKHR != nullptr);

    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR =
        reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkDestroyAccelerationStructureKHR"));
    assert(vkDestroyAccelerationStructureKHR != nullptr);

    constexpr VkDeviceSize buffer_size = 4096;
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = buffer_size;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_ci);

    auto as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.pNext = NULL;
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    PFN_vkCmdCopyAccelerationStructureToMemoryKHR vkCmdCopyAccelerationStructureToMemoryKHR =
        reinterpret_cast<PFN_vkCmdCopyAccelerationStructureToMemoryKHR>(
            vk::GetDeviceProcAddr(device(), "vkCmdCopyAccelerationStructureToMemoryKHR"));
    assert(vkCmdCopyAccelerationStructureToMemoryKHR != nullptr);

    VkAccelerationStructureKHR as;
    vkCreateAccelerationStructureKHR(m_device->handle(), &as_create_info, nullptr, &as);

    constexpr intptr_t alignment_padding = 256 - 1;
    int8_t output[buffer_size + alignment_padding];
    VkDeviceOrHostAddressKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(((intptr_t)output + alignment_padding) & ~alignment_padding);
    auto copy_info = LvlInitStruct<VkCopyAccelerationStructureToMemoryInfoKHR>();
    copy_info.src = as;
    copy_info.dst = output_data;
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
    VkCommandBufferObj cb(m_device, m_commandPool);
    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureToMemoryKHR-None-03559");
    vkCmdCopyAccelerationStructureToMemoryKHR(cb.handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    cb.end();

    vkDestroyAccelerationStructureKHR(m_device->handle(), as, nullptr);
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
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
        return;
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
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
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
    m_errorMonitor->ExpectSuccess();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

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
    m_errorMonitor->ExpectSuccess();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    // if we only signal a binary semaphore we don't need a 'values' array
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = nullptr;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 0;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = nullptr;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &extra_binary;
    m_errorMonitor->ExpectSuccess();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
    vk::DestroySemaphore(m_device->device(), semaphore[0], nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore[1], nullptr);
    vk::DestroySemaphore(m_device->device(), extra_binary, nullptr);
}

TEST_F(VkLayerTest, ValidateExtendedDynamicState2Disabled) {
    TEST_DESCRIPTION("Validate VK_EXT_extended_dynamic_state2 VUs");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
        return;
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state2_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        printf("%s Test requires (unsupported) extendedDynamicState2, skipping\n", kSkipPrefix);
        return;
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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
        return;
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state2_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints) {
        printf("%s Test requires (unsupported) extendedDynamicState2LogicOp, skipping\n", kSkipPrefix);
        return;
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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
        return;
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state2_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!extended_dynamic_state2_features.extendedDynamicState2LogicOp) {
        printf("%s Test requires (unsupported) extendedDynamicState2LogicOp, skipping\n", kSkipPrefix);
        return;
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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
        return;
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state2_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!extended_dynamic_state2_features.extendedDynamicState2) {
        printf("%s Test requires (unsupported) extendedDynamicState2, skipping\n", kSkipPrefix);
        return;
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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
        return;
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state2_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints) {
        printf("%s Test requires (unsupported) extendedDynamicState2PatchControlPoints, skipping\n", kSkipPrefix);
        return;
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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
        return;
    }

    auto extended_dynamic_state2_features = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state2_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!extended_dynamic_state2_features.extendedDynamicState2LogicOp) {
        printf("%s Test requires (unsupported) extendedDynamicState2LogicOp, skipping\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, ValidateVertexInputDynamicStateDisabled) {
    TEST_DESCRIPTION("Validate VK_EXT_vertex_input_dynamic_state VUs when disabled");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
        return;
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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
        return;
    }

    auto vertex_input_dynamic_state_features = LvlInitStruct<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vertex_input_dynamic_state_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        printf("%s Test requires (unsupported) vertexInputDynamicState, skipping\n", kSkipPrefix);
        return;
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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_1);
    if (version < VK_API_VERSION_1_1) {
        printf("%s At least Vulkan version 1.1 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
        return;
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
        return;
    }

    auto vertex_attribute_divisor_features = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    auto vertex_input_dynamic_state_features =
        LvlInitStruct<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>(&vertex_attribute_divisor_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vertex_input_dynamic_state_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!vertex_attribute_divisor_features.vertexAttributeInstanceRateDivisor) {
        printf("%s Test requires (unsupported) vertexAttributeInstanceRateDivisor, skipping\n", kSkipPrefix);
        return;
    }
    if (!vertex_input_dynamic_state_features.vertexInputDynamicState) {
        printf("%s Test requires (unsupported) vertexInputDynamicState, skipping\n", kSkipPrefix);
        return;
    }

    auto vertex_attribute_divisor_properties = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&vertex_attribute_divisor_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &properties2);

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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    bool sync2 = false;
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2 = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>();
    synchronization2.synchronization2 = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>();
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        features2.pNext = &synchronization2;
        sync2 = true;
    }
    InitState(nullptr, &features2);
    sync2 &= (synchronization2.synchronization2 == VK_TRUE);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props.data());
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        printf("%s Device graphic queue has timestampValidBits of 0, skipping.\n", kSkipPrefix);
        return;
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
        printf("%s physical_device_group_count is 0, skipping test\n", kSkipPrefix);
        return;
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
        return;
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

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
        printf("%s physical_device_group_count is 0, skipping test\n", kSkipPrefix);
        return;
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
        printf("%s image format is not supported.\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-physicalDeviceCount-01421");
    VkImage test_image;
    vk::CreateImage(device(), &ici, nullptr, &test_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, QueueSubmitWaitingSameSemaphore) {
    TEST_DESCRIPTION("Submit to queue with waitSemaphore that another queue is already waiting on.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (m_device->graphics_queues().size() < 2) {
        printf("%s 2 graphics queues are needed.\n", kSkipPrefix);
        return;
    }

    VkSemaphoreCreateInfo sem_info = LvlInitStruct<VkSemaphoreCreateInfo>();

    vk_testing::Semaphore semaphore;
    semaphore.init(*m_device, sem_info);

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkSubmitInfo signalSubmitInfo = LvlInitStruct<VkSubmitInfo>();
    signalSubmitInfo.signalSemaphoreCount = 1;
    signalSubmitInfo.pSignalSemaphores = &semaphore.handle();

    VkSubmitInfo waitSubmitInfo = LvlInitStruct<VkSubmitInfo>();
    waitSubmitInfo.waitSemaphoreCount = 1;
    waitSubmitInfo.pWaitSemaphores = &semaphore.handle();
    waitSubmitInfo.pWaitDstStageMask = &stageFlags;

    VkQueue other = m_device->graphics_queues()[1]->handle();

    vk::QueueSubmit(m_device->m_queue, 1, &signalSubmitInfo, VK_NULL_HANDLE);
    vk::QueueSubmit(m_device->m_queue, 1, &waitSubmitInfo, VK_NULL_HANDLE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pWaitSemaphores-00068");
    vk::QueueSubmit(other, 1, &waitSubmitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
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
        printf("%s Required queue families not present (non-graphics capable required).\n", kSkipPrefix);
        return;
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
}

TEST_F(VkLayerTest, InvalidColorWriteEnableFeature) {
    TEST_DESCRIPTION("Invalid usage of vkCmdSetColorWriteEnableEXT with feature not enabled");

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME)) {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME);

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

    m_errorMonitor->ExpectSuccess();
    // mismatch of attachmentCount value is allowed for dynamic
    // see https://gitlab.khronos.org/vulkan/vulkan/-/issues/2868
    vkCmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 2, color_write_enable.data());
    m_errorMonitor->VerifyNotFound();

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
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

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
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&primitives_generated_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        printf("%s primitivesGeneratedQuery feature is not supported.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto transform_feedback_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

    auto fpCmdBeginQueryIndexedEXT =
        reinterpret_cast<PFN_vkCmdBeginQueryIndexedEXT>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT"));
    auto fpCmdEndQueryIndexedEXT =
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

    fpCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0, 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06694");
    fpCmdEndQueryIndexedEXT(m_commandBuffer->handle(), tf_query_pool.handle(), 0,
                            transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-06690");
    fpCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), pg_query_pool.handle(), 0, 0,
                              transform_feedback_properties.maxTransformFeedbackStreams);
    m_errorMonitor->VerifyFound();

    fpCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQueryIndexedEXT-queryType-06695");
    fpCmdEndQueryIndexedEXT(m_commandBuffer->handle(), query_pool.handle(), 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Features12AndppEnabledExtensionNames) {
    TEST_DESCRIPTION("Test VkPhysicalDeviceVulkan12Features and illegal extension in ppEnabledExtensionNames");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping tests\n", kSkipPrefix, VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

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
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_properties =
        LvlInitStruct<VkPhysicalDeviceDiscardRectanglePropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>();
    phys_dev_props_2.pNext = &discard_rectangle_properties;
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

    auto fpCmdSetDiscardRectangleEXT =
        (PFN_vkCmdSetDiscardRectangleEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDiscardRectangleEXT");

    if (discard_rectangle_properties.maxDiscardRectangles == 0) {
        printf("%s Discard rectangles are not supported, skipping test\n", kSkipPrefix);
        return;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_props =
        LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    VkPhysicalDeviceProperties2 phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_props);

    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
    if (transform_feedback_props.transformFeedbackQueries) {
        printf("%s Transform feedback queries are supported, skipping test\n", kSkipPrefix);
        return;
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

    auto vkCmdSetDiscardRectangleEXT =
        (PFN_vkCmdSetDiscardRectangleEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdSetDiscardRectangleEXT");

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

TEST_F(VkLayerTest, ValidateBeginQueryQueryPoolType) {
    TEST_DESCRIPTION("Test CmdBeginQuery with invalid queryPool queryType");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1.";
    }

    bool khr_acceleration_structure = IsExtensionsEnabled(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    bool nv_ray_tracing = IsExtensionsEnabled(VK_NV_RAY_TRACING_EXTENSION_NAME);
    bool ext_transform_feedback = IsExtensionsEnabled(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);

    if (!khr_acceleration_structure && !nv_ray_tracing) {
        GTEST_SKIP() << "Extensions " << VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME << " and " << VK_NV_RAY_TRACING_EXTENSION_NAME
                     << " are not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    PFN_vkCmdBeginQueryIndexedEXT vkCmdBeginQueryIndexedEXT =
        (PFN_vkCmdBeginQueryIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginQueryIndexedEXT");

    VkQueryPoolCreateInfo query_pool_ci = LvlInitStruct<VkQueryPoolCreateInfo>();
    query_pool_ci.queryCount = 1;

    if (khr_acceleration_structure) {
        {
            query_pool_ci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
            VkQueryPool query_pool;
            vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

            m_commandBuffer->begin();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-04728");
            vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
            m_errorMonitor->VerifyFound();

            if (ext_transform_feedback) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728");
                vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool, 0, 0, 0);
                m_errorMonitor->VerifyFound();
            }
            m_commandBuffer->end();
        }

        {
            query_pool_ci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;
            VkQueryPool query_pool;
            vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

            m_commandBuffer->begin();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-04728");
            vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
            m_errorMonitor->VerifyFound();

            if (ext_transform_feedback) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-04728");
                vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool, 0, 0, 0);
                m_errorMonitor->VerifyFound();
            }
            m_commandBuffer->end();
        }
    }
    if (nv_ray_tracing) {
        query_pool_ci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV;
        VkQueryPool query_pool;
        vk::CreateQueryPool(device(), &query_pool_ci, nullptr, &query_pool);

        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-04729");
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyFound();

        if (ext_transform_feedback) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQueryIndexedEXT-queryType-04729");
            vkCmdBeginQueryIndexedEXT(m_commandBuffer->handle(), query_pool, 0, 0, 0);
            m_errorMonitor->VerifyFound();
        }
        m_commandBuffer->end();
    }
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
        printf("%s Required query not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    const size_t out_data_size = 16;
    uint8_t data[out_data_size];
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-04810");
    vk::GetQueryPoolResults(m_device->device(), query_pool, 0, 1, out_data_size, &data, sizeof(uint32_t), 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerTest, CopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyQueryPoolResults with unsupported query type");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto vkCmdCopyAccelerationStructureKHR = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(device(), "vkCmdCopyAccelerationStructureKHR"));
    assert(vkCmdCopyAccelerationStructureKHR != nullptr);

    auto vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");
    assert(vkGetBufferDeviceAddressKHR != nullptr);

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    VkBufferObj buffer_no_mem;
    buffer_no_mem.init_no_mem(*m_device, buffer_ci);

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_ci);

    VkBufferDeviceAddressInfo device_address_info = LvlInitStruct<VkBufferDeviceAddressInfo>();
    device_address_info.buffer = buffer_no_mem.handle();
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &device_address_info);
    if (device_address == 0) {
        printf("%s Failed to get device address, skipping test.\n", kSkipPrefix);
        return;
    }

    auto as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer_no_mem.handle();
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = device_address;

    vk_testing::AccelerationStructureKHR invalid_as(*m_device, as_create_info);

    device_address_info.buffer = buffer.handle();
    as_create_info.buffer = buffer.handle();
    vk_testing::AccelerationStructureKHR valid_as(*m_device, as_create_info);

    auto copy_info = LvlInitStruct<VkCopyAccelerationStructureInfoKHR>();
    copy_info.src = invalid_as.handle();
    copy_info.dst = valid_as.handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    vkCmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = valid_as.handle();
    copy_info.dst = invalid_as.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    vkCmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
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

TEST_F(VkLayerTest, ValidateBeginRenderingDisabled) {
    TEST_DESCRIPTION("Validate VK_KHR_dynamic_rendering VUs when disabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    bool vulkan_13 = (DeviceValidationVersion() >= VK_API_VERSION_1_3);
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginRendering-dynamicRendering-06446");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    if (vulkan_13) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginRendering-dynamicRendering-06446");
        vk::CmdBeginRendering(m_commandBuffer->handle(), &begin_rendering_info);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRendering-None-06161");
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdEndRendering(m_commandBuffer->handle());
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
    }

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CmdCopyUnboundAccelerationStructure) {
    TEST_DESCRIPTION("Test CmdCopyAccelerationStructureKHR with buffers not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&accel_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (accel_features.accelerationStructureHostCommands == VK_FALSE) {
        printf("%s accelerationStructureHostCommands feature is not supported.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCmdCopyAccelerationStructureKHR = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(device(), "vkCmdCopyAccelerationStructureKHR"));
    assert(vkCmdCopyAccelerationStructureKHR != nullptr);
    auto vkCopyAccelerationStructureKHR =
        reinterpret_cast<PFN_vkCopyAccelerationStructureKHR>(vk::GetDeviceProcAddr(device(), "vkCopyAccelerationStructureKHR"));
    assert(vkCopyAccelerationStructureKHR != nullptr);

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    VkBufferObj buffer_no_mem;
    buffer_no_mem.init_no_mem(*m_device, buffer_ci);

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_ci);

    VkBufferObj host_visible_buffer;
    host_visible_buffer.init(*m_device, buffer_ci.size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffer_ci.usage);

    auto as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer_no_mem.handle();
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    vk_testing::AccelerationStructureKHR as_no_mem(*m_device, as_create_info);
    as_create_info.buffer = buffer.handle();
    vk_testing::AccelerationStructureKHR as_mem(*m_device, as_create_info);
    as_create_info.buffer = host_visible_buffer.handle();
    vk_testing::AccelerationStructureKHR as_host_mem(*m_device, as_create_info);

    auto copy_info = LvlInitStruct<VkCopyAccelerationStructureInfoKHR>();
    copy_info.src = as_no_mem.handle();
    copy_info.dst = as_mem.handle();
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03718");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03737");
    vkCmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    copy_info.src = as_mem.handle();
    copy_info.dst = as_no_mem.handle();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyAccelerationStructureInfoKHR-buffer-03719");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyAccelerationStructureKHR-buffer-03738");
    vkCmdCopyAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    copy_info.src = as_mem.handle();
    copy_info.dst = as_host_mem.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyAccelerationStructureKHR-buffer-03727");
    vkCopyAccelerationStructureKHR(m_device->handle(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();

    copy_info.src = as_host_mem.handle();
    copy_info.dst = as_mem.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyAccelerationStructureKHR-buffer-03728");
    vkCopyAccelerationStructureKHR(m_device->handle(), VK_NULL_HANDLE, &copy_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestCmdCopyMemoryToAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate CmdCopyMemoryToAccelerationStructureKHR with dst buffer not bound to memory");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(&accel_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&bda_features);
    if (!InitFrameworkForRayTracingTest(this, true, false, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");
    assert(vkGetBufferDeviceAddressKHR != nullptr);
    PFN_vkCmdCopyMemoryToAccelerationStructureKHR vkCmdCopyMemoryToAccelerationStructureKHR =
        (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)vk::GetInstanceProcAddr(instance(),
                                                                               "vkCmdCopyMemoryToAccelerationStructureKHR");
    assert(vkCmdCopyMemoryToAccelerationStructureKHR != nullptr);
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateAccelerationStructureKHR"));
    assert(vkCreateAccelerationStructureKHR != nullptr);

    auto vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(device(), "vkDestroyAccelerationStructureKHR"));
    assert(vkDestroyAccelerationStructureKHR != nullptr);

    auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkBufferObj src_buffer;
    src_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, &alloc_flags);

    VkBufferCreateInfo buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 1024;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    VkBufferObj dst_buffer;
    dst_buffer.init_no_mem(*m_device, buffer_ci);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = dst_buffer.handle();
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    VkAccelerationStructureKHR as;
    vkCreateAccelerationStructureKHR(m_device->handle(), &as_create_info, nullptr, &as);

    VkBufferDeviceAddressInfo device_address_info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, src_buffer.handle()};
    VkDeviceAddress device_address = vkGetBufferDeviceAddressKHR(m_device->handle(), &device_address_info);

    VkCopyMemoryToAccelerationStructureInfoKHR copy_info = LvlInitStruct<VkCopyMemoryToAccelerationStructureInfoKHR>();
    copy_info.src.deviceAddress = device_address;
    copy_info.dst = as;
    copy_info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyMemoryToAccelerationStructureKHR-buffer-03745");
    m_commandBuffer->begin();
    vkCmdCopyMemoryToAccelerationStructureKHR(m_commandBuffer->handle(), &copy_info);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
    vkDestroyAccelerationStructureKHR(m_device->handle(), as, nullptr);
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
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features_1_1);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
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

TEST_F(VkLayerTest, BuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate buffers used in vkBuildAccelerationStructureKHR");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto acc_structure_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&acc_structure_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (acc_structure_features.accelerationStructureHostCommands == VK_FALSE) {
        printf("%s accelerationStructureHostCommands feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_errorMonitor->ExpectSuccess();

    PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR =
        (PFN_vkBuildAccelerationStructuresKHR)vk::GetInstanceProcAddr(instance(), "vkBuildAccelerationStructuresKHR");
    assert(vkBuildAccelerationStructuresKHR);

    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkBufferObj host_buffer;
    host_buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj bot_level_as(*m_device, as_create_info);
    as_create_info.buffer = host_buffer.handle();
    VkAccelerationStructurekhrObj host_bot_level_as(*m_device, as_create_info);

    auto build_info_khr = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info_khr.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info_khr.dstAccelerationStructure = bot_level_as.handle();
    build_info_khr.geometryCount = 0;
    build_info_khr.pGeometries = nullptr;
    build_info_khr.ppGeometries = nullptr;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info;
    build_range_info.firstVertex = 0;
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 3;
    build_range_info.transformOffset = 0;

    VkAccelerationStructureBuildRangeInfoKHR *p_build_range_info = &build_range_info;

    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    vkBuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &build_info_khr, &p_build_range_info);
    m_errorMonitor->VerifyFound();

    build_info_khr.srcAccelerationStructure = bot_level_as.handle();
    build_info_khr.dstAccelerationStructure = host_bot_level_as.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBuildAccelerationStructuresKHR-pInfos-03723");
    vkBuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &build_info_khr, &p_build_range_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineStatisticsQuery) {
    TEST_DESCRIPTION("Test unsupported pipeline statistics queries");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t graphics_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT);
    uint32_t compute_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (graphics_queue_family_index == std::numeric_limits<uint32_t>::max() &&
        compute_queue_family_index == std::numeric_limits<uint32_t>::max()) {
        printf("%s required queue families not found, skipping test.\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, TestWriteAccelerationStructureMemory) {
    TEST_DESCRIPTION("Test memory in vkWriteAccelerationStructuresPropertiesKHR is host visible");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto as_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&as_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (as_features.accelerationStructure == VK_FALSE) {
        printf("%s accelerationStructure feature is not supported.\n", kSkipPrefix);
        return;
    }
    if (as_features.accelerationStructureHostCommands == VK_FALSE) {
        printf("%s accelerationStructure feature is not supported.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkWriteAccelerationStructuresPropertiesKHR = reinterpret_cast<PFN_vkWriteAccelerationStructuresPropertiesKHR>(
        vk::GetInstanceProcAddr(instance(), "vkWriteAccelerationStructuresPropertiesKHR"));
    assert(vkWriteAccelerationStructuresPropertiesKHR);
    PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR =
        (PFN_vkBuildAccelerationStructuresKHR)vk::GetInstanceProcAddr(instance(), "vkBuildAccelerationStructuresKHR");
    assert(vkBuildAccelerationStructuresKHR);

    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer.handle();
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;

    vk_testing::AccelerationStructureKHR acceleration_structure(*m_device, as_create_info);

    auto build_info_khr = LvlInitStruct<VkAccelerationStructureBuildGeometryInfoKHR>();
    build_info_khr.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_info_khr.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info_khr.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
    build_info_khr.srcAccelerationStructure = VK_NULL_HANDLE;
    build_info_khr.dstAccelerationStructure = acceleration_structure.handle();
    build_info_khr.geometryCount = 0;
    build_info_khr.pGeometries = nullptr;
    build_info_khr.ppGeometries = nullptr;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info;
    build_range_info.firstVertex = 0;
    build_range_info.primitiveCount = 1;
    build_range_info.primitiveOffset = 3;
    build_range_info.transformOffset = 0;

    VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfos = &build_range_info;
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBuildAccelerationStructuresKHR-pInfos-03722");
    vkBuildAccelerationStructuresKHR(device(), VK_NULL_HANDLE, 1, &build_info_khr, &pBuildRangeInfos);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->VerifyNotFound();

    uint32_t data[4096];

    VkAccelerationStructureKHR acceleration_structure_handle = acceleration_structure.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkWriteAccelerationStructuresPropertiesKHR-buffer-03733");
    vkWriteAccelerationStructuresPropertiesKHR(device(), 1, &acceleration_structure_handle,
                                               VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, 4096, &data, 4096);
    m_errorMonitor->VerifyFound();
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

    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto color_write_enable_features = LvlInitStruct<VkPhysicalDeviceColorWriteEnableFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&color_write_enable_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (color_write_enable_features.colorWriteEnable == VK_FALSE) {
        printf("%s colorWriteEnable feature is not supported.\n", kSkipPrefix);
        return;
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-attachmentCount-06667");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBool32 color_write_enable[] = {VK_TRUE, VK_FALSE};
    vkCmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 1, color_write_enable);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-attachmentCount-06667");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vkCmdSetColorWriteEnableEXT(m_commandBuffer->handle(), 2, color_write_enable);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
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
        printf("%s Multiple queues from same queue family are required to run this test. .\n", kSkipPrefix);
        return;
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
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&primitives_generated_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        printf("%s primitivesGeneratedQuery feature is not supported.\n", kSkipPrefix);
        return;
    }
    primitives_generated_features.primitivesGeneratedQueryWithNonZeroStreams = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    auto transform_feedback_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&transform_feedback_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

    uint32_t compute_queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (compute_queue_family_index == std::numeric_limits<uint32_t>::max()) {
        printf("%s required queue family not found, skipping test.\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, TestCopyMemoryToAsBuffer) {
    TEST_DESCRIPTION("Test invalid buffer used in vkCopyMemoryToAccelerationStructureKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto accel_features = LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&accel_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (accel_features.accelerationStructureHostCommands == VK_FALSE) {
        printf("%s accelerationStructureHostCommands feature is not supported.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCopyMemoryToAccelerationStructureKHR = reinterpret_cast<PFN_vkCopyMemoryToAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(device(), "vkCopyMemoryToAccelerationStructureKHR"));
    assert(vkCopyMemoryToAccelerationStructureKHR != nullptr);

    VkBufferObj buffer;
    buffer.init(*m_device, 4096, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    VkAccelerationStructureCreateInfoKHR as_create_info = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
    as_create_info.buffer = buffer.handle();
    as_create_info.createFlags = 0;
    as_create_info.offset = 0;
    as_create_info.size = 0;
    as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    as_create_info.deviceAddress = 0;
    VkAccelerationStructurekhrObj bot_level_as(*m_device, as_create_info);

    uint8_t output[4096];
    VkDeviceOrHostAddressConstKHR output_data;
    output_data.hostAddress = reinterpret_cast<void *>(output);

    auto info = LvlInitStruct<VkCopyMemoryToAccelerationStructureInfoKHR>();
    info.dst = bot_level_as.handle();
    info.src = output_data;
    info.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCopyMemoryToAccelerationStructureKHR-buffer-03730");
    vkCopyMemoryToAccelerationStructureKHR(device(), VK_NULL_HANDLE, &info);
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
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&primitives_generated_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (primitives_generated_features.primitivesGeneratedQuery == VK_FALSE) {
        printf("%s primitivesGeneratedQuery feature is not supported.\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, RayTracingPipelineDeferredOp) {
    TEST_DESCRIPTION(
        "Test that objects created with deferred operations are recorded once the operation has successfully completed.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    auto ray_tracing_features = LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    if (!InitFrameworkForRayTracingTest(this, true, false, &features2)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }

    // Needed for Ray Tracing
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!ray_tracing_features.rayTracingPipeline) {
        printf("%s Feature rayTracing is not supported.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj empty_pipeline_layout(m_device, {});

    const char *empty_shader = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : require
        void main() {}
    )glsl";

    VkShaderObj rgen_shader(this, empty_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    VkShaderObj chit_shader(this, empty_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);

    auto vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vk::GetInstanceProcAddr(instance(), "vkCreateRayTracingPipelinesKHR"));
    ASSERT_TRUE(vkCreateRayTracingPipelinesKHR != nullptr);

    auto vkCreateDeferredOperationKHR =
        reinterpret_cast<PFN_vkCreateDeferredOperationKHR>(vk::GetInstanceProcAddr(instance(), "vkCreateDeferredOperationKHR"));
    ASSERT_TRUE(vkCreateDeferredOperationKHR != nullptr);

    auto vkDeferredOperationJoinKHR =
        reinterpret_cast<PFN_vkDeferredOperationJoinKHR>(vk::GetInstanceProcAddr(instance(), "vkDeferredOperationJoinKHR"));
    ASSERT_TRUE(vkDeferredOperationJoinKHR != nullptr);

    auto vkGetDeferredOperationResultKHR = reinterpret_cast<PFN_vkGetDeferredOperationResultKHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetDeferredOperationResultKHR"));
    ASSERT_TRUE(vkDeferredOperationJoinKHR != nullptr);

    PFN_vkDestroyDeferredOperationKHR vkDestroyDeferredOperationKHR =
        reinterpret_cast<PFN_vkDestroyDeferredOperationKHR>(vk::GetInstanceProcAddr(instance(), "vkDestroyDeferredOperationKHR"));
    ASSERT_TRUE(vkDestroyDeferredOperationKHR != nullptr);

    const VkPipelineLayoutObj pipeline_layout(m_device, {});

    VkPipelineShaderStageCreateInfo stage_create_info = LvlInitStruct<VkPipelineShaderStageCreateInfo>();
    stage_create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    stage_create_info.module = chit_shader.handle();
    stage_create_info.pName = "main";

    VkRayTracingShaderGroupCreateInfoKHR group_create_info = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group_create_info.generalShader = VK_SHADER_UNUSED_KHR;
    group_create_info.closestHitShader = 0;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineInterfaceCreateInfoKHR interface_ci = LvlInitStruct<VkRayTracingPipelineInterfaceCreateInfoKHR>();
    interface_ci.maxPipelineRayHitAttributeSize = 4;
    interface_ci.maxPipelineRayPayloadSize = 4;

    VkRayTracingPipelineCreateInfoKHR library_pipeline = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    library_pipeline.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    library_pipeline.stageCount = 1;
    library_pipeline.pStages = &stage_create_info;
    library_pipeline.groupCount = 1;
    library_pipeline.pGroups = &group_create_info;
    library_pipeline.layout = pipeline_layout.handle();
    library_pipeline.pLibraryInterface = &interface_ci;

    VkPipeline library = VK_NULL_HANDLE;
    vkCreateRayTracingPipelinesKHR(m_device->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &library_pipeline, nullptr, &library);

    VkPipelineLibraryCreateInfoKHR library_info_one = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>();
    library_info_one.libraryCount = 1;
    library_info_one.pLibraries = &library;

    stage_create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    stage_create_info.module = rgen_shader.handle();
    stage_create_info.pName = "main";

    group_create_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group_create_info.generalShader = 0;
    group_create_info.closestHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.anyHitShader = VK_SHADER_UNUSED_KHR;
    group_create_info.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingPipelineCreateInfoKHR pipeline_ci = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    pipeline_ci.pLibraryInfo = &library_info_one;
    pipeline_ci.stageCount = 1;
    pipeline_ci.pStages = &stage_create_info;
    pipeline_ci.groupCount = 1;
    pipeline_ci.pGroups = &group_create_info;
    pipeline_ci.layout = empty_pipeline_layout.handle();
    pipeline_ci.pLibraryInterface = &interface_ci;

    VkDeferredOperationKHR deferredOperation = VK_NULL_HANDLE;
    vkCreateDeferredOperationKHR(m_device->handle(), 0, &deferredOperation);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult result =
        vkCreateRayTracingPipelinesKHR(m_device->handle(), deferredOperation, VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
    ASSERT_EQ(result, VK_OPERATION_DEFERRED_KHR);

    result = vkDeferredOperationJoinKHR(this->m_device->handle(), deferredOperation);
    ASSERT_EQ(result, VK_SUCCESS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-parameter");
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    m_errorMonitor->VerifyFound();

    result = vkGetDeferredOperationResultKHR(m_device->handle(), deferredOperation);
    ASSERT_EQ(result, VK_SUCCESS);

    m_errorMonitor->ExpectSuccess();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
    m_commandBuffer->end();

    vk::DestroyPipeline(m_device->handle(), pipeline, nullptr);
    vkDestroyDeferredOperationKHR(m_device->handle(), deferredOperation, nullptr);
    vk::DestroyPipeline(m_device->handle(), library, nullptr);

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, TestMultipleQueuesWaitingOnSemaphore) {
    TEST_DESCRIPTION("Test submitting two command buffers that wait on the same semaphore to different queues.");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (m_device->graphics_queues().size() < 2) {
        printf("%s 2 graphics queues are needed.\n", kSkipPrefix);
        return;
    }

    auto sem_info = LvlInitStruct<VkSemaphoreCreateInfo>();

    vk_testing::Semaphore semaphore;
    semaphore.init(*m_device, sem_info);

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    auto signal_submit_info = LvlInitStruct<VkSubmitInfo>();
    signal_submit_info.signalSemaphoreCount = 1;
    signal_submit_info.pSignalSemaphores = &semaphore.handle();

    auto wait_submit_info = LvlInitStruct<VkSubmitInfo>();
    wait_submit_info.waitSemaphoreCount = 1;
    wait_submit_info.pWaitSemaphores = &semaphore.handle();
    wait_submit_info.pWaitDstStageMask = &stageFlags;

    auto wait_and_signal_submit_info = LvlInitStruct<VkSubmitInfo>();
    wait_and_signal_submit_info.signalSemaphoreCount = 1;
    wait_and_signal_submit_info.pSignalSemaphores = &semaphore.handle();
    wait_and_signal_submit_info.waitSemaphoreCount = 1;
    wait_and_signal_submit_info.pWaitSemaphores = &semaphore.handle();
    wait_and_signal_submit_info.pWaitDstStageMask = &stageFlags;

    m_errorMonitor->ExpectSuccess();

    VkQueue other = m_device->graphics_queues()[1]->handle();

    vk::QueueSubmit(m_device->m_queue, 1, &signal_submit_info, VK_NULL_HANDLE);
    vk::QueueSubmit(m_device->m_queue, 1, &wait_and_signal_submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyNotFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pWaitSemaphores-00068");
    vk::QueueSubmit(other, 1, &wait_submit_info, VK_NULL_HANDLE);
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
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&multiview_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s multiview feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_errorMonitor->ExpectSuccess();

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
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderPass-00904");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    rpbi.renderPass = render_pass3.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderPass-00904");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}
