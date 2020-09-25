/*
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (c) 2015-2020 Google, Inc.
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
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

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

    auto phys_dev_props_2 = lvl_init_struct<VkPhysicalDeviceProperties2>();
    auto bad_version_1_1_struct = lvl_init_struct<VkPhysicalDeviceVulkan12Properties>();
    phys_dev_props_2.pNext = &bad_version_1_1_struct;

    // VkPhysDevVulkan12Props was introduced in 1.2, so try adding it to a 1.1 pNext chain
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
    m_errorMonitor->VerifyFound();

    // 1.1 context, VK_KHR_depth_stencil_resolve is NOT enabled, but using its struct is valid
    if (DeviceExtensionSupported(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)) {
        auto unenabled_device_ext_struct = lvl_init_struct<VkPhysicalDeviceDepthStencilResolveProperties>();
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

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping.\n", kSkipPrefix);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_PRIVATE_DATA_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_PRIVATE_DATA_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_PRIVATE_DATA_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkDestroyPrivateDataSlotEXT pfn_vkDestroyPrivateDataSlotEXT =
        (PFN_vkDestroyPrivateDataSlotEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkDestroyPrivateDataSlotEXT");
    PFN_vkCreatePrivateDataSlotEXT pfn_vkCreatePrivateDataSlotEXT =
        (PFN_vkCreatePrivateDataSlotEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCreatePrivateDataSlotEXT");
    PFN_vkGetPrivateDataEXT pfn_vkGetPrivateDataEXT =
        (PFN_vkGetPrivateDataEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkGetPrivateDataEXT");
    PFN_vkSetPrivateDataEXT pfn_vkSetPrivateDataEXT =
        (PFN_vkSetPrivateDataEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkSetPrivateDataEXT");

    VkPrivateDataSlotEXT data_slot;
    VkPrivateDataSlotCreateInfoEXT data_create_info;
    data_create_info.sType = VK_STRUCTURE_TYPE_PRIVATE_DATA_SLOT_CREATE_INFO_EXT;
    data_create_info.pNext = NULL;
    data_create_info.flags = 0;
    VkResult err = pfn_vkCreatePrivateDataSlotEXT(m_device->handle(), &data_create_info, NULL, &data_slot);
    if (err != VK_SUCCESS) {
        printf("%s Failed to create private data slot, VkResult %d.\n", kSkipPrefix, err);
    }

    VkSampler sampler;
    VkSamplerCreateInfo sampler_info;
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.pNext = NULL;
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
        printf("%s Failed to set private data. VkResult = %d", kSkipPrefix, err);
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
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);
    VkBufferView buffer_view;
    VkBufferViewCreateInfo bvci = {};
    bvci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    bvci.pNext = &custom_struct;  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;

    m_errorMonitor->ExpectSuccess(kErrorBit);
    vk::CreateBufferView(m_device->device(), &bvci, NULL, &buffer_view);
    m_errorMonitor->VerifyNotFound();

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
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);
    VkBufferView buffer_view;
    VkBufferViewCreateInfo bvci = {};
    bvci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    bvci.pNext = &custom_struct_b;  // Add custom struct through pNext
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    auto msg_limit = DuplicateMsgLimit(3);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, msg_limit.pnext));
    ASSERT_NO_FATAL_FAILURE(InitState());
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    // Create an invalid pNext structure to trigger the stateless validation warning
    VkBaseOutStructure bogus_struct{};
    bogus_struct.sType = static_cast<VkStructureType>(0x33333333);
    auto properties2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&bogus_struct);

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

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE2_EXTENSION_NAME);
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

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE2_EXTENSION_NAME);
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

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE2_EXTENSION_NAME);
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

    auto callback_create_info = lvl_init_struct<VkDebugUtilsMessengerCreateInfoEXT>();
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
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
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
        (DeviceExtensionEnabled(VK_KHR_MAINTENANCE2_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));

    if (!((m_device->format_properties(VK_FORMAT_R8_UINT).optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) &&
          (ycbcr_support ^ maintenance2_support))) {
        printf("%s Device does not support format and extensions required, skipping test case", kSkipPrefix);
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

    VkImageViewCreateInfo imgViewInfo = {};
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
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
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj tcs(m_device, bindStateTscShaderText, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, this);
    VkShaderObj tes(m_device, bindStateTeshaderText, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
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
    auto indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    // Set one of the features values to an invalid boolean value
    indexing_features.descriptorBindingUniformBufferUpdateAfterBind = 800;

    uint32_t queue_node_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, NULL);
    VkQueueFamilyProperties *queue_props = new VkQueueFamilyProperties[queue_node_count];
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, queue_props);
    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];
    VkDeviceCreateInfo dev_info = {};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = NULL;
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();
    dev_info.pNext = &features2;
    VkDevice dev;
    m_errorMonitor->SetDesiredFailureMsg(kWarningBit, "is neither VK_TRUE nor VK_FALSE");
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
    VkEvent event_handle = VK_NULL_HANDLE;
    VkEventCreateInfo event_info = {};
    event_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    event_info.flags = 1;
    vk::CreateEvent(device(), &event_info, NULL, &event_handle);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DebugMarkerNameTest) {
    TEST_DESCRIPTION("Ensure debug marker object names are printed in debug report output");

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

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.size = 1;

    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer);

    VkMemoryRequirements memRequirements;
    vk::GetBufferMemoryRequirements(device(), buffer, &memRequirements);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memRequirements.size;
    memory_allocate_info.memoryTypeIndex = 0;

    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory_1);
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory_2);

    VkDebugMarkerObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
    name_info.pNext = nullptr;
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
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_1);
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_2);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = commandpool_1;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.object = (uint64_t)commandBuffer;
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
    name_info.pObjectName = commandBuffer_name.c_str();
    fpvkDebugMarkerSetObjectNameEXT(device(), &name_info);

    VkCommandBufferBeginInfo cb_begin_Info = {};
    cb_begin_Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
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

    auto callback_create_info = lvl_init_struct<VkDebugUtilsMessengerCreateInfoEXT>();
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

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.size = 1;

    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer);

    VkMemoryRequirements memRequirements;
    vk::GetBufferMemoryRequirements(device(), buffer, &memRequirements);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memRequirements.size;
    memory_allocate_info.memoryTypeIndex = 0;

    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory_1);
    vk::AllocateMemory(device(), &memory_allocate_info, nullptr, &memory_2);

    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.pNext = nullptr;
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
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_1);
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_2);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = commandpool_1;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.objectHandle = (uint64_t)commandBuffer;
    name_info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    name_info.pObjectName = commandBuffer_name.c_str();
    fpvkSetDebugUtilsObjectNameEXT(device(), &name_info);

    VkCommandBufferBeginInfo cb_begin_Info = {};
    cb_begin_Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cb_begin_Info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk::BeginCommandBuffer(commandBuffer, &cb_begin_Info);

    const VkRect2D scissor = {{-1, 0}, {16, 16}};
    const VkRect2D scissors[] = {scissor, scissor};

    auto command_label = lvl_init_struct<VkDebugUtilsLabelEXT>();
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(Init());

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), "value of pCreateInfo->pNext must be NULL");
    // Set VkMemoryAllocateInfo::pNext to a non-NULL value, when pNext must be NULL.
    // Need to pick a function that has no allowed pNext structure types.
    // Expected to trigger an error with parameter_validation::validate_struct_pnext
    VkEvent event = VK_NULL_HANDLE;
    VkEventCreateInfo event_alloc_info = {};
    // Zero-initialization will provide the correct sType
    VkApplicationInfo app_info = {};
    event_alloc_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    event_alloc_info.pNext = &app_info;
    vk::CreateEvent(device(), &event_alloc_info, NULL, &event);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), " chain includes a structure with unexpected VkStructureType ");
    // Set VkMemoryAllocateInfo::pNext to a non-NULL value, but use
    // a function that has allowed pNext structure types and specify
    // a structure type that is not allowed.
    // Expected to trigger an error with parameter_validation::validate_struct_pnext
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkMemoryAllocateInfo memory_alloc_info = {};
    memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_alloc_info.pNext = &app_info;
    vk::AllocateMemory(device(), &memory_alloc_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg((kErrorBit | kWarningBit), " chain includes a structure with unexpected VkStructureType ");
    // Same concept as above, but unlike vkAllocateMemory where VkMemoryAllocateInfo is a const
    // in vkGetPhysicalDeviceProperties2, VkPhysicalDeviceProperties2 is not a const
    VkPhysicalDeviceProperties2 physical_device_properties2 = {};
    physical_device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physical_device_properties2.pNext = &app_info;

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
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    // `stage_flags` is set to a value which, currently, is not a defined stage flag
    // `VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM` works well for this
    VkPipelineStageFlags stage_flags = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    // `waitSemaphoreCount` *must* be greater than 0 to perform this check
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->ClearAllBuffers(m_renderTargets, m_clear_color, nullptr, m_depth_clear_color, m_stencil_clear_color);
    m_commandBuffer->end();

    testFence.init(*m_device, fenceInfo);

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
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

    // Workaround for overzealous layers checking even the guaranteed 0th queue family
    const auto q_props = vk_testing::PhysicalDevice(gpu()).queue_properties();
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = {};
    queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = {};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkDevice leaky_device;
    ASSERT_VK_SUCCESS(vk::CreateDevice(gpu(), &device_ci, nullptr, &leaky_device));

    const VkFenceCreateInfo fence_ci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
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
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];

    VkDeviceCreateInfo device_create_info = {};
    auto features = m_device->phy().features();
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
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
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &esci, 0};

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
    VkPhysicalDeviceExternalFenceInfoKHR efi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO_KHR, nullptr, handle_type};
    VkExternalFencePropertiesKHR efp = {VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES_KHR, nullptr};
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
    VkEventCreateInfo evci = {};
    evci.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    VkResult result = vk::CreateEvent(m_device->device(), &evci, NULL, &event);
    ASSERT_VK_SUCCESS(result);

    m_commandBuffer->begin();
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkEvent");
    // Destroy event dependency prior to submit to cause ERROR
    vk::DestroyEvent(m_device->device(), event, NULL);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdBufferQueryPoolDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a query pool dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo qpci{};
    qpci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
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

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DeviceFeature2AndVertexAttributeDivisorExtensionUnenabled) {
    TEST_DESCRIPTION(
        "Test unenabled VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME & "
        "VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME.");

    VkPhysicalDeviceFeatures2 pd_features2 = {};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = nullptr;

    ASSERT_NO_FATAL_FAILURE(Init());
    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &pd_features2;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    VkDevice testDevice;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pNext-pNext");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = {};
    vadf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT;
    vadf.pNext = nullptr;
    device_create_info.pNext = &vadf;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VK_EXT_vertex_attribute_divisor must be enabled when it creates a device");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Features12AndpNext) {
    TEST_DESCRIPTION("Test VkPhysicalDeviceVulkan12Features and illegal struct in pNext");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        printf("%s Vulkan12Struct requires Vulkan 1.2+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME) ||
        !DeviceExtensionSupported(gpu(), nullptr, VK_KHR_8BIT_STORAGE_EXTENSION_NAME) ||
        !DeviceExtensionSupported(gpu(), nullptr, VK_KHR_16BIT_STORAGE_EXTENSION_NAME)) {
        printf("%s Storage Extension(s) not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    VkPhysicalDevice16BitStorageFeatures sixteen_bit = {};
    sixteen_bit.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
    sixteen_bit.storageBuffer16BitAccess = true;
    VkPhysicalDeviceVulkan11Features features11 = {};
    features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features11.pNext = &sixteen_bit;
    features11.storageBuffer16BitAccess = true;

    VkPhysicalDevice8BitStorageFeatures eight_bit = {};
    eight_bit.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES;
    eight_bit.pNext = &features11;
    eight_bit.storageBuffer8BitAccess = true;
    VkPhysicalDeviceVulkan12Features features12 = {};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.pNext = &eight_bit;
    features12.storageBuffer8BitAccess = true;

    vk_testing::PhysicalDevice physical_device(gpu());
    vk_testing::QueueCreateInfoArray queue_info(physical_device.queue_properties());
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &features12;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    VkDevice testDevice;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pNext-02829");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pNext-02830");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Features12Extensions) {
    TEST_DESCRIPTION("Checks that 1.2 features are enabled if extension is passed in.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        printf("%s Vulkan12Struct requires Vulkan 1.2+, skipping test\n", kSkipPrefix);
        return;
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
    VkPhysicalDeviceVulkan12Features features12 = {};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.pNext = nullptr;
    features12.drawIndirectCount = VK_FALSE;
    features12.samplerMirrorClampToEdge = VK_FALSE;
    features12.descriptorIndexing = VK_FALSE;
    features12.samplerFilterMinmax = VK_FALSE;
    features12.shaderOutputViewportIndex = VK_FALSE;
    features12.shaderOutputLayer = VK_TRUE;  // Set true since both shader_viewport features need to true

    std::vector<const char *> device_extensions;

    // Go through each extension and if supported add to list and add failure to check for
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)) {
        device_extensions.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02831");
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME)) {
        device_extensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02832");
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)) {
        device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02833");
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)) {
        device_extensions.push_back(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02834");
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)) {
        device_extensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-ppEnabledExtensions-02835");
    }

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &features12;
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
    auto variable_features = lvl_init_struct<VkPhysicalDeviceVariablePointersFeatures>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&variable_features);
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

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &features2;
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

    auto multiview_features = lvl_init_struct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&multiview_features);
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

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &features2;
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
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-02804");
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vk::BeginCommandBuffer(m_commandBuffer->handle(), &begin_info);
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, SwapchainAcquireImageNoSync) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with VK_NULL_HANDLE semaphore and fence");

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImageKHR-semaphore-01780");
        uint32_t dummy;
        vk::AcquireNextImageKHR(device(), m_swapchain, UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &dummy);
        m_errorMonitor->VerifyFound();
    }

    DestroySwapchain();
}

TEST_F(VkLayerTest, SwapchainAcquireImageNoSync2KHR) {
    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with VK_NULL_HANDLE semaphore and fence");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    bool extension_dependency_satisfied = false;
    if (InstanceExtensionSupported(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
        extension_dependency_satisfied = true;
    } else if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s vkAcquireNextImage2KHR not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (extension_dependency_satisfied && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    } else if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s vkAcquireNextImage2KHR not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-semaphore-01782");
        VkAcquireNextImageInfoKHR acquire_info = {VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR};
        acquire_info.swapchain = m_swapchain;
        acquire_info.timeout = UINT64_MAX;
        acquire_info.semaphore = VK_NULL_HANDLE;
        acquire_info.fence = VK_NULL_HANDLE;
        acquire_info.deviceMask = 0x1;

        uint32_t dummy;
        vk::AcquireNextImage2KHR(device(), &acquire_info, &dummy);
        m_errorMonitor->VerifyFound();
    }

    DestroySwapchain();
}

TEST_F(VkLayerTest, SwapchainAcquireImageNoBinarySemaphore) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with non-binary semaphore");

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_TRUE(InitSwapchain());

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info{};
    semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = &semaphore_type_create_info;

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImageKHR-semaphore-03265");
    uint32_t image_i;
    vk::AcquireNextImageKHR(device(), m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &image_i);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    DestroySwapchain();
}

TEST_F(VkLayerTest, SwapchainAcquireImageNoBinarySemaphore2KHR) {
    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with non-binary semaphore");

    TEST_DESCRIPTION("Test vkAcquireNextImage2KHR with VK_NULL_HANDLE semaphore and fence");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    bool extension_dependency_satisfied = false;
    if (InstanceExtensionSupported(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
        extension_dependency_satisfied = true;
    } else if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s vkAcquireNextImage2KHR not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (extension_dependency_satisfied && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    } else if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s vkAcquireNextImage2KHR not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!CheckTimelineSemaphoreSupportAndInitState(this)) {
        printf("%s Timeline semaphore not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_TRUE(InitSwapchain());

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info{};
    semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = &semaphore_type_create_info;

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkAcquireNextImageInfoKHR acquire_info = {};
    acquire_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = UINT64_MAX;
    acquire_info.semaphore = semaphore;
    acquire_info.deviceMask = 0x1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-semaphore-03266");
    uint32_t image_i;
    vk::AcquireNextImage2KHR(device(), &acquire_info, &image_i);
    m_errorMonitor->VerifyFound();

    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    DestroySwapchain();
}

TEST_F(VkLayerTest, SwapchainAcquireTooManyImages) {
    TEST_DESCRIPTION("Acquiring invalid amount of images from the swapchain.");

    if (!AddSurfaceInstanceExtension()) return;
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AddSwapchainDeviceExtension()) return;

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());
    uint32_t image_count;
    ASSERT_VK_SUCCESS(vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr));
    VkSurfaceCapabilitiesKHR caps;
    ASSERT_VK_SUCCESS(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu(), m_surface, &caps));

    const uint32_t acquirable_count = image_count - caps.minImageCount + 1;
    std::vector<VkFenceObj> fences(acquirable_count);
    for (uint32_t i = 0; i < acquirable_count; ++i) {
        fences[i].init(*m_device, VkFenceObj::create_info());
        uint32_t image_i;
        const auto res = vk::AcquireNextImageKHR(device(), m_swapchain, UINT64_MAX, VK_NULL_HANDLE, fences[i].handle(), &image_i);
        ASSERT_TRUE(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);
    }
    VkFenceObj error_fence;
    error_fence.init(*m_device, VkFenceObj::create_info());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImageKHR-swapchain-01802");
    uint32_t image_i;
    vk::AcquireNextImageKHR(device(), m_swapchain, UINT64_MAX, VK_NULL_HANDLE, error_fence.handle(), &image_i);
    m_errorMonitor->VerifyFound();

    // Cleanup
    vk::WaitForFences(device(), fences.size(), MakeVkHandles<VkFence>(fences).data(), VK_TRUE, UINT64_MAX);
    DestroySwapchain();
}

TEST_F(VkLayerTest, SwapchainAcquireTooManyImages2KHR) {
    TEST_DESCRIPTION("Acquiring invalid amount of images from the swapchain via vkAcquireNextImage2KHR.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    bool extension_dependency_satisfied = false;
    if (InstanceExtensionSupported(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
        extension_dependency_satisfied = true;
    } else if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s vkAcquireNextImage2KHR not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AddSurfaceInstanceExtension()) return;
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (extension_dependency_satisfied && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    } else if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s vkAcquireNextImage2KHR not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AddSwapchainDeviceExtension()) return;

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());
    uint32_t image_count;
    ASSERT_VK_SUCCESS(vk::GetSwapchainImagesKHR(device(), m_swapchain, &image_count, nullptr));
    VkSurfaceCapabilitiesKHR caps;
    ASSERT_VK_SUCCESS(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(gpu(), m_surface, &caps));

    const uint32_t acquirable_count = image_count - caps.minImageCount + 1;
    std::vector<VkFenceObj> fences(acquirable_count);
    for (uint32_t i = 0; i < acquirable_count; ++i) {
        fences[i].init(*m_device, VkFenceObj::create_info());
        uint32_t image_i;
        const auto res = vk::AcquireNextImageKHR(device(), m_swapchain, UINT64_MAX, VK_NULL_HANDLE, fences[i].handle(), &image_i);
        ASSERT_TRUE(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);
    }
    VkFenceObj error_fence;
    error_fence.init(*m_device, VkFenceObj::create_info());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImage2KHR-swapchain-01803");
    VkAcquireNextImageInfoKHR acquire_info = {VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR};
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = UINT64_MAX;
    acquire_info.fence = error_fence.handle();
    acquire_info.deviceMask = 0x1;

    uint32_t image_i;
    vk::AcquireNextImage2KHR(device(), &acquire_info, &image_i);
    m_errorMonitor->VerifyFound();

    // Cleanup
    vk::WaitForFences(device(), fences.size(), MakeVkHandles<VkFence>(fences).data(), VK_TRUE, UINT64_MAX);
    DestroySwapchain();
}

TEST_F(VkLayerTest, InvalidDeviceMask) {
    TEST_DESCRIPTION("Invalid deviceMask.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    bool support_surface = true;
    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping VkAcquireNextImageInfoKHR test\n", kSkipPrefix);
        support_surface = false;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (support_surface) {
        if (!AddSwapchainDeviceExtension()) {
            printf("%s swapchain extensions not supported, skipping BindSwapchainImageMemory test\n", kSkipPrefix);
            support_surface = false;
        }
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Device Groups requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
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
    VkDeviceGroupDeviceCreateInfo create_device_pnext = {};
    create_device_pnext.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!InitSwapchain()) {
        printf("%s Cannot create surface or swapchain, skipping VkAcquireNextImageInfoKHR test\n", kSkipPrefix);
        support_surface = false;
    }

    // Test VkMemoryAllocateFlagsInfo
    VkMemoryAllocateFlagsInfo alloc_flags_info = {};
    alloc_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT;
    alloc_flags_info.deviceMask = 0xFFFFFFFF;
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = &alloc_flags_info;
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = 1024;

    VkDeviceMemory mem;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00675");
    vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    m_errorMonitor->VerifyFound();

    alloc_flags_info.deviceMask = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00676");
    vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    m_errorMonitor->VerifyFound();

    uint32_t pdev_group_count = 0;
    std::vector<VkPhysicalDeviceGroupProperties> group_props;
    VkResult err = vk::EnumeratePhysicalDeviceGroups(instance(), &pdev_group_count, nullptr);
    group_props.resize(pdev_group_count);
    err = vk::EnumeratePhysicalDeviceGroups(instance(), &pdev_group_count, &group_props[0]);

    auto tgt = gpu();
    bool test_run = false;
    for (uint32_t i = 0; i < pdev_group_count; i++) {
        if ((group_props[i].physicalDeviceCount > 1) && !test_run) {
            for (uint32_t j = 0; j < group_props[i].physicalDeviceCount; j++) {
                if (tgt == group_props[i].physicalDevices[j]) {
                    void *data;
                    VkDeviceMemory mi_mem;
                    alloc_flags_info.deviceMask = 3;
                    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mi_mem);
                    if (VK_SUCCESS == err) {
                        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-memory-00683");
                        vk::MapMemory(m_device->device(), mi_mem, 0, 1024, 0, &data);
                        m_errorMonitor->VerifyFound();
                        vk::FreeMemory(m_device->device(), mi_mem, nullptr);
                    }
                    test_run = true;
                    break;
                }
            }
        }
    }

    // Test VkDeviceGroupCommandBufferBeginInfo
    VkDeviceGroupCommandBufferBeginInfo dev_grp_cmd_buf_info = {};
    dev_grp_cmd_buf_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO;
    dev_grp_cmd_buf_info.deviceMask = 0xFFFFFFFF;
    VkCommandBufferBeginInfo cmd_buf_info = {};
    cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_info.pNext = &dev_grp_cmd_buf_info;

    m_commandBuffer->reset();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00106");
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    dev_grp_cmd_buf_info.deviceMask = 0;
    m_commandBuffer->reset();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00107");
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    // Test VkDeviceGroupRenderPassBeginInfo
    dev_grp_cmd_buf_info.deviceMask = 0x00000001;
    m_commandBuffer->reset();
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);

    VkDeviceGroupRenderPassBeginInfo dev_grp_rp_info = {};
    dev_grp_rp_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO;
    dev_grp_rp_info.deviceMask = 0xFFFFFFFF;
    m_renderPassBeginInfo.pNext = &dev_grp_rp_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00905");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00907");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    dev_grp_rp_info.deviceMask = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00906");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    dev_grp_rp_info.deviceMask = 0x00000001;
    dev_grp_rp_info.deviceRenderAreaCount = physical_device_group[0].physicalDeviceCount + 1;
    std::vector<VkRect2D> device_render_areas(dev_grp_rp_info.deviceRenderAreaCount, m_renderPassBeginInfo.renderArea);
    dev_grp_rp_info.pDeviceRenderAreas = device_render_areas.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceRenderAreaCount-00908");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    // Test vk::CmdSetDeviceMask()
    vk::CmdSetDeviceMask(m_commandBuffer->handle(), 0x00000001);

    dev_grp_rp_info.deviceRenderAreaCount = physical_device_group[0].physicalDeviceCount;
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDeviceMask-deviceMask-00108");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDeviceMask-deviceMask-00110");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDeviceMask-deviceMask-00111");
    vk::CmdSetDeviceMask(m_commandBuffer->handle(), 0xFFFFFFFF);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDeviceMask-deviceMask-00109");
    vk::CmdSetDeviceMask(m_commandBuffer->handle(), 0);
    m_errorMonitor->VerifyFound();

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));
    VkSemaphore semaphore2;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore2));
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;
    ASSERT_VK_SUCCESS(vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence));

    if (support_surface) {
        // Test VkAcquireNextImageInfoKHR
        uint32_t imageIndex;
        VkAcquireNextImageInfoKHR acquire_next_image_info = {};
        acquire_next_image_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
        acquire_next_image_info.semaphore = semaphore;
        acquire_next_image_info.swapchain = m_swapchain;
        acquire_next_image_info.fence = fence;
        acquire_next_image_info.deviceMask = 0xFFFFFFFF;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-deviceMask-01290");
        vk::AcquireNextImage2KHR(m_device->device(), &acquire_next_image_info, &imageIndex);
        m_errorMonitor->VerifyFound();

        vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, std::numeric_limits<int>::max());
        vk::ResetFences(m_device->device(), 1, &fence);

        acquire_next_image_info.semaphore = semaphore2;
        acquire_next_image_info.deviceMask = 0;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-deviceMask-01291");
        vk::AcquireNextImage2KHR(m_device->device(), &acquire_next_image_info, &imageIndex);
        m_errorMonitor->VerifyFound();
        DestroySwapchain();
    }

    // Test VkDeviceGroupSubmitInfo
    VkDeviceGroupSubmitInfo device_group_submit_info = {};
    device_group_submit_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO;
    device_group_submit_info.commandBufferCount = 1;
    std::array<uint32_t, 1> command_buffer_device_masks = {{0xFFFFFFFF}};
    device_group_submit_info.pCommandBufferDeviceMasks = command_buffer_device_masks.data();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = &device_group_submit_info;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    m_commandBuffer->reset();
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupSubmitInfo-pCommandBufferDeviceMasks-00086");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);

    vk::WaitForFences(m_device->device(), 1, &fence, VK_TRUE, std::numeric_limits<int>::max());
    vk::DestroyFence(m_device->device(), fence, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore, nullptr);
    vk::DestroySemaphore(m_device->device(), semaphore2, nullptr);
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

    VkValidationCacheCreateInfoEXT validationCacheCreateInfo;
    validationCacheCreateInfo.sType = VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT;
    validationCacheCreateInfo.pNext = NULL;
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
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkBufferCreateInfo buffCI = {};
    buffCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffCI.queueFamilyIndexCount = 2;
    // Introduce failure by specifying invalid queue_family_index
    uint32_t qfi[2];
    qfi[0] = 777;
    qfi[1] = 0;

    buffCI.pQueueFamilyIndices = qfi;
    buffCI.sharingMode = VK_SHARING_MODE_CONCURRENT;  // qfi only matters in CONCURRENT mode

    // Test for queue family index out of range
    CreateBufferTest(*this, &buffCI, "VUID-VkBufferCreateInfo-sharingMode-01419");

    // Test for non-unique QFI in array
    qfi[0] = 0;
    CreateBufferTest(*this, &buffCI, "VUID-VkBufferCreateInfo-sharingMode-01419");

    if (m_device->queue_props.size() > 2) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "which was not created allowing concurrent");

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
    float priorities = {1.0f};
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = queue_props->queueCount;
    queue_info.pQueuePriorities = &priorities;
    VkDeviceCreateInfo dev_info{};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
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
    VkDeviceCreateInfo device_create_info = {};
    auto features = m_device->phy().features();
    // Intentionally disable pipeline stats
    features.pipelineStatisticsQuery = VK_FALSE;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.pEnabledFeatures = &features;
    VkResult err = vk::CreateDevice(gpu(), &device_create_info, nullptr, &local_device);
    ASSERT_VK_SUCCESS(err);

    VkQueryPoolCreateInfo qpci{};
    qpci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
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
    VkQueueFamilyProperties *queue_props = new VkQueueFamilyProperties[queue_count];
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props);
    const uint32_t timestampValidBits = queue_props[m_device->graphics_queue_node_index_].timestampValidBits;

    VkBufferObj buffer;
    buffer.init(*m_device, 128, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkMemoryRequirements mem_reqs = {};
    vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &mem_reqs);
    const VkDeviceSize buffer_size = mem_reqs.size;

    const uint32_t query_pool_size = 4;
    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = query_pool_size;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_commandBuffer->begin();

    // firstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00796");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00797");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, query_pool_size, 1);
    m_errorMonitor->VerifyFound();

    // sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetQueryPool-firstQuery-00797");
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 1, query_pool_size);
    m_errorMonitor->VerifyFound();

    // Actually reset all queries so they can be used
    m_errorMonitor->ExpectSuccess();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, query_pool_size);
    m_errorMonitor->VerifyNotFound();

    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);

    // query index to large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndQuery-query-00810");
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, query_pool_size);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);

    // firstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00820");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, query_pool_size, 1, buffer.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-firstQuery-00821");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 1, query_pool_size, buffer.handle(), 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // offset larger than buffer size
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-dstOffset-00819");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), buffer_size + 4, 0, 0);
    m_errorMonitor->VerifyFound();

    // buffer does not have enough storage from offset to contain result of each query
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-dstBuffer-00824");
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 2, buffer.handle(), buffer_size - 4, 4, 0);
    m_errorMonitor->VerifyFound();

    // Query is not a timestamp type
    if (timestampValidBits == 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-timestampValidBits-00829");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWriteTimestamp-queryPool-01416");
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, query_pool, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    const size_t out_data_size = 128;
    uint8_t data[out_data_size];

    // firstQuery is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00813");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00816");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidQuery");
    vk::GetQueryPoolResults(m_device->device(), query_pool, query_pool_size, 1, out_data_size, &data, 0, 0);
    m_errorMonitor->VerifyFound();

    // sum of firstQuery and queryCount is too large
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-firstQuery-00816");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidQuery");
    vk::GetQueryPoolResults(m_device->device(), query_pool, 1, query_pool_size, out_data_size, &data, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, UnclosedAndDuplicateQueries) {
    TEST_DESCRIPTION("End a command buffer with a query still in progress, create nested queries.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = {};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
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
        VkEventCreateInfo event_create_info{};
        event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        vk::CreateEvent(m_device->handle(), &event_create_info, nullptr, &event);

        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00800");

        VkQueryPool query_pool;
        VkQueryPoolCreateInfo query_pool_create_info = {};
        query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        query_pool_create_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_create_info.queryCount = 1;
        vk::CreateQueryPool(m_device->handle(), &query_pool_create_info, nullptr, &query_pool);

        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
        vk::DestroyQueryPool(m_device->handle(), query_pool, nullptr);
        vk::DestroyEvent(m_device->handle(), event, nullptr);
    }

    // Test for precise bit when precise feature is not available
    features.occlusionQueryPrecise = false;
    VkDeviceObj test_device(0, gpu(), device_extension_names, &features);

    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vk::CreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext = NULL;
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_VK_SUCCESS(err);

    VkEvent event;
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(test_device.handle(), &event_create_info, nullptr, &event);

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    vk::BeginCommandBuffer(cmd_buffer, &begin_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-00800");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = {};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(test_device.handle(), &query_pool_create_info, nullptr, &query_pool);

    vk::CmdResetQueryPool(cmd_buffer, query_pool, 0, 1);
    vk::CmdBeginQuery(cmd_buffer, query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
    m_errorMonitor->VerifyFound();

    vk::EndCommandBuffer(cmd_buffer);
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

    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vk::CreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext = NULL;
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_VK_SUCCESS(err);

    VkEvent event;
    VkEventCreateInfo evci = {};
    evci.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    VkResult result = vk::CreateEvent(test_device.handle(), &evci, NULL, &event);
    ASSERT_VK_SUCCESS(result);

    VkCommandBufferBeginInfo cbbi = {};
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
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
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-01149");
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent-stageMask-01153");
    vk::CmdResetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_HOST_BIT);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_HOST_BIT;
    VkSubmitInfo submit_info = {};

    // Signal the semaphore so the next test can wait on it.
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyNotFound();

    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkShaderObj fs(m_device, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
    dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    VkImageCreateInfo image_ci = {};
    image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_ci.pNext = NULL;
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
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkEventCreateInfo event_create_info = {};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    m_commandBuffer->end();
    vk::DestroyEvent(m_device->device(), event, nullptr);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;
    ASSERT_VK_SUCCESS(vk::CreateFence(m_device->device(), &fence_create_info, nullptr, &fence));

    VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_test.GetBuffer(), 1024, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    VkEvent event;
    VkEventCreateInfo event_create_info = {};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    m_commandBuffer->begin();

    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkEventCreateInfo event_create_info = {};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    vk::CmdWaitEvents(commandBuffer1.handle(), 1, &event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer1.end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkEventCreateInfo event_create_info = {};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    // wrong srcStageMask
    vk::CmdWaitEvents(commandBuffer1.handle(), 1, &event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    commandBuffer1.end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkEventCreateInfo event_create_info = {};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    commandBuffer1.end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkEventCreateInfo event_create_info = {};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    commandBuffer1.begin();
    vk::CmdSetEvent(commandBuffer1.handle(), event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    commandBuffer1.end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkQueueFamilyProperties *queue_props = new VkQueueFamilyProperties[queue_count];
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props);
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        printf("%s Device graphic queue has timestampValidBits of 0, skipping.\n", kSkipPrefix);
        return;
    }

    VkBufferObj buffer;
    buffer.init(*m_device, 128, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
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
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

TEST_F(VkLayerTest, QueryPoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use query pool.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_commandBuffer->begin();
    // Use query pool to create binding with cmd buffer
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
    m_commandBuffer->end();

    // Submit cmd buffer and then destroy query pool while in-flight
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkShaderObj fs(m_device, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
    dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkBufferView view;
    VkBufferViewCreateInfo bvci = {};
    bvci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;

    VkResult err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
    ASSERT_VK_SUCCESS(err);

    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(set=0, binding=0, r32f) uniform readonly imageBuffer s;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = imageLoad(s, 0);\n"
        "}\n";
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
    dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkShaderObj fs(m_device, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
    dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    test_platform_thread thread;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "THREADING ERROR");
    m_errorMonitor->SetAllowedFailureMsg("THREADING ERROR");  // Ignore any extra threading errors found beyond the first one

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Calls AllocateCommandBuffers
    VkCommandBufferObj commandBuffer(m_device, m_commandPool);

    commandBuffer.begin();

    VkEventCreateInfo event_info;
    VkEvent event;
    VkResult err;

    memset(&event_info, 0, sizeof(event_info));
    event_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

    err = vk::CreateEvent(device(), &event_info, NULL, &event);
    ASSERT_VK_SUCCESS(err);

    err = vk::ResetEvent(device(), event);
    ASSERT_VK_SUCCESS(err);

    struct thread_data_struct data;
    data.commandBuffer = commandBuffer.handle();
    data.event = event;
    bool bailout = false;
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // First do some correct operations using multiple threads.
    // Add many entries to command buffer from another thread.
    test_platform_thread_create(&thread, AddToCommandBuffer, (void *)&data);
    // Make non-conflicting calls from this thread at the same time.
    for (int i = 0; i < 80000; i++) {
        uint32_t count;
        vk::EnumeratePhysicalDevices(instance(), &count, NULL);
    }
    test_platform_thread_join(thread, NULL);

    // Then do some incorrect operations using multiple threads.
    // Add many entries to command buffer from another thread.
    test_platform_thread_create(&thread, AddToCommandBuffer, (void *)&data);
    // Add many entries to command buffer from this thread at the same time.
    AddToCommandBuffer(&data);

    test_platform_thread_join(thread, NULL);
    commandBuffer.end();

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyFound();

    vk::DestroyEvent(device(), event, NULL);
}

TEST_F(VkLayerTest, ThreadUpdateDescriptorCollision) {
    TEST_DESCRIPTION("Two threads updating the same descriptor set, expected to generate a threading error");
    test_platform_thread thread;

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

    struct thread_data_struct data;
    data.device = device();
    data.descriptorSet = normal_descriptor_set.set_;
    data.binding = 0;
    data.buffer = buffer.handle();
    bool bailout = false;
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Update descriptors from another thread.
    test_platform_thread_create(&thread, UpdateDescriptor, (void *)&data);
    // Update descriptors from this thread at the same time.

    struct thread_data_struct data2;
    data2.device = device();
    data2.descriptorSet = normal_descriptor_set.set_;
    data2.binding = 1;
    data2.buffer = buffer.handle();
    data2.bailout = &bailout;

    UpdateDescriptor(&data2);

    test_platform_thread_join(thread, NULL);

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ThreadUpdateDescriptorUpdateAfterBindNoCollision) {
    TEST_DESCRIPTION("Two threads updating the same UAB descriptor set, expected not to generate a threading error");
    test_platform_thread thread;
    m_errorMonitor->ExpectSuccess();

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) &&
        DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE3_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    } else {
        printf("%s Descriptor Indexing or Maintenance3 Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables descriptorBindingStorageBufferUpdateAfterBind
    auto indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
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
    auto flags_create_info = lvl_init_struct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
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

    struct thread_data_struct data;
    data.device = device();
    data.descriptorSet = normal_descriptor_set.set_;
    data.binding = 0;
    data.buffer = buffer.handle();
    bool bailout = false;
    data.bailout = &bailout;
    m_errorMonitor->SetBailout(data.bailout);

    // Update descriptors from another thread.
    test_platform_thread_create(&thread, UpdateDescriptor, (void *)&data);
    // Update descriptors from this thread at the same time.

    struct thread_data_struct data2;
    data2.device = device();
    data2.descriptorSet = normal_descriptor_set.set_;
    data2.binding = 1;
    data2.buffer = buffer.handle();
    data2.bailout = &bailout;

    UpdateDescriptor(&data2);

    test_platform_thread_join(thread, NULL);

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyNotFound();
}
#endif  // GTEST_IS_THREADSAFE

TEST_F(VkLayerTest, ExecuteUnrecordedPrimaryCB) {
    TEST_DESCRIPTION("Attempt vkQueueSubmit with a CB in the initial state");
    ASSERT_NO_FATAL_FAILURE(Init());
    // never record m_commandBuffer

    VkSubmitInfo si = {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &m_commandBuffer->handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkQueueSubmit-pCommandBuffers-00072");
    vk::QueueSubmit(m_device->m_queue, 1, &si, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Maintenance1AndNegativeViewport) {
    TEST_DESCRIPTION("Attempt to enable AMD_negative_viewport_height and Maintenance1_KHR extension simultaneously");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!((DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) &&
          (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME)))) {
        printf("%s Maintenance1 and AMD_negative viewport height extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    const char *extension_names[2] = {"VK_KHR_maintenance1", "VK_AMD_negative_viewport_height"};
    VkDevice testDevice;
    VkDeviceCreateInfo device_create_info = {};
    auto features = m_device->phy().features();
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
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

TEST_F(VkLayerTest, HostQueryResetNotEnabled) {
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT without enabling the feature");

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
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

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features{};
    host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2{};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &host_query_reset_features;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
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

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features{};
    host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2{};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &host_query_reset_features;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
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

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features{};
    host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2{};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &host_query_reset_features;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    // Create and destroy a query pool.
    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
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

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features{};
    host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2{};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &host_query_reset_features;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vk::GetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    // Create a second device with the feature enabled.
    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    auto features = m_device->phy().features();

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &host_query_reset_features;
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
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vk::CreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vk::BeginCommandBuffer(command_buffer, &begin_info);

        vk::CmdResetEvent(command_buffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        vk::EndCommandBuffer(command_buffer);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables shading_rate_image but disables multiViewport
    auto shading_rate_image_features = lvl_init_struct<VkPhysicalDeviceShadingRateImageFeaturesNV>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&shading_rate_image_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    features2.features.multiViewport = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test shading rate image creation
    VkResult result = VK_RESULT_MAX_ENUM;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
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
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
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
    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.pNext = nullptr;
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02066");
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

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include "android_ndk_types.h"

TEST_F(VkLayerTest, AndroidHardwareBufferImageCreate) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer image create info.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkImage img = VK_NULL_HANDLE;
    auto reset_img = [&img, dev]() {
        if (VK_NULL_HANDLE != img) vk::DestroyImage(dev, img, NULL);
        img = VK_NULL_HANDLE;
    };

    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.pNext = nullptr;
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
    m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-Image-FormatNotSupported");
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    vk::CreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();

    // also undefined format
    VkExternalFormatANDROID efa = {};
    efa.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
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
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = {};
    ahb_fmt_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props.pNext = &ahb_fmt_props;
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
    VkExternalMemoryImageCreateInfo emici = {};
    emici.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkImage img = VK_NULL_HANDLE;
    auto reset_img = [&img, dev]() {
        if (VK_NULL_HANDLE != img) vk::DestroyImage(dev, img, NULL);
        img = VK_NULL_HANDLE;
    };

    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.pNext = nullptr;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkExternalMemoryImageCreateInfo emici = {};
    emici.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
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
    VkImageMemoryRequirementsInfo2 imri = {};
    imri.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
    imri.image = img;
    VkMemoryRequirements2 mem_reqs = {};
    mem_reqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-01897");
    vk::GetImageMemoryRequirements2(dev, &imri, &mem_reqs);
    m_errorMonitor->VerifyFound();

    reset_img();
}

TEST_F(VkLayerTest, AndroidHardwareBufferMemoryAllocation) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer memory allocation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
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
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = {};
    ahb_fmt_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props.pNext = &ahb_fmt_props;
    VkImportAndroidHardwareBufferInfoANDROID iahbi = {};
    iahbi.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;

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
    VkExternalFormatANDROID efa = {};
    efa.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    efa.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.pNext = &efa;
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

    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.pNext = &iahbi;  // Chained import struct
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
    VkMemoryDedicatedAllocateInfo mdai = {};
    mdai.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
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
    VkExportMemoryAllocateInfo emai = {};
    emai.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    // Enable Ycbcr Conversion Features
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = {};
    ycbcr_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
    ycbcr_features.samplerYcbcrConversion = VK_TRUE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ycbcr_features));
    VkDevice dev = m_device->device();

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = {};
    sycci.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-format-04061");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    VkExternalFormatANDROID efa = {};
    efa.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    efa.externalFormat = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    sycci.format = VK_FORMAT_R8G8B8A8_UNORM;
    sycci.pNext = &efa;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01904");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, AndroidHardwareBufferPhysDevImageFormatProp2) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer GetPhysicalDeviceImageFormatProperties.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    if ((DeviceValidationVersion() < VK_API_VERSION_1_1) &&
        !InstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    VkImageFormatProperties2 ifp = {};
    ifp.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    VkPhysicalDeviceImageFormatInfo2 pdifi = {};
    pdifi.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    pdifi.format = VK_FORMAT_R8G8B8A8_UNORM;
    pdifi.tiling = VK_IMAGE_TILING_OPTIMAL;
    pdifi.type = VK_IMAGE_TYPE_2D;
    pdifi.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkAndroidHardwareBufferUsageANDROID ahbu = {};
    ahbu.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID;
    ahbu.androidHardwareBufferUsage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ifp.pNext = &ahbu;

    // AHB_usage chained to input without a matching external image format struc chained to output
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868");
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &pdifi, &ifp);
    m_errorMonitor->VerifyFound();

    // output struct chained, but does not include VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID usage
    VkPhysicalDeviceExternalImageFormatInfo pdeifi = {};
    pdeifi.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
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
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = {};
    ahb_fmt_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props.pNext = &ahb_fmt_props;
    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vk::GetDeviceProcAddr(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);
    pfn_GetAHBProps(dev, ahb, &ahb_props);
    AHardwareBuffer_release(ahb);

    VkExternalMemoryImageCreateInfo emici = {};
    emici.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    emici.pNext = nullptr;
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    // Give image an external format
    VkExternalFormatANDROID efa = {};
    efa.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    efa.pNext = (void *)&emici;
    efa.externalFormat = ahb_fmt_props.externalFormat;

    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Create another VkExternalFormatANDROID for test VUID-VkImageViewCreateInfo-image-02400
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props_Ycbcr = {};
    ahb_fmt_props_Ycbcr.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    VkAndroidHardwareBufferPropertiesANDROID ahb_props_Ycbcr = {};
    ahb_props_Ycbcr.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props_Ycbcr.pNext = &ahb_fmt_props_Ycbcr;
    pfn_GetAHBProps(dev, ahb, &ahb_props_Ycbcr);
    AHardwareBuffer_release(ahb);

    VkExternalFormatANDROID efa_Ycbcr = {};
    efa_Ycbcr.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    efa_Ycbcr.externalFormat = ahb_fmt_props_Ycbcr.externalFormat;

    // Need to make sure format has sample bit needed for image usage
    if ((ahb_fmt_props_Ycbcr.formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0) {
        printf("%s VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT feature bit not supported for format %" PRIu64 ".", kSkipPrefix,
               ahb_fmt_props_Ycbcr.externalFormat);
        return;
    }

    // Create the image
    VkImage img = VK_NULL_HANDLE;
    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.pNext = &efa;
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
    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
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
    VkSamplerYcbcrConversionCreateInfo sycci = {};
    sycci.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    sycci.pNext = &efa_Ycbcr;
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    vk::CreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    VkSamplerYcbcrConversionInfo syci = {};
    syci.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
    syci.conversion = ycbcr_conv;

    // Create a view
    VkImageView image_view = VK_NULL_HANDLE;
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.pNext = &syci;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
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
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    VkImportAndroidHardwareBufferInfoANDROID iahbi = {};
    iahbi.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;

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
    VkExternalMemoryBufferCreateInfo ext_buf_info = {};
    ext_buf_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR;
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;

    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.pNext = &ext_buf_info;
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkDeviceMemory mem_handle = VK_NULL_HANDLE;

    // Allocate device memory, no linked export struct indicating AHB handle type
    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = 65536;
    mai.memoryTypeIndex = 0;
    vk::AllocateMemory(dev, &mai, NULL, &mem_handle);

    PFN_vkGetMemoryAndroidHardwareBufferANDROID pfn_GetMemAHB =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)vk::GetDeviceProcAddr(dev, "vkGetMemoryAndroidHardwareBufferANDROID");
    ASSERT_TRUE(pfn_GetMemAHB != nullptr);

    VkMemoryGetAndroidHardwareBufferInfoANDROID mgahbi = {};
    mgahbi.sType = VK_STRUCTURE_TYPE_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
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
    VkExportMemoryAllocateInfo emai = {};
    emai.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    emai.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    mai.pNext = &emai;

    // Create an image, do not bind memory
    VkImage img = VK_NULL_HANDLE;
    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
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
    VkMemoryDedicatedAllocateInfo mdai = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO};
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
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

    VkExternalMemoryBufferCreateInfo ext_buf_info = {};
    ext_buf_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR;
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = &ext_buf_info;
    buffer_create_info.size = 1 << 20;  // 1 MB
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkBuffer buffer = VK_NULL_HANDLE;
    vk::CreateBuffer(m_device->device(), &buffer_create_info, nullptr, &buffer);

    // Try to get memory requirements prior to binding memory
    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = {};
    import_ahb_Info.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    import_ahb_Info.pNext = nullptr;
    import_ahb_Info.buffer = ahb;

    VkMemoryAllocateInfo memory_info = {};
    memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_info.pNext = &import_ahb_Info;
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
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
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = nullptr;
    buffer_create_info.size = 512;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    vk::CreateBuffer(m_device->device(), &buffer_create_info, nullptr, &buffer);

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = {};
    import_ahb_Info.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    import_ahb_Info.pNext = nullptr;
    import_ahb_Info.buffer = ahb;

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props.pNext = nullptr;
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.pNext = &import_ahb_Info;
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

    VkBindBufferMemoryInfo bind_buffer_info = {};
    bind_buffer_info.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
    bind_buffer_info.pNext = nullptr;
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
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
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
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

    VkMemoryDedicatedAllocateInfo memory_dedicated_info = {};
    memory_dedicated_info.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    memory_dedicated_info.pNext = nullptr;
    memory_dedicated_info.image = image;
    memory_dedicated_info.buffer = VK_NULL_HANDLE;

    VkImportAndroidHardwareBufferInfoANDROID import_ahb_Info = {};
    import_ahb_Info.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    import_ahb_Info.pNext = &memory_dedicated_info;
    import_ahb_Info.buffer = ahb;

    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props.pNext = nullptr;
    pfn_GetAHBProps(m_device->device(), ahb, &ahb_props);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.pNext = &import_ahb_Info;
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

    VkBindImageMemoryInfo bind_image_info = {};
    bind_image_info.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bind_image_info.pNext = nullptr;
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

#endif  // VK_USE_PLATFORM_ANDROID_KHR

TEST_F(VkLayerTest, ValidateStride) {
    TEST_DESCRIPTION("Validate Stride.");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (IsPlatform(kPixelC)) {
        printf("%s This test should not run on Pixel C\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    m_commandBuffer->begin();
    vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    vk::CmdWriteTimestamp(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
    VkBufferCreateInfo buff_create_info = {};
    buff_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
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

TEST_F(VkLayerTest, WarningSwapchainCreateInfoPreTransform) {
    TEST_DESCRIPTION("Print warning when preTransform doesn't match curretTransform");

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-CoreValidation-SwapchainPreTransform");
    m_errorMonitor->SetUnexpectedError("VUID-VkSwapchainCreateInfoKHR-preTransform-01279");
    InitSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR);
    m_errorMonitor->VerifyFound();
    DestroySwapchain();
}

TEST_F(VkLayerTest, ValidateGeometryNV) {
    TEST_DESCRIPTION("Validate acceleration structure geometries.");
    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor)) {
        return;
    }

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

    VkBufferCreateInfo unbound_buffer_ci = {};
    unbound_buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
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

    VkGeometryNV valid_geometry_triangles = {};
    valid_geometry_triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    valid_geometry_triangles.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
    valid_geometry_triangles.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
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
    valid_geometry_triangles.geometry.aabbs = {};
    valid_geometry_triangles.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;

    VkGeometryNV valid_geometry_aabbs = {};
    valid_geometry_aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
    valid_geometry_aabbs.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
    valid_geometry_aabbs.geometry.triangles = {};
    valid_geometry_aabbs.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
    valid_geometry_aabbs.geometry.aabbs = {};
    valid_geometry_aabbs.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
    valid_geometry_aabbs.geometry.aabbs.aabbData = aabbbo.handle();
    valid_geometry_aabbs.geometry.aabbs.numAABBs = 1;
    valid_geometry_aabbs.geometry.aabbs.offset = 0;
    valid_geometry_aabbs.geometry.aabbs.stride = 24;

    PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV = reinterpret_cast<PFN_vkCreateAccelerationStructureNV>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateAccelerationStructureNV"));
    assert(vkCreateAccelerationStructureNV != nullptr);

    const auto GetCreateInfo = [](const VkGeometryNV &geometry) {
        VkAccelerationStructureCreateInfoNV as_create_info = {};
        as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
        as_create_info.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
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
        vkCreateAccelerationStructureNV(m_device->handle(), &as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateCreateAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure creation.");
    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor)) {
        return;
    }

    PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV = reinterpret_cast<PFN_vkCreateAccelerationStructureNV>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateAccelerationStructureNV"));
    assert(vkCreateAccelerationStructureNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = {};
    as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    as_create_info.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;

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
        VkGeometryNV aabb_geometry = {};
        aabb_geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
        aabb_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
        aabb_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
        aabb_geometry.geometry.aabbs = {};
        aabb_geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
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
    if (!InitFrameworkForRayTracingTest(this, true, m_instance_extension_names, m_device_extension_names, m_errorMonitor, false,
                                        false, true)) {
        return;
    }

    auto ray_tracing_features = lvl_init_struct<VkPhysicalDeviceRayTracingFeaturesKHR>();
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (ray_tracing_features.rayQuery == VK_FALSE && ray_tracing_features.rayTracing == VK_FALSE) {
        printf("%s Both of the required features rayQuery and rayTracing are not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ray_tracing_features));
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCreateAccelerationStructureKHR"));
    assert(vkCreateAccelerationStructureKHR != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    // Get an NV geometry in the helper, then pull out the bits we need for Create
    VkGeometryNV geometryNV;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometryNV);

    VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo = {};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
    geometryInfo.geometryType = geometryNV.geometryType;
    geometryInfo.maxPrimitiveCount = 1024;
    geometryInfo.indexType = geometryNV.geometry.triangles.indexType;
    geometryInfo.maxVertexCount = 1024;
    geometryInfo.vertexFormat = geometryNV.geometry.triangles.vertexFormat;
    geometryInfo.allowsTransforms = VK_TRUE;

    VkAccelerationStructureCreateInfoKHR as_create_info = {};
    as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;

    VkAccelerationStructureKHR as = VK_NULL_HANDLE;

    // Top level can not have geometry
    {
        VkAccelerationStructureCreateInfoKHR bad_top_level_create_info = as_create_info;
        bad_top_level_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        bad_top_level_create_info.maxGeometryCount = 1;
        bad_top_level_create_info.pGeometryInfos = &geometryInfo;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkAccelerationStructureCreateInfoKHR-type-03496");
        vkCreateAccelerationStructureKHR(m_device->handle(), &bad_top_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // If type is VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
    // and compactedSize is 0, maxGeometryCount must be 1
    // also tests If compactedSize is 0 then maxGeometryCount must not be 0
    {
        VkAccelerationStructureCreateInfoKHR bad_top_level_create_info = as_create_info;
        bad_top_level_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        bad_top_level_create_info.maxGeometryCount = 0;
        bad_top_level_create_info.compactedSize = 0;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-compactedSize-02993");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkAccelerationStructureCreateInfoKHR-type-03495");
        vkCreateAccelerationStructureKHR(m_device->handle(), &bad_top_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Bot level can not have instances
    {
        VkAccelerationStructureCreateInfoKHR bad_bot_level_create_info = as_create_info;
        bad_bot_level_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        bad_bot_level_create_info.maxGeometryCount = 1;
        VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo2 = geometryInfo;
        geometryInfo2.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        bad_bot_level_create_info.pGeometryInfos = &geometryInfo2;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkAccelerationStructureCreateInfoKHR-type-03497");
        vkCreateAccelerationStructureKHR(m_device->handle(), &bad_bot_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not prefer both fast trace and fast build
    {
        VkAccelerationStructureCreateInfoKHR bad_flags_level_create_info = as_create_info;
        bad_flags_level_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        bad_flags_level_create_info.maxGeometryCount = 1;
        bad_flags_level_create_info.pGeometryInfos = &geometryInfo;
        bad_flags_level_create_info.flags =
            VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-flags-03499");
        vkCreateAccelerationStructureKHR(m_device->handle(), &bad_flags_level_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not have geometry or instance for compacting
    {
        VkAccelerationStructureCreateInfoKHR bad_compacting_as_create_info = as_create_info;
        bad_compacting_as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        bad_compacting_as_create_info.maxGeometryCount = 1;
        bad_compacting_as_create_info.pGeometryInfos = &geometryInfo;
        bad_compacting_as_create_info.flags = 0;
        bad_compacting_as_create_info.compactedSize = 1024;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-compactedSize-03490");
        vkCreateAccelerationStructureKHR(m_device->handle(), &bad_compacting_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // Can not mix different geometry types into single bottom level acceleration structure
    {
        VkAccelerationStructureCreateGeometryTypeInfoKHR aabb_geometry = {};
        aabb_geometry = geometryInfo;
        aabb_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;

        std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> geometries = {geometryInfo, aabb_geometry};

        VkAccelerationStructureCreateInfoKHR mix_geometry_types_as_create_info = as_create_info;
        mix_geometry_types_as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        mix_geometry_types_as_create_info.maxGeometryCount = static_cast<uint32_t>(geometries.size());
        mix_geometry_types_as_create_info.pGeometryInfos = geometries.data();
        mix_geometry_types_as_create_info.flags = 0;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkAccelerationStructureCreateInfoKHR-type-03498");
        vkCreateAccelerationStructureKHR(m_device->handle(), &mix_geometry_types_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
    // If geometryType is VK_GEOMETRY_TYPE_TRIANGLES_KHR, indexType must be
    // VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or VK_INDEX_TYPE_NONE_KHR
    {
        VkAccelerationStructureCreateGeometryTypeInfoKHR invalid_index = geometryInfo;
        invalid_index.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        invalid_index.indexType = VK_INDEX_TYPE_UINT8_EXT;
        VkAccelerationStructureCreateInfoKHR invalid_index_geometry_types_as_create_info = as_create_info;
        invalid_index_geometry_types_as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        invalid_index_geometry_types_as_create_info.pGeometryInfos = &invalid_index;
        invalid_index_geometry_types_as_create_info.maxGeometryCount = 1;
        invalid_index_geometry_types_as_create_info.flags = 0;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateGeometryTypeInfoKHR-geometryType-03502");
        vkCreateAccelerationStructureKHR(m_device->handle(), &invalid_index_geometry_types_as_create_info, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }

    // flags must be a valid combination of VkBuildAccelerationStructureFlagBitsNV
    {
        VkAccelerationStructureCreateInfoKHR invalid_flag = as_create_info;
        invalid_flag.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        invalid_flag.flags = VK_BUILD_ACCELERATION_STRUCTURE_FLAG_BITS_MAX_ENUM_KHR;
        invalid_flag.pGeometryInfos = &geometryInfo;
        invalid_flag.maxGeometryCount = 1;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkAccelerationStructureCreateInfoKHR-flags-parameter");
        vkCreateAccelerationStructureKHR(m_device->handle(), &invalid_flag, nullptr, &as);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateBindAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure binding.");
    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor)) {
        return;
    }

    PFN_vkBindAccelerationStructureMemoryNV vkBindAccelerationStructureMemoryNV =
        reinterpret_cast<PFN_vkBindAccelerationStructureMemoryNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkBindAccelerationStructureMemoryNV"));
    assert(vkBindAccelerationStructureMemoryNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = {};
    as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    as_create_info.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    as_create_info.info.geometryCount = 1;
    as_create_info.info.pGeometries = &geometry;
    as_create_info.info.instanceCount = 0;

    VkAccelerationStructureObj as(*m_device, as_create_info, false);
    m_errorMonitor->VerifyNotFound();

    VkMemoryRequirements as_memory_requirements = as.memory_requirements().memoryRequirements;

    VkBindAccelerationStructureMemoryInfoNV as_bind_info = {};
    as_bind_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
    as_bind_info.accelerationStructure = as.handle();

    VkMemoryAllocateInfo as_memory_alloc = {};
    as_memory_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoKHR-memory-parameter");
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoKHR-memoryOffset-02594");
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoKHR-memoryOffset-02451");
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

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoKHR-size-02595");
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

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindAccelerationStructureMemoryInfoKHR-memory-02593");
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkBindAccelerationStructureMemoryInfoKHR-accelerationStructure-02450");
        (void)vkBindAccelerationStructureMemoryNV(device(), 1, &as_bind_info_twice_2);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), as_memory_twice_1, NULL);
        vk::FreeMemory(device(), as_memory_twice_2, NULL);
    }
}

TEST_F(VkLayerTest, ValidateWriteDescriptorSetAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");
    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor)) {
        return;
    }

    OneOffDescriptorSet ds(m_device,
                           {
                               {0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
                           });

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;

    VkAccelerationStructureNV badHandle = (VkAccelerationStructureNV)12345678;
    VkWriteDescriptorSetAccelerationStructureKHR acc = {};
    acc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    acc.accelerationStructureCount = 1;
    acc.pAccelerationStructures = &badHandle;
    descriptor_write.pNext = &acc;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-parameter");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    VkAccelerationStructureCreateInfoNV top_level_as_create_info = {};
    top_level_as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    top_level_as_create_info.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    top_level_as_create_info.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
    top_level_as_create_info.info.instanceCount = 1;
    top_level_as_create_info.info.geometryCount = 0;

    VkAccelerationStructureObj top_level_as(*m_device, top_level_as_create_info);

    acc.pAccelerationStructures = &top_level_as.handle();
    m_errorMonitor->ExpectSuccess();
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, ValidateCmdBuildAccelerationStructureNV) {
    TEST_DESCRIPTION("Validate acceleration structure building.");
    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor)) {
        return;
    }

    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBuildAccelerationStructureNV"));
    assert(vkCmdBuildAccelerationStructureNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = {};
    bot_level_as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    bot_level_as_create_info.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
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
    VkBufferCreateInfo too_small_scratch_buffer_info = {};
    too_small_scratch_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
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
                                      bot_level_as_updated.handle(), bot_level_as.handle(), bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Src must have been built before with the VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV flag
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_FALSE,
                                      bot_level_as.handle(), VK_NULL_HANDLE, bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyNotFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBuildAccelerationStructureNV-update-02489");
    vkCmdBuildAccelerationStructureNV(m_commandBuffer->handle(), &bot_level_as_create_info.info, VK_NULL_HANDLE, 0, VK_TRUE,
                                      bot_level_as_updated.handle(), bot_level_as.handle(), bot_level_as_scratch.handle(), 0);
    m_errorMonitor->VerifyFound();

    // invalid scratch buff
    VkBufferObj bot_level_as_invalid_scratch;
    VkBufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
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

TEST_F(VkLayerTest, ValidateGetAccelerationStructureHandleNV) {
    TEST_DESCRIPTION("Validate acceleration structure handle querying.");
    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor)) {
        return;
    }

    PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV =
        reinterpret_cast<PFN_vkGetAccelerationStructureHandleNV>(
            vk::GetDeviceProcAddr(m_device->handle(), "vkGetAccelerationStructureHandleNV"));
    assert(vkGetAccelerationStructureHandleNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV bot_level_as_create_info = {};
    bot_level_as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    bot_level_as_create_info.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
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
    if (!InitFrameworkForRayTracingTest(this, false, m_instance_extension_names, m_device_extension_names, m_errorMonitor)) {
        return;
    }

    PFN_vkCmdCopyAccelerationStructureNV vkCmdCopyAccelerationStructureNV = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureNV>(
        vk::GetDeviceProcAddr(m_device->handle(), "vkCmdCopyAccelerationStructureNV"));
    assert(vkCmdCopyAccelerationStructureNV != nullptr);

    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometry;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometry);

    VkAccelerationStructureCreateInfoNV as_create_info = {};
    as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    as_create_info.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
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

TEST_F(VkLayerTest, QueryPerformanceCreation) {
    TEST_DESCRIPTION("Create performance query without support");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto performance_features = lvl_init_struct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&performance_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performance_features.performanceCounterQueryPools) {
        printf("%s Performance query pools are not supported.\n", kSkipPrefix);
        return;
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
            c.sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
            c.pNext = nullptr;
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;
        break;
    }

    if (counters.empty()) {
        printf("%s No queue reported any performance counter.\n", kSkipPrefix);
        return;
    }

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci{};
    perf_query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counters.size();
    std::vector<uint32_t> counterIndices;
    for (uint32_t c = 0; c < counters.size(); c++) counterIndices.push_back(c);
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
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
    m_errorMonitor->ExpectSuccess(kErrorBit);
    vk::CreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);
    m_errorMonitor->VerifyNotFound();

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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto performanceFeatures = lvl_init_struct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        printf("%s Performance query pools are not supported.\n", kSkipPrefix);
        return;
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performanceFeatures, pool_flags));
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
            c.sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
            c.pNext = nullptr;
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
        printf("%s No queue reported any performance counter with command buffer scope.\n", kSkipPrefix);
        return;
    }

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci{};
    perf_query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_ci.pNext = &perf_query_pool_ci;
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
        VkAcquireProfilingLockInfoKHR lock_info{};
        lock_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR;
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    // Not the first command.
    {
        VkBufferCreateInfo buf_info = {};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
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

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
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
        VkBufferCreateInfo buf_info = {};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        m_commandBuffer->begin();

        m_errorMonitor->ExpectSuccess(kErrorBit);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        m_errorMonitor->VerifyNotFound();

        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);

        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);

        m_commandBuffer->end();

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto performanceFeatures = lvl_init_struct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        printf("%s Performance query pools are not supported.\n", kSkipPrefix);
        return;
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, pool_flags));
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
            c.sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
            c.pNext = nullptr;
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
        printf("%s No queue reported any performance counter with render pass scope.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci{};
    perf_query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_ci.pNext = &perf_query_pool_ci;
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
        VkAcquireProfilingLockInfoKHR lock_info{};
        lock_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR;
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

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto performanceFeatures = lvl_init_struct<VkPhysicalDevicePerformanceQueryFeaturesKHR>();
    features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        printf("%s Performance query pools are not supported.\n", kSkipPrefix);
        return;
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performanceFeatures, pool_flags));
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
            c.sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
            c.pNext = nullptr;
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
        printf("%s No queue reported any performance counter with render pass scope.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci{};
    perf_query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_ci.pNext = &perf_query_pool_ci;
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
        VkAcquireProfilingLockInfoKHR lock_info{};
        lock_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR;
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        m_commandBuffer->end();

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
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
        VkBufferCreateInfo buf_info = {};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
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
            VkAcquireProfilingLockInfoKHR lock_info{};
            lock_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR;
            VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
            ASSERT_TRUE(result == VK_SUCCESS);
        }

        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);

        m_commandBuffer->end();

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        return;
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto hostQueryResetFeatures = lvl_init_struct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    auto performanceFeatures = lvl_init_struct<VkPhysicalDevicePerformanceQueryFeaturesKHR>(&hostQueryResetFeatures);
    features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        printf("%s Performance query pools are not supported.\n", kSkipPrefix);
        return;
    }
    if (!hostQueryResetFeatures.hostQueryReset) {
        printf("%s Missing host query reset.\n", kSkipPrefix);
        return;
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performanceFeatures, pool_flags));
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
            c.sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
            c.pNext = nullptr;
        }
        vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(gpu(), idx, &nCounters, &counters[0], nullptr);
        queueFamilyIndex = idx;

        for (uint32_t counterIdx = 0; counterIdx < counters.size(); counterIdx++) {
            if (counters[counterIdx].scope == VK_QUERY_SCOPE_COMMAND_KHR) counterIndices.push_back(counterIdx);
        }

        VkQueryPoolPerformanceCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
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
        printf("%s No queue reported a set of counters that needs more than one pass.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci{};
    perf_query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_ci.pNext = &perf_query_pool_ci;
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
        VkAcquireProfilingLockInfoKHR lock_info{};
        lock_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR;
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        VkBufferCreateInfo buf_info = {};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);

        m_commandBuffer->begin(&command_buffer_begin_info);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, 4096, 0);
        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
        m_commandBuffer->end();

        // Invalid pass index
        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info{};
            perf_submit_info.sType = VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR;
            perf_submit_info.counterPassIndex = nPasses;
            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pNext = &perf_submit_info;
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
            VkPerformanceQuerySubmitInfoKHR perf_submit_info{};
            perf_submit_info.sType = VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR;
            perf_submit_info.counterPassIndex = passIdx;
            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pNext = &perf_submit_info;
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
                                sizeof(VkPerformanceCounterResultKHR), VK_QUERY_RESULT_WAIT_BIT);
        m_errorMonitor->VerifyFound();

        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info{};
            perf_submit_info.sType = VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR;
            perf_submit_info.counterPassIndex = nPasses - 1;
            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pNext = &perf_submit_info;
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

        // Invalid stride
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03229");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR) + 4, VK_QUERY_RESULT_WAIT_BIT);
        m_errorMonitor->VerifyFound();

        // Invalid flags
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR), VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR), VK_QUERY_RESULT_PARTIAL_BIT);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-03230");
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR), VK_QUERY_RESULT_64_BIT);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->ExpectSuccess(kErrorBit);
        vk::GetQueryPoolResults(device(), query_pool, 0, 1, sizeof(VkPerformanceCounterResultKHR) * results.size(), &results[0],
                                sizeof(VkPerformanceCounterResultKHR), VK_QUERY_RESULT_WAIT_BIT);
        m_errorMonitor->VerifyNotFound();

        vk::DestroyBuffer(device(), buffer, nullptr);
        vk::FreeMemory(device(), mem, NULL);
    }

    vkReleaseProfilingLockKHR(device());

    vk::DestroyQueryPool(m_device->device(), query_pool, NULL);
}

TEST_F(VkLayerTest, QueryPerformanceResetAndBegin) {
    TEST_DESCRIPTION("Verify that we get an error if we reset & begin a performance query within the same primary command buffer.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
        return;
    }
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFeatures2KHR features2 = {};
    auto hostQueryResetFeatures = lvl_init_struct<VkPhysicalDeviceHostQueryResetFeaturesEXT>();
    auto performanceFeatures = lvl_init_struct<VkPhysicalDevicePerformanceQueryFeaturesKHR>(&hostQueryResetFeatures);
    features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&performanceFeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!performanceFeatures.performanceCounterQueryPools) {
        printf("%s Performance query pools are not supported.\n", kSkipPrefix);
        return;
    }
    if (!hostQueryResetFeatures.hostQueryReset) {
        printf("%s Missing host query reset.\n", kSkipPrefix);
        return;
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &performanceFeatures, pool_flags));
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
            c.sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
            c.pNext = nullptr;
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
        printf("%s No queue reported a set of counters that needs more than one pass.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPoolPerformanceCreateInfoKHR perf_query_pool_ci{};
    perf_query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
    perf_query_pool_ci.queueFamilyIndex = queueFamilyIndex;
    perf_query_pool_ci.counterIndexCount = counterIndices.size();
    perf_query_pool_ci.pCounterIndices = &counterIndices[0];
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_ci.pNext = &perf_query_pool_ci;
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
        VkAcquireProfilingLockInfoKHR lock_info{};
        lock_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR;
        VkResult result = vkAcquireProfilingLockKHR(device(), &lock_info);
        ASSERT_TRUE(result == VK_SUCCESS);
    }

    {
        VkBufferCreateInfo buf_info = {};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buf_info.size = 4096;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBuffer buffer;
        VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = 4096;
        VkDeviceMemory mem;
        err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
        ASSERT_VK_SUCCESS(err);
        vk::BindBufferMemory(device(), buffer, mem, 0);

        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBeginQuery-None-02863");

        m_commandBuffer->reset();
        m_commandBuffer->begin(&command_buffer_begin_info);
        vk::CmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);
        vk::CmdEndQuery(m_commandBuffer->handle(), query_pool, 0);
        m_commandBuffer->end();

        {
            VkPerformanceQuerySubmitInfoKHR perf_submit_info{};
            perf_submit_info.sType = VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR;
            perf_submit_info.counterPassIndex = 0;
            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.pNext = &perf_submit_info;
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

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info{};
    semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = &semaphore_type_create_info;

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info[2] = {};
    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].commandBufferCount = 0;
    submit_info[0].pWaitDstStageMask = &stageFlags;
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitSemaphores-03239");
    vk::QueueSubmit(m_device->m_queue, 1, submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info{};
    uint64_t signalValue = 1;
    timeline_semaphore_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    submit_info[0].pNext = &timeline_semaphore_submit_info;

    submit_info[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto timelineproperties = lvl_init_struct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info{};
    semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = &semaphore_type_create_info;

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info = {};
    uint64_t signalValue = 1;
    uint64_t waitValue = 3;
    timeline_semaphore_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info[2] = {};
    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].pNext = &timeline_semaphore_submit_info;
    submit_info[0].pWaitDstStageMask = &stageFlags;
    submit_info[0].signalSemaphoreCount = 1;
    submit_info[0].pSignalSemaphores = &semaphore;

    submit_info[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore[3];
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[0]));
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[1]));
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[2]));

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info[3] = {};
    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    submit_info[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    submit_info[2].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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

    // We need two queues for this
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    VkQueueFamilyProperties *queue_props = new VkQueueFamilyProperties[queue_count];
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props);

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
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = family_index[0];
    queue_info[0].queueCount = queue_count > 1 ? 1 : 2;
    queue_info[0].pQueuePriorities = &(priorities[0]);

    queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[1].queueFamilyIndex = family_index[1];
    queue_info[1].queueCount = queue_count > 1 ? 1 : 2;
    queue_info[1].pQueuePriorities = &(priorities[0]);

    VkDeviceCreateInfo dev_info{};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.queueCreateInfoCount = queue_count > 1 ? 2 : 1;
    dev_info.pQueueCreateInfos = &(queue_info[0]);
    dev_info.enabledLayerCount = 0;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    auto timeline_semaphore_features = lvl_init_struct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    timeline_semaphore_features.timelineSemaphore = true;
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&timeline_semaphore_features);
    dev_info.pNext = &features2;

    VkDevice dev;
    ASSERT_VK_SUCCESS(vk::CreateDevice(gpu(), &dev_info, nullptr, &dev));

    VkQueue queue[2];
    vk::GetDeviceQueue(dev, family_index[0], queue_index[0], &(queue[0]));
    vk::GetDeviceQueue(dev, family_index[1], queue_index[1], &(queue[1]));

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info{};
    semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = &semaphore_type_create_info;

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(dev, &semaphore_create_info, nullptr, &semaphore));

    uint64_t semaphoreValues[] = {10, 100, 0, 10};
    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info{};
    timeline_semaphore_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &(semaphoreValues[0]);
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &(semaphoreValues[1]);

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = &timeline_semaphore_submit_info;
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
    VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    semaphore_create_info.pNext = nullptr;
    semaphore_create_info.flags = 0;
    VkSemaphore import_semaphore;
    VkResult err = vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &import_semaphore);
    ASSERT_VK_SUCCESS(err);

    int fd = 0;
    VkImportSemaphoreFdInfoKHR import_semaphore_fd_info = {VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR};
    import_semaphore_fd_info.pNext = nullptr;
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

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info{};
    semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = &semaphore_type_create_info;

    VkSemaphore semaphore[2];
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &(semaphore[0])));

    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &(semaphore[1])));

    VkSemaphoreWaitInfo semaphore_wait_info{};
    semaphore_wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
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

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    auto timelinefeatures = lvl_init_struct<VkPhysicalDeviceTimelineSemaphoreFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&timelinefeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!timelinefeatures.timelineSemaphore) {
        printf("%s Timeline semaphores are not supported.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));

    VkSemaphoreSignalInfo semaphore_signal_info{};
    semaphore_signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
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

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    auto timelineproperties = lvl_init_struct<VkPhysicalDeviceTimelineSemaphorePropertiesKHR>();
    auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&timelineproperties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    VkSemaphoreTypeCreateInfoKHR semaphore_type_create_info{};
    semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
    semaphore_type_create_info.initialValue = 5;

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = &semaphore_type_create_info;

    VkSemaphore semaphore[2];
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[0]));
    ASSERT_VK_SUCCESS(vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore[1]));

    VkSemaphoreSignalInfo semaphore_signal_info{};
    semaphore_signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    semaphore_signal_info.semaphore = semaphore[0];
    semaphore_signal_info.value = 3;
    auto vkSignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)vk::GetDeviceProcAddr(m_device->device(), "vkSignalSemaphoreKHR");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSemaphoreSignalInfo-value-03258");
    vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info);
    m_errorMonitor->VerifyFound();

    semaphore_signal_info.value = 10;
    ASSERT_VK_SUCCESS(vkSignalSemaphoreKHR(m_device->device(), &semaphore_signal_info));

    VkTimelineSemaphoreSubmitInfoKHR timeline_semaphore_submit_info{};
    uint64_t waitValue = 10;
    uint64_t signalValue = 20;
    timeline_semaphore_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR;
    timeline_semaphore_submit_info.waitSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pWaitSemaphoreValues = &waitValue;
    timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
    timeline_semaphore_submit_info.pSignalSemaphoreValues = &signalValue;

    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = &timeline_semaphore_submit_info;
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

            VkSemaphoreCreateInfo binary_semaphore_create_info{};
            binary_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

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

TEST_F(VkLayerTest, InvalidSemaphoreCounterType) {
    TEST_DESCRIPTION("Get payload from a non Timeline Semaphore");

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

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    auto timelinefeatures = lvl_init_struct<VkPhysicalDeviceTimelineSemaphoreFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&timelinefeatures);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!timelinefeatures.timelineSemaphore) {
        printf("%s Timeline semaphores are not supported.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kMockICD)) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
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

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = nullptr;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageFormatProperties2 image_format_prop = {};
    image_format_prop.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    VkPhysicalDeviceImageFormatInfo2 image_format_info = {};
    image_format_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    image_format_info.format = image_info.format;
    image_format_info.tiling = image_info.tiling;
    image_format_info.type = image_info.imageType;
    image_format_info.usage = image_info.usage;
    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_mod_info = {};
    drm_format_mod_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT;
    drm_format_mod_info.pNext = nullptr;
    drm_format_mod_info.drmFormatModifier = dummy_modifiers[0];
    drm_format_mod_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    drm_format_mod_info.queueFamilyIndexCount = 0;
    image_format_info.pNext = (void *)&drm_format_mod_info;
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info, &image_format_prop);

    VkSubresourceLayout dummyPlaneLayout = {0, 0, 0, 0, 0};

    VkImageDrmFormatModifierListCreateInfoEXT drm_format_mod_list = {};
    drm_format_mod_list.sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT;
    drm_format_mod_list.pNext = nullptr;
    drm_format_mod_list.drmFormatModifierCount = 2;
    drm_format_mod_list.pDrmFormatModifiers = dummy_modifiers;

    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_mod_explicit = {};
    drm_format_mod_explicit.sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
    drm_format_mod_explicit.pNext = nullptr;
    drm_format_mod_explicit.drmFormatModifierPlaneCount = 1;
    drm_format_mod_explicit.pPlaneLayouts = &dummyPlaneLayout;

    VkImage image = VK_NULL_HANDLE;

    // No pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-tiling-02261");
    vk::CreateImage(device(), &image_info, nullptr, &image);
    m_errorMonitor->VerifyFound();

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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME; skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    VkDevice test_device;
    VkQueue test_queue;
    VkResult result;

    // Use the first Physical device and queue family
    // Makes test more portable as every driver has at least 1 queue with a queueCount of 1
    uint32_t queue_family_count = 1;
    uint32_t queue_family_index = 0;
    VkQueueFamilyProperties queue_properties;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_family_count, &queue_properties);

    float queue_priority = 1.0;
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.flags = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
    queue_create_info.pNext = nullptr;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceProtectedMemoryFeatures protect_features = {};
    protect_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES;
    protect_features.pNext = nullptr;
    protect_features.protectedMemory = VK_FALSE;  // Starting with it off

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &protect_features;
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

    VkPhysicalDeviceFeatures2 features2;
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &protect_features;
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (protect_features.protectedMemory == VK_TRUE) {
        result = vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
        if (result != VK_SUCCESS) {
            printf("%s CreateDevice returned back %s, skipping rest of tests\n", kSkipPrefix, string_VkResult(result));
            return;
        }

        // TODO: Re-enable test when Vulkan-Loader issue #384 is resolved and upstream
        // Try using GetDeviceQueue with a queue that has as flag
        // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-flags-01841");
        // vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);
        // m_errorMonitor->VerifyFound();

        vk::DestroyDevice(test_device, nullptr);
    }

    // Create device without protected queue
    protect_features.protectedMemory = VK_FALSE;
    queue_create_info.flags = 0;
    result = vk::CreateDevice(gpu(), &device_create_info, nullptr, &test_device);
    if (result != VK_SUCCESS) {
        printf("%s CreateDevice returned back %s, skipping rest of tests\n", kSkipPrefix, string_VkResult(result));
        return;
    }

    // TODO: Re-enable test when Vulkan-Loader issue #384 is resolved and upstream
    // Set queueIndex 1 over size of queueCount
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-queueIndex-00385");
    // vk::GetDeviceQueue(test_device, queue_family_index, queue_properties.queueCount, &test_queue);
    // m_errorMonitor->VerifyFound();

    // Use an unknown queue family index
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceQueue-queueFamilyIndex-00384");
    // vk::GetDeviceQueue(test_device, queue_family_index + 1, 0, &test_queue);
    // m_errorMonitor->VerifyFound();

    // Sanity check can still get the queue
    m_errorMonitor->ExpectSuccess();
    vk::GetDeviceQueue(test_device, queue_family_index, 0, &test_queue);
    m_errorMonitor->VerifyNotFound();

    vk::DestroyDevice(test_device, nullptr);
}

TEST_F(VkLayerTest, DisabledProtectedMemory) {
    TEST_DESCRIPTION("Validate cases where protectedMemory feature is not enabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s test requires Vulkan 1.1 extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto protected_memory_features = lvl_init_struct<VkPhysicalDeviceProtectedMemoryFeatures>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&protected_memory_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    // Set false to trigger VUs
    protected_memory_features.protectedMemory = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = {};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandPoolCreateInfo-flags-02860");
    vk::CreateCommandPool(device(), &pool_create_info, nullptr, &command_pool);
    m_errorMonitor->VerifyFound();

    VkBuffer buffer;
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = nullptr;
    buffer_create_info.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    buffer_create_info.size = 4096;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferCreateInfo-flags-01887");
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer);
    m_errorMonitor->VerifyFound();

    VkImage image;
    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
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
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
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

    VkProtectedSubmitInfo protected_submit_info = {};
    protected_submit_info.sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
    protected_submit_info.pNext = nullptr;
    protected_submit_info.protectedSubmit = VK_TRUE;

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = &protected_submit_info;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    m_commandBuffer->begin();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkProtectedSubmitInfo-protectedSubmit-01816");
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

    auto protected_memory_features = lvl_init_struct<VkPhysicalDeviceProtectedMemoryFeatures>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&protected_memory_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (protected_memory_features.protectedMemory == VK_FALSE) {
        printf("%s protectedMemory feature not supported, skipped.\n", kSkipPrefix);
        return;
    };

    // Turns m_commandBuffer into a protected command buffer
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    bool sparse_support = (m_device->phy().features().sparseBinding == VK_TRUE);

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    VkBuffer buffer_protected = VK_NULL_HANDLE;
    VkBuffer buffer_unprotected = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = nullptr;
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
    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
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

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
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
    if (!InitFrameworkForRayTracingTest(this, true, m_instance_extension_names, m_device_extension_names, m_errorMonitor)) {
        return;
    }
    VkBuffer buffer;
    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = 4096;
    VkDeviceMemory mem;
    err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);
    vk::BindBufferMemory(device(), buffer, mem, 0);

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto ray_tracing_properties = lvl_init_struct<VkPhysicalDeviceRayTracingPropertiesKHR>();
    auto properties2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)vk::GetInstanceProcAddr(instance(), "vkCmdTraceRaysKHR");
    ASSERT_TRUE(vkCmdTraceRaysKHR != nullptr);

    VkStridedBufferRegionKHR stridebufregion = {};
    stridebufregion.buffer = buffer;
    stridebufregion.offset = 0;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleSize;
    stridebufregion.size = buf_info.size;
    // invalid offset
    {
        VkStridedBufferRegionKHR invalid_offset = stridebufregion;
        invalid_offset.offset = ray_tracing_properties.shaderGroupBaseAlignment + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-offset-04038");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_offset, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-offset-04032");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_offset, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-offset-04026");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_offset, &stridebufregion, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-pRayGenShaderBindingTable-04021");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &invalid_offset, &stridebufregion, &stridebufregion, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();
    }

    // Invalid stride multiplier
    {
        VkStridedBufferRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04040");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04034");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysKHR-stride-04028");
        vkCmdTraceRaysKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion, 100,
                          100, 1);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride, greater than maxShaderGroupStride
    {
        VkStridedBufferRegionKHR invalid_stride = stridebufregion;
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
}

TEST_F(VkLayerTest, ValidateCmdTraceRaysIndirectKHR) {
    TEST_DESCRIPTION("Validate vkCmdTraceRaysIndirectKHR.");
    if (!InitFrameworkForRayTracingTest(this, true, m_instance_extension_names, m_device_extension_names, m_errorMonitor, false,
                                        false, true)) {
        return;
    }
    auto ray_tracing_features = lvl_init_struct<VkPhysicalDeviceRayTracingFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (ray_tracing_features.rayTracingIndirectTraceRays == VK_FALSE) {
        printf("%s rayTracingIndirectTraceRays not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ray_tracing_features));
    VkBuffer buffer;
    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 4096;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = 4096;
    VkDeviceMemory mem;
    err = vk::AllocateMemory(device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);
    vk::BindBufferMemory(device(), buffer, mem, 0);

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);

    auto ray_tracing_properties = lvl_init_struct<VkPhysicalDeviceRayTracingPropertiesKHR>();
    auto properties2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&ray_tracing_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    PFN_vkCmdTraceRaysIndirectKHR vkCmdTraceRaysIndirectKHR =
        (PFN_vkCmdTraceRaysIndirectKHR)vk::GetInstanceProcAddr(instance(), "vkCmdTraceRaysIndirectKHR");
    ASSERT_TRUE(vkCmdTraceRaysIndirectKHR != nullptr);

    VkStridedBufferRegionKHR stridebufregion = {};
    stridebufregion.buffer = buffer;
    stridebufregion.offset = 0;
    stridebufregion.stride = ray_tracing_properties.shaderGroupHandleSize;
    stridebufregion.size = buf_info.size;
    // invalid offset
    {
        VkStridedBufferRegionKHR invalid_offset = stridebufregion;
        invalid_offset.offset = ray_tracing_properties.shaderGroupBaseAlignment + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-offset-04038");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_offset,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-offset-04032");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_offset, &stridebufregion,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-offset-04026");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_offset, &stridebufregion, &stridebufregion,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-pRayGenShaderBindingTable-04021");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &invalid_offset, &stridebufregion, &stridebufregion, &stridebufregion,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();
    }

    // Invalid stride multiplier
    {
        VkStridedBufferRegionKHR invalid_stride = stridebufregion;
        invalid_stride.stride = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04040");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04034");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04028");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();
    }
    // Invalid stride, greater than maxShaderGroupStride
    {
        VkStridedBufferRegionKHR invalid_stride = stridebufregion;
        uint32_t align = ray_tracing_properties.shaderGroupHandleSize;
        invalid_stride.stride =
            ray_tracing_properties.maxShaderGroupStride + (align - (ray_tracing_properties.maxShaderGroupStride % align));
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04041");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &stridebufregion, &invalid_stride,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04035");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &stridebufregion, &invalid_stride, &stridebufregion,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdTraceRaysIndirectKHR-stride-04029");
        vkCmdTraceRaysIndirectKHR(m_commandBuffer->handle(), &stridebufregion, &invalid_stride, &stridebufregion, &stridebufregion,
                                  buffer, 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateVkAccelerationStructureVersionKHR) {
    TEST_DESCRIPTION("Validate VkAccelerationStructureVersionKHR.");
    if (!InitFrameworkForRayTracingTest(this, true, m_instance_extension_names, m_device_extension_names, m_errorMonitor, false,
                                        false, true)) {
        return;
    }

    auto ray_tracing_features = lvl_init_struct<VkPhysicalDeviceRayTracingFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&ray_tracing_features);
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (ray_tracing_features.rayTracing == VK_FALSE) {
        printf("%s rayTracing not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ray_tracing_features));
    PFN_vkGetDeviceAccelerationStructureCompatibilityKHR vkGetDeviceAccelerationStructureCompatibilityKHR =
        (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetDeviceAccelerationStructureCompatibilityKHR");
    ASSERT_TRUE(vkGetDeviceAccelerationStructureCompatibilityKHR != nullptr);
    VkAccelerationStructureVersionKHR valid_version = {};
    uint8_t mode[] = {VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR, VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR};
    valid_version.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_VERSION_KHR;
    valid_version.versionData = mode;
    {
        VkAccelerationStructureVersionKHR invalid_version = valid_version;
        invalid_version.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionKHR-sType-sType");
        vkGetDeviceAccelerationStructureCompatibilityKHR(m_device->handle(), &invalid_version);
        m_errorMonitor->VerifyFound();
    }

    {
        VkAccelerationStructureVersionKHR invalid_version = valid_version;
        invalid_version.versionData = NULL;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureVersionKHR-versionData-parameter");
        vkGetDeviceAccelerationStructureCompatibilityKHR(m_device->handle(), &invalid_version);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ValidateCmdBuildAccelerationStructureKHR) {
    TEST_DESCRIPTION("Validate acceleration structure building.");
    if (!InitFrameworkForRayTracingTest(this, true, m_instance_extension_names, m_device_extension_names, m_errorMonitor)) {
        return;
    }

    PFN_vkCmdBuildAccelerationStructureKHR vkCmdBuildAccelerationStructureKHR =
        (PFN_vkCmdBuildAccelerationStructureKHR)vk::GetDeviceProcAddr(device(), "vkCmdBuildAccelerationStructureKHR");

    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");

    assert(vkCmdBuildAccelerationStructureKHR != nullptr);
    VkBufferObj vbo;
    VkBufferObj ibo;
    VkGeometryNV geometryNV;
    GetSimpleGeometryForAccelerationStructureTests(*m_device, &vbo, &ibo, &geometryNV);

    VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo = {};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
    geometryInfo.geometryType = geometryNV.geometryType;
    geometryInfo.maxPrimitiveCount = 1024;
    geometryInfo.indexType = geometryNV.geometry.triangles.indexType;
    geometryInfo.maxVertexCount = 1024;
    geometryInfo.vertexFormat = geometryNV.geometry.triangles.vertexFormat;
    geometryInfo.allowsTransforms = VK_TRUE;

    VkAccelerationStructureCreateInfoKHR bot_level_as_create_info = {};
    bot_level_as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    bot_level_as_create_info.pNext = NULL;
    bot_level_as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    bot_level_as_create_info.maxGeometryCount = 1;
    bot_level_as_create_info.pGeometryInfos = &geometryInfo;
    VkAccelerationStructureObj bot_level_as(*m_device, bot_level_as_create_info);

    VkBufferDeviceAddressInfo vertexAddressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                   geometryNV.geometry.triangles.vertexData};
    VkDeviceAddress vertexAddress = vkGetBufferDeviceAddressKHR(m_device->handle(), &vertexAddressInfo);

    VkBufferDeviceAddressInfo indexAddressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL,
                                                  geometryNV.geometry.triangles.indexData};
    VkDeviceAddress indexAddress = vkGetBufferDeviceAddressKHR(m_device->handle(), &indexAddressInfo);

    VkAccelerationStructureGeometryKHR valid_geometry_triangles = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
    valid_geometry_triangles.geometryType = geometryNV.geometryType;
    valid_geometry_triangles.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    valid_geometry_triangles.geometry.triangles.pNext = NULL;
    valid_geometry_triangles.geometry.triangles.vertexFormat = geometryNV.geometry.triangles.vertexFormat;
    valid_geometry_triangles.geometry.triangles.vertexData.deviceAddress = vertexAddress;
    valid_geometry_triangles.geometry.triangles.vertexStride = 8;
    valid_geometry_triangles.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    valid_geometry_triangles.geometry.triangles.indexData.deviceAddress = indexAddress;
    valid_geometry_triangles.geometry.triangles.transformData.deviceAddress = 0;
    valid_geometry_triangles.flags = 0;

    VkAccelerationStructureGeometryKHR *pGeometry_triangles = &valid_geometry_triangles;
    VkAccelerationStructureBuildGeometryInfoKHR valid_asInfo_triangles = {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
    valid_asInfo_triangles.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    valid_asInfo_triangles.flags = 0;
    valid_asInfo_triangles.update = VK_FALSE;
    valid_asInfo_triangles.srcAccelerationStructure = VK_NULL_HANDLE;
    valid_asInfo_triangles.dstAccelerationStructure = bot_level_as.handle();
    valid_asInfo_triangles.geometryArrayOfPointers = VK_FALSE;
    valid_asInfo_triangles.geometryCount = 1;
    valid_asInfo_triangles.ppGeometries = &pGeometry_triangles;
    valid_asInfo_triangles.scratchData.deviceAddress = 0;

    VkAccelerationStructureBuildOffsetInfoKHR buildOffsetInfo = {
        1,
        0,
        0,
        0,
    };
    const VkAccelerationStructureBuildOffsetInfoKHR *pBuildOffsetInfo = &buildOffsetInfo;
    m_commandBuffer->begin();

    // build  valid src acc  for update VK_TRUE case with VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR set
    VkAccelerationStructureBuildGeometryInfoKHR valid_src_asInfo_triangles = valid_asInfo_triangles;
    valid_src_asInfo_triangles.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    valid_src_asInfo_triangles.srcAccelerationStructure = bot_level_as.handle();
    valid_src_asInfo_triangles.dstAccelerationStructure = bot_level_as.handle();
    vkCmdBuildAccelerationStructureKHR(m_commandBuffer->handle(), 1, &valid_src_asInfo_triangles, &pBuildOffsetInfo);

    // positive test
    {
        VkAccelerationStructureBuildGeometryInfoKHR asInfo_validupdate = valid_asInfo_triangles;
        asInfo_validupdate.update = VK_TRUE;
        asInfo_validupdate.srcAccelerationStructure = bot_level_as.handle();
        m_errorMonitor->ExpectSuccess();
        vkCmdBuildAccelerationStructureKHR(m_commandBuffer->handle(), 1, &asInfo_validupdate, &pBuildOffsetInfo);
        m_errorMonitor->VerifyNotFound();
    }
    // If update is VK_TRUE, srcAccelerationStructure must not be VK_NULL_HANDLE
    {
        VkAccelerationStructureBuildGeometryInfoKHR asInfo_invalidupdate = valid_asInfo_triangles;
        asInfo_invalidupdate.update = VK_TRUE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-update-03537");
        vkCmdBuildAccelerationStructureKHR(m_commandBuffer->handle(), 1, &asInfo_invalidupdate, &pBuildOffsetInfo);
        m_errorMonitor->VerifyFound();
    }

    {
        VkAccelerationStructureBuildGeometryInfoKHR invalid_src_asInfo_triangles = valid_src_asInfo_triangles;
        invalid_src_asInfo_triangles.flags = 0;
        invalid_src_asInfo_triangles.srcAccelerationStructure = bot_level_as.handle();
        invalid_src_asInfo_triangles.dstAccelerationStructure = bot_level_as.handle();

        // build src As without flag VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR set
        vkCmdBuildAccelerationStructureKHR(m_commandBuffer->handle(), 1, &invalid_src_asInfo_triangles, &pBuildOffsetInfo);
        VkAccelerationStructureBuildGeometryInfoKHR asInfo_invalidupdate = valid_asInfo_triangles;

        asInfo_invalidupdate.update = VK_TRUE;
        asInfo_invalidupdate.srcAccelerationStructure = bot_level_as.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureBuildGeometryInfoKHR-update-03538");
        vkCmdBuildAccelerationStructureKHR(m_commandBuffer->handle(), 1, &asInfo_invalidupdate, &pBuildOffsetInfo);
        m_errorMonitor->VerifyFound();
    }
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
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = &external_image_info;
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
    alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer_import.memory_requirements(), mem_flags);
    alloc_info.pNext = &import_info_buffer;
    vk_testing::DeviceMemory memory_buffer_import;
    memory_buffer_import.init(*m_device, alloc_info);

    alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image_import.memory_requirements(), mem_flags);
    alloc_info.pNext = &import_info_image;
    vk_testing::DeviceMemory memory_image_import;
    memory_image_import.init(*m_device, alloc_info);
    m_errorMonitor->VerifyNotFound();

    // Bind imported memory with different handleType
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-02727");
    vk::BindBufferMemory(device(), buffer_import.handle(), memory_buffer_import.handle(), 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = {};
    bind_buffer_info.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
    bind_buffer_info.pNext = nullptr;
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

    VkBindImageMemoryInfo bind_image_info = {};
    bind_image_info.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bind_image_info.pNext = nullptr;
    bind_image_info.image = image_import.handle();
    bind_image_info.memory = memory_buffer_import.handle();
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-memory-02729");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-memory-01614");
    m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-memory-01612");
    vkBindImageMemory2Function(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicStateDisabled) {
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

    auto extended_dynamic_state_features = lvl_init_struct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state_features);
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
    VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
    dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetCullModeEXT-None-03384");
    vkCmdSetCullModeEXT(commandBuffer.handle(), VK_CULL_MODE_NONE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthBoundsTestEnableEXT-None-03349");
    vkCmdSetDepthBoundsTestEnableEXT(commandBuffer.handle(), VK_FALSE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthCompareOpEXT-None-03353");
    vkCmdSetDepthCompareOpEXT(commandBuffer.handle(), VK_COMPARE_OP_NEVER);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthTestEnableEXT-None-03352");
    vkCmdSetDepthTestEnableEXT(commandBuffer.handle(), VK_FALSE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDepthWriteEnableEXT-None-03354");
    vkCmdSetDepthWriteEnableEXT(commandBuffer.handle(), VK_FALSE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetFrontFaceEXT-None-03383");
    vkCmdSetFrontFaceEXT(commandBuffer.handle(), VK_FRONT_FACE_CLOCKWISE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetPrimitiveTopologyEXT-None-03347");
    vkCmdSetPrimitiveTopologyEXT(commandBuffer.handle(), VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCountEXT-None-03396");
    VkRect2D scissor = {{0, 0}, {1, 1}};
    vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetStencilOpEXT-None-03351");
    vkCmdSetStencilOpEXT(commandBuffer.handle(), VK_STENCIL_FACE_BACK_BIT, VK_STENCIL_OP_ZERO, VK_STENCIL_OP_ZERO,
                         VK_STENCIL_OP_ZERO, VK_COMPARE_OP_NEVER);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetStencilTestEnableEXT-None-03350");
    vkCmdSetStencilTestEnableEXT(commandBuffer.handle(), VK_FALSE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCountEXT-None-03393");
    VkViewport viewport = {0, 0, 1, 1, 0.0f, 0.0f};
    vkCmdSetViewportWithCountEXT(commandBuffer.handle(), 1, &viewport);
    m_errorMonitor->VerifyFound();

    commandBuffer.end();
}

TEST_F(VkLayerTest, ValidateExtendedDynamicStateEnabled) {
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

    auto extended_dynamic_state_features = lvl_init_struct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state_features);
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

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Verify viewportCount and scissorCount are specified as zero.
    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        const VkDynamicState dyn_states[] = {
            VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT,
            VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT,
        };
        VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
        dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03379");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03380");
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
        VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
        dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
    VkPipelineDynamicStateCreateInfo dyn_state_ci = {};
    dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2EXT-firstBinding-03355");
    vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), m_device->props.limits.maxVertexInputBindings, 1, buffers.data(),
                               offsets.data(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2EXT-firstBinding-03356");
    vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, m_device->props.limits.maxVertexInputBindings + 1, buffers.data(),
                               offsets.data(), 0, 0);
    m_errorMonitor->VerifyFound();

    {
        VkBufferObj bufferWrongUsage;
        bufferWrongUsage.init(*m_device, 16, 0, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2EXT-pBuffers-03359");
        VkBuffer buffers2[1] = {bufferWrongUsage.handle()};
        VkDeviceSize offsets2[1] = {};
        vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, 0);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2EXT-pBuffers-04111");
        m_errorMonitor->SetUnexpectedError("UNASSIGNED-GeneralParameterError-RequiredParameter");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindVertexBuffers2EXT-pBuffers-parameter");
        VkBuffer buffers2[1] = {VK_NULL_HANDLE};
        VkDeviceSize offsets2[1] = {16};
        VkDeviceSize strides[1] = {m_device->props.limits.maxVertexInputBindingStride + 1ull};
        vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, 0);
        m_errorMonitor->VerifyFound();

        buffers2[0] = buffers[0];
        VkDeviceSize sizes[1] = {16};
        // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2EXT-pBuffers-04112");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2EXT-pOffsets-03357");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2EXT-pSizes-03358");
        vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, sizes, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2EXT-pStrides-03362");
        vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers2, offsets2, 0, strides);
        m_errorMonitor->VerifyFound();
    }

    commandBuffer.BeginRenderPass(m_renderPassBeginInfo);

    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    VkPipelineDynamicStateCreateInfo dyn_state_ci2 = {};
    dyn_state_ci2.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
    VkPipelineDynamicStateCreateInfo dyn_state_ci3 = {};
    dyn_state_ci3.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers2EXT-pStrides-03363");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewportCount-03419");
    vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    VkViewport viewport = {0, 0, 1, 1, 0.0f, 0.0f};
    vkCmdSetViewportWithCountEXT(commandBuffer.handle(), 1, &viewport);
    strides[0] = 4;
    vkCmdBindVertexBuffers2EXT(commandBuffer.handle(), 0, 1, buffers.data(), offsets.data(), 0, strides);

    vkCmdSetPrimitiveTopologyEXT(commandBuffer.handle(), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-primitiveTopology-03420");
    vk::CmdDraw(commandBuffer.handle(), 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndRenderPass(commandBuffer.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCountEXT-viewportCount-03394");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdSetViewportWithCountEXT-viewportCount-arraylength");
    VkViewport viewport2 = {
        0, 0, 1, 1, 0.0f, 0.0f,
    };
    vkCmdSetViewportWithCountEXT(commandBuffer.handle(), 0, &viewport2);
    m_errorMonitor->VerifyFound();

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCountEXT-offset-03400");
        VkRect2D scissor2 = {{1, 0}, {INT32_MAX, 16}};
        vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor2);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCountEXT-offset-03401");
        VkRect2D scissor2 = {{0, 1}, {16, INT32_MAX}};
        vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCountEXT-scissorCount-03397");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdSetScissorWithCountEXT-scissorCount-arraylength");
    vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCountEXT-x-03399");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCountEXT-x-03399");
    VkRect2D scissor3 = {{-1, -1}, {0, 0}};
    vkCmdSetScissorWithCountEXT(commandBuffer.handle(), 1, &scissor3);
    m_errorMonitor->VerifyFound();

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

    auto extended_dynamic_state_features = lvl_init_struct<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2>(&extended_dynamic_state_features);
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetViewportWithCountEXT-viewportCount-03395");
    VkViewport viewport = {0, 0, 1, 1, 0.0f, 0.0f};
    VkViewport viewports[] = {viewport, viewport};
    vkCmdSetViewportWithCountEXT(commandBuffer.handle(), size(viewports), viewports);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetScissorWithCountEXT-scissorCount-03398");
    VkRect2D scissor = {{0, 0}, {1, 1}};
    VkRect2D scissors[] = {scissor, scissor};
    vkCmdSetScissorWithCountEXT(commandBuffer.handle(), size(scissors), scissors);
    m_errorMonitor->VerifyFound();

    commandBuffer.end();
}
