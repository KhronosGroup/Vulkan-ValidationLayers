/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "utils/hash_util.h"
#include "generated/vk_validation_error_messages.h"

static std::string format(const char *message, ...) {
    std::size_t const STRING_BUFFER(4096);

    assert(message != nullptr);
    assert(strlen(message) < STRING_BUFFER);

    char buffer[STRING_BUFFER];
    va_list list;

    va_start(list, message);
    vsnprintf(buffer, STRING_BUFFER, message, list);
    va_end(list);

    return buffer;
}

TEST_F(VkLayerTest, VersionCheckPromotedAPIs) {
    TEST_DESCRIPTION("Validate that promoted APIs are not valid in old versions.");
    SetTargetApiVersion(VK_API_VERSION_1_0);

    RETURN_IF_SKIP(Init());

    // TODO - Currently not working on MockICD with Profiles using 1.0
    // Seems API version is not being passed through correctly
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    const auto vkGetPhysicalDeviceProperties2 =
        GetInstanceProcAddr<PFN_vkGetPhysicalDeviceProperties2>("vkGetPhysicalDeviceProperties2");

    VkPhysicalDeviceProperties2 phys_dev_props_2 = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("UNASSIGNED-API-Version-Violation");
    vkGetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnsupportedPnextApiVersion) {
    TEST_DESCRIPTION("Validate that newer pnext structs are not valid for old Vulkan versions.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    RETURN_IF_SKIP(Init());

    // 1.1 context, VK_KHR_depth_stencil_resolve is NOT enabled, but using its struct is valid
    if (DeviceExtensionSupported(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)) {
        VkPhysicalDeviceDepthStencilResolveProperties unenabled_device_ext_struct = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 phys_dev_props_2 = vku::InitStructHelper(&unenabled_device_ext_struct);
        if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
            vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
        } else {
            m_errorMonitor->SetDesiredError("UNASSIGNED-API-Version-Violation");
            vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
            m_errorMonitor->VerifyFound();
        }
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
    const char *id[] = {"3000300000", "24"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "custom_stype_list", VK_LAYER_SETTING_TYPE_STRING_EXT,
                                       static_cast<uint32_t>(std::size(id)), &id};
    VkLayerSettingsCreateInfoEXT layer_setting_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_setting_create_info));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper(&custom_struct);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);
}

TEST_F(VkLayerTest, CustomStypeStructStringArray) {
    TEST_DESCRIPTION("Positive Test for ability to specify custom pNext structs using a vector of strings");

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
    const std::string string_stype_a = format("%u", custom_stype_a);
    const std::string string_stype_b = format("%u", custom_stype_b);
    const std::string sizeof_struct = format("%d", sizeof(CustomStruct));

    const char *ids[] = {
        string_stype_a.c_str(), sizeof_struct.c_str(),
        string_stype_b.c_str(), sizeof_struct.c_str(),
        string_stype_a.c_str(), sizeof_struct.c_str(),
    };
    const VkLayerSettingEXT setting = {
        OBJECT_LAYER_NAME, "custom_stype_list", VK_LAYER_SETTING_TYPE_STRING_EXT, static_cast<uint32_t>(std::size(ids)), &ids};
    VkLayerSettingsCreateInfoEXT layer_setting_create_info = {
        VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_setting_create_info));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper(&custom_struct_b);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);
}

TEST_F(VkLayerTest, CustomStypeStructIntegerArray) {
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
    const uint32_t ids[] = {
        custom_stype_a, sizeof(CustomStruct),
        custom_stype_b, sizeof(CustomStruct),
        custom_stype_a, sizeof(CustomStruct)
    };

    const VkLayerSettingEXT setting[] = {
        {OBJECT_LAYER_NAME, "custom_stype_list", VK_LAYER_SETTING_TYPE_UINT32_EXT, static_cast<uint32_t>(std::size(ids)), ids}
    };
    VkLayerSettingsCreateInfoEXT layer_setting_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, setting};

    RETURN_IF_SKIP(InitFramework(&layer_setting_create_info));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper(&custom_struct_b);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);
}

TEST_F(VkLayerTest, DuplicateMessageLimit) {
    TEST_DESCRIPTION("Use the duplicate_message_id setting and verify correct operation");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    uint32_t value = 3;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "duplicate_message_limit", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &value};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());

    // Create an invalid pNext structure to trigger the stateless validation warning
    VkBaseOutStructure bogus_struct{};
    bogus_struct.sType = static_cast<VkStructureType>(0x33333333);
    VkPhysicalDeviceProperties2KHR properties2 = vku::InitStructHelper(&bogus_struct);

    // Should get the first three errors just fine
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
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
        const uint32_t hash = hash_util::VuidHash(vuid_spec_text_pair.vuid);
        hashes.push_back(hash);
    }
    std::sort(hashes.begin(), hashes.end());
    const auto it = std::adjacent_find(hashes.begin(), hashes.end());
    ASSERT_TRUE(it == hashes.end());
}

TEST_F(VkLayerTest, VuidHashStability) {
    TEST_DESCRIPTION("Ensure stability of VUID hashes clients rely on for filtering");
    ASSERT_TRUE(hash_util::VuidHash("VUID-VkRenderPassCreateInfo-pNext-01963") == 0xa19880e3);
    ASSERT_TRUE(hash_util::VuidHash("VUID-BaryCoordKHR-BaryCoordKHR-04154") == 0xcc72e520);
    ASSERT_TRUE(hash_util::VuidHash("VUID-FragDepth-FragDepth-04213") == 0x840af838);
    ASSERT_TRUE(hash_util::VuidHash("VUID-RayTmaxKHR-RayTmaxKHR-04349") == 0x8e67514c);
    ASSERT_TRUE(hash_util::VuidHash("VUID-RuntimeSpirv-SubgroupUniformControlFlowKHR-06379") == 0x2f574188);
}

TEST_F(VkLayerTest, VuidIdFilterString) {
    TEST_DESCRIPTION("Validate that message id string filtering is working");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.

    const char *ids[] = {"VUID-VkRenderPassCreateInfo-pNext-01963"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, ids};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));

    RETURN_IF_SKIP(InitState());
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

    const char *ids[] = {"0xa19880e3"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, ids};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));

    RETURN_IF_SKIP(InitState());
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

    const char *ids[] = {"2711126243"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, ids};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());
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

TEST_F(VkLayerTest, RequiredParameter) {
    TEST_DESCRIPTION("Specify VK_NULL_HANDLE, NULL, and 0 for required handle, pointer, array, and array count parameters");

    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceFeatures-pFeatures-parameter");
    // Specify NULL for a pointer to a handle
    // Expected to trigger an error with
    // StatelessValidation::ValidateRequiredPointer
    vk::GetPhysicalDeviceFeatures(gpu(), NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceQueueFamilyProperties-pQueueFamilyPropertyCount-parameter");
    // Specify NULL for pointer to array count
    // Expected to trigger an error with StatelessValidation::ValidateArray
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), NULL, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetViewport-viewportCount-arraylength");
    // Specify 0 for a required array count
    // Expected to trigger an error with StatelessValidation::ValidateArray
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    vk::CmdSetViewport(m_command_buffer.handle(), 0, 0, &viewport);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetViewport-pViewports-parameter");
    // Specify NULL for a required array
    // Expected to trigger an error with StatelessValidation::ValidateArray
    vk::CmdSetViewport(m_command_buffer.handle(), 0, 1, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("UNASSIGNED-GeneralParameterError-RequiredHandle");
    // Specify VK_NULL_HANDLE for a required handle
    // Expected to trigger an error with
    // StatelessValidation::ValidateRequiredHandle
    vk::UnmapMemory(device(), VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("UNASSIGNED-GeneralParameterError-RequiredHandleArray");
    // Specify VK_NULL_HANDLE for a required handle array entry
    // Expected to trigger an error with
    // StatelessValidation::ValidateRequiredHandleArray
    VkFence fence = VK_NULL_HANDLE;
    vk::ResetFences(device(), 1, &fence);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkAllocateMemory-pAllocateInfo-parameter");
    // Specify NULL for a required struct pointer
    // Expected to trigger an error with
    // StatelessValidation::ValidateStructType
    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), NULL, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetStencilReference-faceMask-requiredbitmask");
    // Specify 0 for a required VkFlags parameter
    // Expected to trigger an error with StatelessValidation::ValidateFlags
    vk::CmdSetStencilReference(m_command_buffer.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-sType-sType");
    // Set a bogus sType and see what happens
    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submitInfo = vku::InitStructHelper();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.pWaitDstStageMask = &stageFlags;
    submitInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitSemaphores-parameter");
    stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    // Set a null pointer for pWaitSemaphores
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = &stageFlags;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitDstStageMask-parameter");
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.pWaitDstStageMask = nullptr;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SpecLinks) {
    TEST_DESCRIPTION("Test that spec links in a typical error message are well-formed");
    AddOptionalExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

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

    m_errorMonitor->SetDesiredError(spec_version.c_str());
    vk::GetPhysicalDeviceFeatures(gpu(), NULL);
    m_errorMonitor->VerifyFound();

    // Now generate a 'default' message and check the link
    bool ycbcr_support =
        (IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));
    bool maintenance2_support =
        (IsExtensionsEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));

    if (!((m_device->FormatFeaturesOptimal(VK_FORMAT_R8_UINT) & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) &&
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
    vkt::Image mutImage(*m_device, imageInfo, vkt::set_layout);

    VkImageViewCreateInfo imgViewInfo = vku::InitStructHelper();
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

TEST_F(VkLayerTest, DeviceIDPropertiesUnsupported) {
    TEST_DESCRIPTION("VkPhysicalDeviceIDProperties cannot be used without extensions in 1.0");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        GTEST_SKIP() << "Test's for 1.0 only";
    }

    VkPhysicalDeviceIDProperties id_props = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&id_props);
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2KHR(gpu(), &props2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UsePnextOnlyStructWithoutExtensionEnabled) {
    TEST_DESCRIPTION(
        "Validate that using VkPipelineTessellationDomainOriginStateCreateInfo in VkPipelineTessellationStateCreateInfo.pNext "
        "in a 1.0 context will generate an error message.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredFeature(vkt::Feature::tessellationShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

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
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    // one for each struct
    m_errorMonitor->SetDesiredError("VUID-VkPipelineTessellationStateCreateInfo-pNext-pNext");
    m_errorMonitor->SetDesiredError("VUID-VkPipelineTessellationStateCreateInfo-pNext-pNext");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PnextOnlyStructValidation) {
    TEST_DESCRIPTION("See if checks occur on structs ONLY used in pnext chains.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    // Create a device passing in a bad PdevFeatures2 value
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(indexing_features);
    // Set one of the features values to an invalid boolean value
    indexing_features.descriptorBindingUniformBufferUpdateAfterBind = 800;

    uint32_t queue_node_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, NULL);
    std::vector<VkQueueFamilyProperties> queue_props;
    queue_props.resize(queue_node_count);
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, queue_props.data());
    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];
    VkDeviceCreateInfo dev_info = vku::InitStructHelper();
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();
    dev_info.pNext = &features2;
    VkDevice dev;
    m_errorMonitor->SetDesiredError("is neither VK_TRUE nor VK_FALSE");
    m_errorMonitor->SetUnexpectedError("Failed to create");
    vk::CreateDevice(gpu(), &dev_info, NULL, &dev);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ReservedParameter) {
    TEST_DESCRIPTION("Specify a non-zero value for a reserved parameter");

    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError(" must be 0");
    // Specify 0 for a reserved VkFlags parameter
    // Expected to trigger an error with
    // StatelessValidation::ValidateReservedFlags
    VkSemaphore sem_handle = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo sem_info = vku::InitStructHelper();
    sem_info.flags = 1;
    vk::CreateSemaphore(device(), &sem_info, NULL, &sem_handle);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidStructSType) {
    TEST_DESCRIPTION("Specify an invalid VkStructureType for a Vulkan structure's sType field");

    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-VkMemoryAllocateInfo-sType-sType");
    // Zero struct memory, effectively setting sType to
    // VK_STRUCTURE_TYPE_APPLICATION_INFO
    // Expected to trigger an error with
    // StatelessValidation::ValidateStructType
    VkMemoryAllocateInfo alloc_info = {};
    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &alloc_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-sType-sType");
    // Zero struct memory, effectively setting sType to
    // VK_STRUCTURE_TYPE_APPLICATION_INFO
    // Expected to trigger an error with
    // StatelessValidation::ValidateStructTypeArray
    VkSubmitInfo submit_info = {};
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidStructPNext) {
    TEST_DESCRIPTION("Specify an invalid value for a Vulkan structure's pNext field");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-VkCommandPoolCreateInfo-pNext-pNext");
    // Set VkCommandPoolCreateInfo::pNext to a non-NULL value, when pNext must be NULL.
    // Need to pick a function that has no allowed pNext structure types.
    // Expected to trigger an error with StatelessValidation::ValidateStructPnext
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo pool_ci = vku::InitStructHelper();
    VkApplicationInfo app_info = vku::InitStructHelper();
    pool_ci.pNext = &app_info;
    vk::CreateCommandPool(device(), &pool_ci, NULL, &pool);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError(" chain includes a structure with unexpected VkStructureType ");
    // Set VkMemoryAllocateInfo::pNext to a non-NULL value, but use
    // a function that has allowed pNext structure types and specify
    // a structure type that is not allowed.
    // Expected to trigger an error with StatelessValidation::ValidateStructPnext
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkMemoryAllocateInfo memory_alloc_info = vku::InitStructHelper(&app_info);
    vk::AllocateMemory(device(), &memory_alloc_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError(" chain includes a structure with unexpected VkStructureType ");
    // Same concept as above, but unlike vkAllocateMemory where VkMemoryAllocateInfo is a const
    // in vkGetPhysicalDeviceProperties2, VkPhysicalDeviceProperties2 is not a const
    VkPhysicalDeviceProperties2 physical_device_properties2 = vku::InitStructHelper(&app_info);

    vk::GetPhysicalDeviceProperties2KHR(gpu(), &physical_device_properties2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedEnumOutOfRange) {
    RETURN_IF_SKIP(Init());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance5 is supported";
    }

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceFormatProperties-format-parameter");
    // Specify an invalid VkFormat value
    // Expected to trigger an error with
    // StatelessValidation::ValidateRangedEnum
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), static_cast<VkFormat>(8000), &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedEnumOutOfRange2) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance5 is supported";
    }

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceFormatProperties2-format-parameter");
    VkFormatProperties2 format_properties = vku::InitStructHelper();
    vk::GetPhysicalDeviceFormatProperties2(gpu(), static_cast<VkFormat>(8000), &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedFlagOutOfRange) {
    RETURN_IF_SKIP(Init());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance5 is supported";
    }

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceImageFormatProperties-usage-parameter");
    VkImageFormatProperties format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_1D, VK_IMAGE_TILING_OPTIMAL,
                                               static_cast<VkImageUsageFlags>(0xffffffff), 0, &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedFlagOutOfRange2) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance5 is supported";
    }

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceImageFormatInfo2-usage-parameter");
    VkImageFormatProperties2 format_properties = vku::InitStructHelper();
    VkPhysicalDeviceImageFormatInfo2 format_info = vku::InitStructHelper();
    format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    format_info.flags = 0;
    format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    format_info.type = VK_IMAGE_TYPE_1D;
    format_info.usage = static_cast<VkImageUsageFlags>(0xffffffff);
    vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &format_info, &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadMask) {
    RETURN_IF_SKIP(Init());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance5 is supported";
    }

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceImageFormatProperties-usage-parameter");
    // Specify an invalid VkFlags bitmask value
    // Expected to trigger an error with StatelessValidation::ValidateFlags
    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                               static_cast<VkImageUsageFlags>(1 << 25), 0, &image_format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadFlag) {
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitDstStageMask-parameter");
    // Specify an invalid VkFlags array entry
    // Expected to trigger an error with StatelessValidation::ValidateFlagsArray
    vkt::Semaphore semaphore(*m_device);
    // `stage_flags` is set to a value which, currently, is not a defined stage flag
    // `VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM` works well for this
    VkPipelineStageFlags stage_flags = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    // `waitSemaphoreCount` *must* be greater than 0 to perform this check
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &stage_flags;
    vk::QueueSubmit(m_default_queue->handle(), 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadBool) {
    // Make sure using VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE doesn't trigger a false positive.
    AddRequiredExtensions(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    // Specify an invalid VkBool32 value, expecting a warning with StatelessValidation::ValidateBool32
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

    // Not VK_TRUE or VK_FALSE
    sampler_info.anisotropyEnable = 3;
    CreateSamplerTest(*this, &sampler_info, "UNASSIGNED-GeneralParameterError-UnrecognizedBool32");
}

TEST_F(VkLayerTest, UnrecognizedValueMaxEnum) {
    RETURN_IF_SKIP(Init());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance5 is supported";
    }

    // Specify MAX_ENUM
    VkFormatProperties format_properties;
    m_errorMonitor->SetDesiredError("does not fall within the begin..end range");
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_MAX_ENUM, &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UseObjectWithWrongDevice) {
    TEST_DESCRIPTION(
        "Try to destroy a render pass object using a device other than the one it was created on. This should generate a distinct "
        "error from the invalid handle error.");
    // Create first device and renderpass
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Create second device
    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    auto features = m_device->phy().features();
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_info;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.pEnabledFeatures = &features;

    VkDevice second_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(gpu(), &device_create_info, NULL, &second_device));

    // Try to destroy the renderpass from the first device using the second device
    m_errorMonitor->SetDesiredError("VUID-vkDestroyRenderPass-renderPass-parent");
    vk::DestroyRenderPass(second_device, m_renderPass, NULL);
    m_errorMonitor->VerifyFound();

    vk::DestroyDevice(second_device, NULL);
}

TEST_F(VkLayerTest, InvalidAllocationCallbacks) {
    TEST_DESCRIPTION("Test with invalid VkAllocationCallbacks");

    RETURN_IF_SKIP(Init());

    const std::optional queueFamilyIndex = DeviceObj()->QueueFamily(VK_QUEUE_GRAPHICS_BIT);
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
        m_errorMonitor->SetDesiredError("VUID-VkAllocationCallbacks-pfnAllocation-00632");
        const VkAllocationCallbacks allocator = {nullptr, nullptr, Alloc::realloc, Alloc::free, nullptr, nullptr};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkAllocationCallbacks-pfnReallocation-00633");
        const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, nullptr, Alloc::free, nullptr, nullptr};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkAllocationCallbacks-pfnFree-00634");
        const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, Alloc::realloc, nullptr, nullptr, nullptr};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkAllocationCallbacks-pfnInternalAllocation-00635");
        const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, Alloc::realloc, Alloc::free, nullptr, Alloc::internalFree};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredError("VUID-VkAllocationCallbacks-pfnInternalAllocation-00635");
        const VkAllocationCallbacks allocator = {nullptr, Alloc::alloc, Alloc::realloc, Alloc::free, Alloc::internalAlloc, nullptr};
        vk::CreateCommandPool(device(), &cpci, &allocator, &cmdPool);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, DeviceFeature2AndVertexAttributeDivisorExtensionUnenabled) {
    TEST_DESCRIPTION(
        "Test unenabled VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME & "
        "VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME.");

    VkPhysicalDeviceFeatures2 pd_features2 = vku::InitStructHelper();

    RETURN_IF_SKIP(Init());
    vkt::QueueCreateInfoArray queue_info(m_device->phy().queue_properties_);
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&pd_features2);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    VkDevice testDevice;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = vku::InitStructHelper();
    device_create_info.pNext = &vadf;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
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
    RETURN_IF_SKIP(Init());

    VkPhysicalDevice16BitStorageFeatures sixteen_bit = vku::InitStructHelper();
    sixteen_bit.storageBuffer16BitAccess = true;
    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper(&sixteen_bit);
    features11.storageBuffer16BitAccess = true;

    VkPhysicalDevice8BitStorageFeatures eight_bit = vku::InitStructHelper(&features11);
    eight_bit.storageBuffer8BitAccess = true;
    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper(&eight_bit);
    features12.storageBuffer8BitAccess = true;

    VkPhysicalDeviceVulkan13Features features13 = {};
    VkPhysicalDeviceDynamicRenderingFeatures dyn_rendering_features = {};
    if (DeviceValidationVersion() >= VK_API_VERSION_1_3) {
        dyn_rendering_features = vku::InitStructHelper();
        dyn_rendering_features.dynamicRendering = true;
        dyn_rendering_features.pNext = &eight_bit;
        features13 = vku::InitStructHelper(&dyn_rendering_features);
        features13.dynamicRendering = true;
        features12.pNext = &features13;
        m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-06532");
    }

    vkt::PhysicalDevice physical_device(gpu());
    vkt::QueueCreateInfoArray queue_info(physical_device.queue_properties_);
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&features12);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    VkDevice testDevice;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-02829");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-02830");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RequiredPromotedFeaturesExtensions) {
    TEST_DESCRIPTION("Checks that features are enabled if extension is passed in for promoted extensions with requirement.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    const bool test_1_2 = (DeviceValidationVersion() >= VK_API_VERSION_1_2);

    vkt::PhysicalDevice physical_device(gpu());
    vkt::QueueCreateInfoArray queue_info(physical_device.queue_properties_);
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    // Explicity set all tested features to false
    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    features12.drawIndirectCount = VK_FALSE;
    features12.samplerMirrorClampToEdge = VK_FALSE;
    features12.descriptorIndexing = VK_FALSE;
    features12.samplerFilterMinmax = VK_FALSE;
    features12.shaderOutputViewportIndex = VK_FALSE;
    features12.shaderOutputLayer = VK_TRUE;  // Set true since both shader_viewport features need to true

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    features11.pNext = (test_1_2 == true) ? &features12 : nullptr;
    features11.shaderDrawParameters = VK_FALSE;

    std::vector<const char *> device_extensions;

    // Go through each extension and if supported add to list and add failure to check for
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME)) {
        device_extensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
        m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-04476");
    }
    if (test_1_2 == true) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)) {
            device_extensions.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
            m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02831");
        }
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME)) {
            device_extensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
            m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02832");
        }
        if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)) {
            device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
            device_extensions.push_back(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
            m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02833");
        }
        if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)) {
            device_extensions.push_back(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
            m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02834");
        }
        if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)) {
            device_extensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
            m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02835");
        }
    } else {
        // VkPhysicalDeviceVulkan11Features was not added until Vulkan 1.2
        m_errorMonitor->SetUnexpectedError("VUID-VkDeviceCreateInfo-pNext-pNext");
    }

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&features11);
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

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    std::vector<const char *> device_extensions;
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        device_extensions.push_back(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
    }

    // Create a device that enables variablePointers but not variablePointersStorageBuffer
    VkPhysicalDeviceVariablePointersFeatures variable_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(variable_features);
    if (variable_features.variablePointers == VK_FALSE) {
        GTEST_SKIP() << "variablePointer feature not supported";
    }

    variable_features.variablePointersStorageBuffer = VK_FALSE;

    vkt::PhysicalDevice physical_device(gpu());
    vkt::QueueCreateInfoArray queue_info(physical_device.queue_properties_);
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t i = 0; i < queue_info.size(); ++i) {
        if (qci[i].queueCount) {
            create_queue_infos.push_back(qci[i]);
        }
    }

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&features2);
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.enabledExtensionCount = device_extensions.size();
    VkDevice testDevice;

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceVariablePointersFeatures-variablePointers-01431");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidationCacheTestBadMerge) {
    AddRequiredExtensions(VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkValidationCacheCreateInfoEXT validationCacheCreateInfo = vku::InitStructHelper();
    validationCacheCreateInfo.initialDataSize = 0;
    validationCacheCreateInfo.pInitialData = NULL;
    validationCacheCreateInfo.flags = 0;
    VkValidationCacheEXT validationCache = VK_NULL_HANDLE;
    VkResult res = vk::CreateValidationCacheEXT(device(), &validationCacheCreateInfo, nullptr, &validationCache);
    ASSERT_EQ(VK_SUCCESS, res);

    m_errorMonitor->SetDesiredError("VUID-vkMergeValidationCachesEXT-dstCache-01536");
    res = vk::MergeValidationCachesEXT(device(), validationCache, 1, &validationCache);
    m_errorMonitor->VerifyFound();

    vk::DestroyValidationCacheEXT(device(), validationCache, nullptr);
}

TEST_F(VkLayerTest, UnclosedAndDuplicateQueries) {
    TEST_DESCRIPTION("End a command buffer with a query still in progress, create nested queries.");

    RETURN_IF_SKIP(Init());

    VkQueue queue = VK_NULL_HANDLE;
    vk::GetDeviceQueue(device(), m_device->graphics_queue_node_index_, 0, &queue);

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 5);
    m_command_buffer.begin();
    vk::CmdResetQueryPool(m_command_buffer.handle(), query_pool.handle(), 0, 5);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryPool-01922");
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 1, 0);
    // Attempt to begin a query that has the same type as an active query
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 3, 0);
    vk::CmdEndQuery(m_command_buffer.handle(), query_pool.handle(), 1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-commandBuffer-00061");
    vk::CmdBeginQuery(m_command_buffer.handle(), query_pool.handle(), 0, 0);
    vk::EndCommandBuffer(m_command_buffer.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ExecuteUnrecordedCB) {
    TEST_DESCRIPTION("Attempt vkQueueSubmit with a CB in the initial state");

    RETURN_IF_SKIP(Init());
    // never record m_command_buffer

    m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00070");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();

    // Testing an "unfinished secondary CB" crashes on some HW/drivers (notably Pixel 3 and RADV)
    // vkt::CommandBuffer cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    // m_command_buffer.begin();
    // vk::CmdExecuteCommands(m_command_buffer.handle(), 1u, &cb.handle());
    // m_command_buffer.end();

    // m_errorMonitor->SetDesiredError("VUID-vkQueueSubmit-pCommandBuffers-00072");
    // vk::QueueSubmit(m_default_queue->handle(), 1, &si, VK_NULL_HANDLE);
    // m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Maintenance1AndNegativeViewport) {
    TEST_DESCRIPTION("Attempt to enable AMD_negative_viewport_height and Maintenance1_KHR extension simultaneously");
    RETURN_IF_SKIP(Init());
    if (!((DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) &&
          (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME)))) {
        GTEST_SKIP() << "Maintenance1 and AMD_negative viewport height extensions not supported";
    }

    vkt::QueueCreateInfoArray queue_info(m_device->phy().queue_properties_);
    const char *extension_names[2] = {"VK_KHR_maintenance1", "VK_AMD_negative_viewport_height"};
    VkDevice testDevice;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    auto features = m_device->phy().features();
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.enabledExtensionCount = 2;
    device_create_info.ppEnabledExtensionNames = (const char *const *)extension_names;
    device_create_info.pEnabledFeatures = &features;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374");
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
    RETURN_IF_SKIP(InitFramework());

    vkt::PhysicalDevice physical_device(gpu_);
    VkPhysicalDeviceFeatures features = physical_device.features();
    vkt::QueueCreateInfoArray queue_info(physical_device.queue_properties_);
    const char *extension_names[1] = {VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME};
    VkDevice testDevice;
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = (const char *const *)extension_names;
    device_create_info.pEnabledFeatures = &features;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-01840");
    // The following unexpected error is coming from the LunarG loader. Do not make it a desired message because platforms that do
    // not use the LunarG loader (e.g. Android) will not see the message and the test will fail.
    m_errorMonitor->SetUnexpectedError("Failed to create device chain.");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateArrayLength) {
    TEST_DESCRIPTION("Validate arraylength VUs");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Used to have a valid pointed to set object too
    VkCommandBuffer unused_command_buffer;
    VkDescriptorSet unused_descriptor_set;

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Fence fence(*m_device);
    vkt::Event event(*m_device);

    m_errorMonitor->SetDesiredError("VUID-vkAllocateCommandBuffers-pAllocateInfo::commandBufferCount-arraylength");
    {
        VkCommandBufferAllocateInfo info = vku::InitStructHelper();
        info.commandPool = m_command_pool.handle();
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 0;  // invalid
        vk::AllocateCommandBuffers(device(), &info, &unused_command_buffer);
    }
    m_errorMonitor->VerifyFound();

    // One exception in spec where the size of a field is used in both the function call it and the struct
    m_errorMonitor->SetDesiredError("VUID-vkAllocateDescriptorSets-pAllocateInfo::descriptorSetCount-arraylength");
    m_errorMonitor->SetDesiredError("VUID-VkDescriptorSetAllocateInfo-descriptorSetCount-arraylength");
    {
        VkDescriptorSetAllocateInfo info = vku::InitStructHelper();
        info.descriptorPool = descriptor_set.pool_;
        info.descriptorSetCount = 0;  // invalid
        info.pSetLayouts = &descriptor_set.layout_.handle();
        vk::AllocateDescriptorSets(device(), &info, &unused_descriptor_set);
    }
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkFreeCommandBuffers-commandBufferCount-arraylength");
    vk::FreeCommandBuffers(device(), m_command_pool.handle(), 0, &unused_command_buffer);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkFreeDescriptorSets-descriptorSetCount-arraylength");
    vk::FreeDescriptorSets(device(), descriptor_set.pool_, 0, &descriptor_set.set_);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkResetFences-fenceCount-arraylength");
    vk::ResetFences(device(), 0, &fence.handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkWaitForFences-fenceCount-arraylength");
    vk::WaitForFences(device(), 0, &fence.handle(), true, 1);
    m_errorMonitor->VerifyFound();

    vkt::CommandBuffer command_buffer(*m_device, m_command_pool);
    command_buffer.begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindDescriptorSets-descriptorSetCount-arraylength");
    vk::CmdBindDescriptorSets(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 0,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBufferCount-arraylength");
    vk::CmdExecuteCommands(command_buffer.handle(), 0, &unused_command_buffer);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-eventCount-arraylength");
    vk::CmdWaitEvents(command_buffer.handle(), 0, &event.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                      0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    command_buffer.end();
}

TEST_F(VkLayerTest, DuplicatePhysicalDevices) {
    TEST_DESCRIPTION("Duplicated physical devices in DeviceGroupDeviceCreateInfo.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());
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

    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = 2;
    create_device_pnext.pPhysicalDevices = physicalDevices;

    RETURN_IF_SKIP(InitState());

    vkt::QueueCreateInfoArray queue_info(m_device->phy().queue_properties_);

    VkDeviceCreateInfo create_info = vku::InitStructHelper();
    create_info.pNext = &create_device_pnext;
    create_info.queueCreateInfoCount = queue_info.size();
    create_info.pQueueCreateInfos = queue_info.data();
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = nullptr;
    create_info.enabledExtensionCount = m_device_extension_names.size();
    create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice device;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceGroupDeviceCreateInfo-pPhysicalDevices-00375");
    vk::CreateDevice(gpu(), &create_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCombinationOfDeviceFeatures) {
    TEST_DESCRIPTION("Test invalid combinations of device features.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT shader_image_atomic_int64_feature =
        vku::InitStructHelper();
    shader_image_atomic_int64_feature.sparseImageInt64Atomics = VK_TRUE;
    shader_image_atomic_int64_feature.shaderImageInt64Atomics = VK_FALSE;

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shader_atomic_float_feature =
        vku::InitStructHelper();
    shader_atomic_float_feature.sparseImageFloat32Atomics = VK_TRUE;
    shader_atomic_float_feature.shaderImageFloat32Atomics = VK_FALSE;
    shader_atomic_float_feature.sparseImageFloat32AtomicAdd = VK_TRUE;
    shader_atomic_float_feature.shaderImageFloat32AtomicAdd = VK_FALSE;

    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT shader_atomic_float_feature2 =
        vku::InitStructHelper();
    shader_atomic_float_feature2.sparseImageFloat32AtomicMinMax = VK_TRUE;
    shader_atomic_float_feature2.shaderImageFloat32AtomicMinMax = VK_FALSE;

    VkPhysicalDeviceFeatures2 pd_features2 = vku::InitStructHelper(&shader_image_atomic_int64_feature);

    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    vkt::QueueCreateInfoArray queue_info(m_device->phy().queue_properties_);
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    device_create_info.pNext = &pd_features2;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice testDevice;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-None-04896");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();

    pd_features2.pNext = &shader_atomic_float_feature;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-None-04897");
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-None-04898");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();

    pd_features2.pNext = &shader_atomic_float_feature2;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-sparseImageFloat32AtomicMinMax-04975");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidImageCreateFlagWithPhysicalDeviceCount) {
    TEST_DESCRIPTION("Test for invalid imageCreate flags bit with physicalDeviceCount.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    RETURN_IF_SKIP(InitFramework());

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = 1;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));

    VkImageCreateInfo ici = vku::InitStructHelper();
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
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    features12.bufferDeviceAddress = VK_TRUE;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    const char *enabled_ext = VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&features12);
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_info;
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = &enabled_ext;

    VkDevice testDevice;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-04748");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ZeroBitmask) {
    TEST_DESCRIPTION("Test a reserved flags field set to a non-zero value");

    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-VkSemaphoreCreateInfo-flags-zerobitmask");
    VkSemaphoreCreateInfo semaphore_ci = vku::InitStructHelper();
    semaphore_ci.flags = 1;
    VkSemaphore semaphore = VK_NULL_HANDLE;
    vk::CreateSemaphore(device(), &semaphore_ci, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidExtEnum) {
    TEST_DESCRIPTION("Use an enum from an extension that is not enabled.");
    RETURN_IF_SKIP(Init());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerCreateInfo-magFilter-parameter");
    vkt::Sampler sampler(*m_device, sampler_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ExtensionNotEnabledYCbCr) {
    TEST_DESCRIPTION("Validate that using an API from an unenabled extension returns an error");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    // The feature bit samplerYcbcrConversion prevents the function from being called even in Vulkan 1.0
    m_errorMonitor->SetDesiredError("VUID-vkCreateSamplerYcbcrConversion-None-01648");
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
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

    // VK_KHR_get_physical_device_properties2 promoted to 1.1
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-sType-unique");
    // in VkPhysicalDeviceProperties2 create a chain of pNext of type A -> B -> A
    // Also using different instance of struct to not trip the cycle checkings
    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties_0 =
        vku::InitStructHelper();

    VkPhysicalDeviceIDProperties id_properties = vku::InitStructHelper(&protected_memory_properties_0);

    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties_1 =
        vku::InitStructHelper(&id_properties);

    VkPhysicalDeviceProperties2 physical_device_properties2 =
        vku::InitStructHelper(&protected_memory_properties_1);

    vk::GetPhysicalDeviceProperties2(gpu(), &physical_device_properties2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, GetPhysicalDeviceImageFormatPropertiesFlags) {
    RETURN_IF_SKIP(Init());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance5 is supported";
    }

    VkImageFormatProperties dummy_props;
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceImageFormatProperties-usage-requiredbitmask");
    vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, 0, 0, &dummy_props);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceImageFormatProperties-flags-parameter");
    vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0xBAD00000, &dummy_props);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, GetCalibratedTimestampsDuplicate) {
    TEST_DESCRIPTION("vkGetCalibratedTimestampsEXT with duplicated timeDomain.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    uint32_t count = 0;
    vk::GetPhysicalDeviceCalibrateableTimeDomainsEXT(gpu(), &count, nullptr);
    std::vector<VkTimeDomainEXT> time_domains(count);
    vk::GetPhysicalDeviceCalibrateableTimeDomainsEXT(gpu(), &count, time_domains.data());

    VkCalibratedTimestampInfoEXT timestamp_infos[2];
    timestamp_infos[0] = vku::InitStructHelper();
    timestamp_infos[0].timeDomain = time_domains[0];
    timestamp_infos[1] = vku::InitStructHelper();
    timestamp_infos[1].timeDomain = time_domains[0];

    uint64_t timestamps[2];
    uint64_t max_deviation;
    m_errorMonitor->SetDesiredError("VUID-vkGetCalibratedTimestampsEXT-timeDomain-09246");
    vk::GetCalibratedTimestampsEXT(device(), 2, timestamp_infos, timestamps, &max_deviation);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, GetCalibratedTimestampsDuplicateKHR) {
    TEST_DESCRIPTION("vkGetCalibratedTimestampsKHR with duplicated timeDomain.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    uint32_t count = 0;
    vk::GetPhysicalDeviceCalibrateableTimeDomainsKHR(gpu(), &count, nullptr);
    std::vector<VkTimeDomainEXT> time_domains(count);
    vk::GetPhysicalDeviceCalibrateableTimeDomainsKHR(gpu(), &count, time_domains.data());

    VkCalibratedTimestampInfoEXT timestamp_infos[2];
    timestamp_infos[0] = vku::InitStructHelper();
    timestamp_infos[0].timeDomain = time_domains[0];
    timestamp_infos[1] = vku::InitStructHelper();
    timestamp_infos[1].timeDomain = time_domains[0];

    uint64_t timestamps[2];
    uint64_t max_deviation;
    m_errorMonitor->SetDesiredError("VUID-vkGetCalibratedTimestampsEXT-timeDomain-09246");
    vk::GetCalibratedTimestampsKHR(device(), 2, timestamp_infos, timestamps, &max_deviation);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, GetCalibratedTimestampsQuery) {
    TEST_DESCRIPTION("vkGetCalibratedTimestampsEXT with invalid timeDomain.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    uint32_t count = 0;
    vk::GetPhysicalDeviceCalibrateableTimeDomainsEXT(gpu(), &count, nullptr);
    std::vector<VkTimeDomainEXT> time_domains(count);
    vk::GetPhysicalDeviceCalibrateableTimeDomainsEXT(gpu(), &count, time_domains.data());

    for (uint32_t i = 0; i < count; i++) {
        if (time_domains[i] == VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT) {
            GTEST_SKIP() << "Support for VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT";
        }
    }
    VkCalibratedTimestampInfoEXT timestamp_info = vku::InitStructHelper();
    timestamp_info.timeDomain = VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT;

    uint64_t timestamp;
    uint64_t max_deviation;
    m_errorMonitor->SetDesiredError("VUID-VkCalibratedTimestampInfoEXT-timeDomain-02354");
    vk::GetCalibratedTimestampsEXT(device(), 1, &timestamp_info, &timestamp, &max_deviation);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RequiredMeshShaderFeatures) {
    TEST_DESCRIPTION("Create device with missing required features");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.multiviewMeshShader && !mesh_shader_features.primitiveFragmentShadingRateMeshShader) {
        GTEST_SKIP() << "None of tested features are supported";
    }

    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_info;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice device;

    if (mesh_shader_features.multiviewMeshShader) {
        VkPhysicalDeviceMeshShaderFeaturesEXT tested_features = vku::InitStructHelper();
        tested_features.multiviewMeshShader = VK_TRUE;
        VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&tested_features);
        device_create_info.pNext = &features2;

        m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceMeshShaderFeaturesEXT-multiviewMeshShader-07032");
        vk::CreateDevice(gpu(), &device_create_info, nullptr, &device);
        m_errorMonitor->VerifyFound();
    }

    if (mesh_shader_features.primitiveFragmentShadingRateMeshShader) {
        VkPhysicalDeviceMeshShaderFeaturesEXT tested_features = vku::InitStructHelper();
        tested_features.primitiveFragmentShadingRateMeshShader = VK_TRUE;
        VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&tested_features);
        device_create_info.pNext = &features2;

        m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceMeshShaderFeaturesEXT-primitiveFragmentShadingRateMeshShader-07033");
        vk::CreateDevice(gpu(), &device_create_info, nullptr, &device);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, RayTracingStageFlagWithoutFeature) {
    TEST_DESCRIPTION("Test using the ray tracing stage flag without enabling any of ray tracing features");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::Semaphore semaphore(*m_device);
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreCount = 1u;
    submit_info.pSignalSemaphores = &semaphore.handle();
    vk::QueueSubmit(m_default_queue->handle(), 1u, &submit_info, VK_NULL_HANDLE);

    submit_info.signalSemaphoreCount = 0u;
    submit_info.waitSemaphoreCount = 1u;
    submit_info.pWaitSemaphores = &semaphore.handle();
    submit_info.pWaitDstStageMask = &stage;

    m_errorMonitor->SetDesiredError("VUID-VkSubmitInfo-pWaitDstStageMask-07949");
    vk::QueueSubmit(m_default_queue->handle(), 1u, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vkt::Event event(*m_device);
    m_command_buffer.begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdSetEvent-stageMask-07949");
    vk::CmdSetEvent(m_command_buffer.handle(), event.handle(), stage);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdResetEvent-stageMask-07949");
    vk::CmdResetEvent(m_command_buffer.handle(), event.handle(), stage);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-srcStageMask-07949");
    vk::CmdWaitEvents(m_command_buffer.handle(), 1u, &event.handle(), stage, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0u, nullptr, 0u,
                      nullptr, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdWaitEvents-dstStageMask-07949");
    vk::CmdWaitEvents(m_command_buffer.handle(), 1u, &event.handle(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, stage, 0u, nullptr, 0u,
                      nullptr, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-srcStageMask-07949");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), stage, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0u, 0u, nullptr, 0u, nullptr, 0u,
                           nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdPipelineBarrier-dstStageMask-07949");
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, stage, 0u, 0u, nullptr, 0u, nullptr, 0u,
                           nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.end();

    m_default_queue->Wait();
}

TEST_F(VkLayerTest, ExtensionXmlDependsLogic) {
    TEST_DESCRIPTION("Make sure the OR in 'depends' from XML is observed correctly");
    // VK_KHR_buffer_device_address requires
    // (VK_KHR_get_physical_device_properties2 AND VK_KHR_device_group) OR VK_VERSION_1_1
    // If Vulkan 1.1 is not supported, should still be valid
    SetTargetApiVersion(VK_API_VERSION_1_0);
    if (!InstanceExtensionSupported(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME) ||
        !InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find the required instance extensions";
    }
    m_instance_extension_names.push_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    if (!DeviceExtensionSupported(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) ||
        !DeviceExtensionSupported(VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find the required device extensions";
    }

    m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    // missing VK_KHR_device_group

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    VkDeviceCreateInfo dev_info = vku::InitStructHelper();
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = nullptr;
    dev_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extension_names.size());
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredError("VUID-vkCreateDevice-ppEnabledExtensionNames-01387");
    VkDevice device;
    vk::CreateDevice(gpu_, &dev_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ExtensionXmlDependsLogic2) {
    // VK_KHR_shared_presentable_image requires
    // VK_KHR_swapchain
    //    and
    // VK_KHR_get_surface_capabilities2 (missing)
    //    and
    //      VK_KHR_get_physical_device_properties2
    //         or
    //      Version 1.1
    SetTargetApiVersion(VK_API_VERSION_1_0);
    if (!InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME) ||
        !InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find the required instance extensions";
    }
    m_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    if (!DeviceExtensionSupported(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME) ||
        !DeviceExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find the required device extensions";
    }

    m_device_extension_names.push_back(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    VkDeviceCreateInfo dev_info = vku::InitStructHelper();
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = nullptr;
    dev_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extension_names.size());
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredError("VUID-vkCreateDevice-ppEnabledExtensionNames-01387");
    VkDevice device;
    vk::CreateDevice(gpu_, &dev_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ExtensionXmlDependsLogic3) {
    // VK_KHR_shared_presentable_image requires
    // VK_KHR_swapchain
    //    and
    // VK_KHR_get_surface_capabilities2
    //    and
    //      VK_KHR_get_physical_device_properties2  (missing)
    //         or
    //      Version 1.1  (missing)
    SetTargetApiVersion(VK_API_VERSION_1_0);
    if (!InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME) ||
        !InstanceExtensionSupported(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find the required instance extensions";
    }
    m_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    m_instance_extension_names.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    if (!DeviceExtensionSupported(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME) ||
        !DeviceExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find the required device extensions";
    }

    m_device_extension_names.push_back(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    VkDeviceCreateInfo dev_info = vku::InitStructHelper();
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = nullptr;
    dev_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extension_names.size());
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    m_errorMonitor->SetDesiredError("VUID-vkCreateDevice-ppEnabledExtensionNames-01387");
    VkDevice device;
    vk::CreateDevice(gpu_, &dev_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingExtensionPhysicalDeviceProperties) {
    TEST_DESCRIPTION("Don't enable instance extension needed");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // requires VK_KHR_external_fence_capabilities
    VkPhysicalDeviceIDPropertiesKHR id_properties = vku::InitStructHelper();
    VkPhysicalDeviceProperties2 properties2 = vku::InitStructHelper(&id_properties);
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2KHR(gpu(), &properties2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidGetExternalBufferPropertiesUsage) {
    TEST_DESCRIPTION("Call vkGetPhysicalDeviceExternalBufferProperties with invalid usage");

    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

#ifdef _WIN32
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif

    VkPhysicalDeviceExternalBufferInfo externalBufferInfo = vku::InitStructHelper();
    externalBufferInfo.usage = 0x80000000;
    externalBufferInfo.handleType = handle_type;

    VkExternalBufferProperties externalBufferProperties = vku::InitStructHelper();

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceExternalBufferInfo-None-09499");
    vk::GetPhysicalDeviceExternalBufferPropertiesKHR(gpu(), &externalBufferInfo, &externalBufferProperties);
    m_errorMonitor->VerifyFound();

    externalBufferInfo.usage = 0u;
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceExternalBufferInfo-None-09500");
    vk::GetPhysicalDeviceExternalBufferPropertiesKHR(gpu(), &externalBufferInfo, &externalBufferProperties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingExtensionPhysicalDeviceFeature) {
    TEST_DESCRIPTION("Add feature to vkCreateDevice withouth extension");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework());
    if (!DeviceExtensionSupported(VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_EXT_astc_decode_mode not supported";
    }
    // likely to never be promoted to core
    VkPhysicalDeviceASTCDecodeFeaturesEXT astc_feature = vku::InitStructHelper();
    astc_feature.decodeModeSharedExponent = VK_TRUE;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    VkDeviceCreateInfo dev_info = vku::InitStructHelper(&astc_feature);
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    VkDevice device;
    vk::CreateDevice(gpu_, &dev_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingExtensionPhysicalDeviceFeatureMultiple) {
    TEST_DESCRIPTION(
        "VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR has an EXT and KHR extension that can enable it, but we forgot both");
    RETURN_IF_SKIP(InitFramework());
    if (!DeviceExtensionSupported(VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME) &&
        !DeviceExtensionSupported(VK_EXT_GLOBAL_PRIORITY_QUERY_EXTENSION_NAME)) {
        GTEST_SKIP() << "VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR not supported";
    }
    VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR query_feature = vku::InitStructHelper();
    query_feature.globalPriorityQuery = VK_TRUE;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    VkDeviceCreateInfo dev_info = vku::InitStructHelper(&query_feature);
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    VkDevice device;
    vk::CreateDevice(gpu_, &dev_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingExtensionPhysicalDeviceFeaturePromoted) {
    TEST_DESCRIPTION("Add feature to vkCreateDevice withouth extension (for a promoted extension)");
    SetTargetApiVersion(VK_API_VERSION_1_2);  // VK_KHR_maintenance4 added in 1.3
    RETURN_IF_SKIP(InitFramework());
    if (!DeviceExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance4 not supported";
    }
    VkPhysicalDeviceMaintenance4Features maintenance4_feature = vku::InitStructHelper();
    maintenance4_feature.maintenance4 = VK_TRUE;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;

    VkDeviceCreateInfo dev_info = vku::InitStructHelper(&maintenance4_feature);
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.enabledExtensionCount = 0;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    VkDevice device;
    vk::CreateDevice(gpu_, &dev_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DescriptorBufferNoExtension) {
    TEST_DESCRIPTION("Create VkBuffer without the extension.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init());
    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 64;
    buffer_ci.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-None-09499");
    vk::CreateBuffer(*m_device, &buffer_ci, nullptr, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingExtensionStruct) {
    TEST_DESCRIPTION("Don't add extension but use extended structure");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    if (!DeviceExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_maintenance5 not supported";
    }

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);

    // added in VK_KHR_maintenance5
    VkBufferUsageFlags2CreateInfoKHR buffer_usage_flags = vku::InitStructHelper();
    buffer_usage_flags.usage = VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT_KHR;

    VkBufferViewCreateInfo buffer_view_ci = vku::InitStructHelper(&buffer_usage_flags);
    buffer_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    buffer_view_ci.range = VK_WHOLE_SIZE;
    buffer_view_ci.buffer = buffer.handle();
    m_errorMonitor->SetDesiredError("VUID-VkBufferViewCreateInfo-pNext-pNext");
    vkt::BufferView view(*m_device, buffer_view_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Features11WithoutVulkan12) {
    TEST_DESCRIPTION("VkPhysicalDeviceVulkan11Features was added in Vulkan1.2");
    if (m_instance_api_version < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "Need 1.2 instance support";
    }
    app_info_.apiVersion = m_instance_api_version.Value();
    RETURN_IF_SKIP(InitFramework());
    if (physDevProps().apiVersion > VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Need 1.0/1.1 device support";
    }

    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    VkDeviceCreateInfo device_create_info = vku::InitStructHelper(&features11);
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_info;
    VkDevice testDevice;

    m_errorMonitor->SetDesiredError("VUID-VkDeviceCreateInfo-pNext-pNext");
    vk::CreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();

    if (physDevProps().apiVersion == VK_API_VERSION_1_1) {
        VkPhysicalDeviceVulkan12Properties bad_version_1_1_struct = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 phys_dev_props_2 = vku::InitStructHelper(&bad_version_1_1_struct);
        m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
        vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, MissingCreateInfo) {
    RETURN_IF_SKIP(Init());

    VkBuffer buffer;
    m_errorMonitor->SetDesiredError("VUID-vkCreateBuffer-pCreateInfo-parameter");
    vk::CreateBuffer(device(), nullptr, nullptr, &buffer);
    m_errorMonitor->VerifyFound();

    VkImage image;
    m_errorMonitor->SetDesiredError("VUID-vkCreateImage-pCreateInfo-parameter");
    vk::CreateImage(device(), nullptr, nullptr, &image);
    m_errorMonitor->VerifyFound();

    VkBufferView buffer_view;
    m_errorMonitor->SetDesiredError("VUID-vkCreateBufferView-pCreateInfo-parameter");
    vk::CreateBufferView(device(), nullptr, nullptr, &buffer_view);
    m_errorMonitor->VerifyFound();

    VkImageView image_view;
    m_errorMonitor->SetDesiredError("VUID-vkCreateImageView-pCreateInfo-parameter");
    vk::CreateImageView(device(), nullptr, nullptr, &image_view);
    m_errorMonitor->VerifyFound();

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredError("VUID-vkCreateRenderPass-pCreateInfo-parameter");
    vk::CreateRenderPass(device(), nullptr, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-vkCreateFramebuffer-pCreateInfo-parameter");
    vk::CreateFramebuffer(device(), nullptr, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-vkCreateQueryPool-pCreateInfo-parameter");
    vk::CreateQueryPool(device(), nullptr, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    VkPipelineLayout pipeline_layout;
    m_errorMonitor->SetDesiredError("VUID-vkCreatePipelineLayout-pCreateInfo-parameter");
    vk::CreatePipelineLayout(device(), nullptr, nullptr, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    VkPipelineCache pipeline_cache;
    m_errorMonitor->SetDesiredError("VUID-vkCreatePipelineCache-pCreateInfo-parameter");
    vk::CreatePipelineCache(device(), nullptr, nullptr, &pipeline_cache);
    m_errorMonitor->VerifyFound();

    VkShaderModule shader_module;
    m_errorMonitor->SetDesiredError("VUID-vkCreateShaderModule-pCreateInfo-parameter");
    vk::CreateShaderModule(device(), nullptr, nullptr, &shader_module);
    m_errorMonitor->VerifyFound();

    VkFence fence;
    m_errorMonitor->SetDesiredError("VUID-vkCreateFence-pCreateInfo-parameter");
    vk::CreateFence(device(), nullptr, nullptr, &fence);
    m_errorMonitor->VerifyFound();

    VkSemaphore semaphore;
    m_errorMonitor->SetDesiredError("VUID-vkCreateSemaphore-pCreateInfo-parameter");
    vk::CreateSemaphore(device(), nullptr, nullptr, &semaphore);
    m_errorMonitor->VerifyFound();

    VkEvent event;
    m_errorMonitor->SetDesiredError("VUID-vkCreateEvent-pCreateInfo-parameter");
    vk::CreateEvent(device(), nullptr, nullptr, &event);
    m_errorMonitor->VerifyFound();

    VkSampler sampler;
    m_errorMonitor->SetDesiredError("VUID-vkCreateSampler-pCreateInfo-parameter");
    vk::CreateSampler(device(), nullptr, nullptr, &sampler);
    m_errorMonitor->VerifyFound();

    VkCommandPool command_pool;
    m_errorMonitor->SetDesiredError("VUID-vkCreateCommandPool-pCreateInfo-parameter");
    vk::CreateCommandPool(device(), nullptr, nullptr, &command_pool);
    m_errorMonitor->VerifyFound();

    VkDescriptorSetLayout set_layout;
    m_errorMonitor->SetDesiredError("VUID-vkCreateDescriptorSetLayout-pCreateInfo-parameter");
    vk::CreateDescriptorSetLayout(device(), nullptr, nullptr, &set_layout);
    m_errorMonitor->VerifyFound();

    VkDescriptorPool descriptor_pool;
    m_errorMonitor->SetDesiredError("VUID-vkCreateDescriptorPool-pCreateInfo-parameter");
    vk::CreateDescriptorPool(device(), nullptr, nullptr, &descriptor_pool);
    m_errorMonitor->VerifyFound();

    VkCommandBuffer command_buffer;
    m_errorMonitor->SetDesiredError("VUID-vkAllocateCommandBuffers-pAllocateInfo-parameter");
    vk::AllocateCommandBuffers(device(), nullptr, &command_buffer);
    m_errorMonitor->VerifyFound();

    // TODO - vvl::AllocateDescriptorSetsData currently doesn't null check pAllocateInfo
    // VkDescriptorSet descriptor_set;
    // m_errorMonitor->SetDesiredError("VUID-vkAllocateDescriptorSets-pAllocateInfo-parameter");
    // vk::AllocateDescriptorSets(device(), nullptr, &descriptor_set);
    // m_errorMonitor->VerifyFound();

    VkDeviceMemory device_memory;
    m_errorMonitor->SetDesiredError("VUID-vkAllocateMemory-pAllocateInfo-parameter");
    vk::AllocateMemory(device(), nullptr, nullptr, &device_memory);
    m_errorMonitor->VerifyFound();

    VkPipeline pipeline;
    m_errorMonitor->SetDesiredError("VUID-vkCreateGraphicsPipelines-pCreateInfos-parameter");
    vk::CreateGraphicsPipelines(device(), VK_NULL_HANDLE, 1, nullptr, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCreateComputePipelines-pCreateInfos-parameter");
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, nullptr, nullptr, &pipeline);
    m_errorMonitor->VerifyFound();
}

// Android loader returns an error in this case, so never makes it to the VVL
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
TEST_F(VkLayerTest, GetDeviceProcAddrInstance) {
    TEST_DESCRIPTION("Call GetDeviceProcAddr on an instance function");
    RETURN_IF_SKIP(Init());
    m_errorMonitor->SetDesiredWarning("WARNING-vkGetDeviceProcAddr-device");
    vk::GetDeviceProcAddr(device(), "vkGetPhysicalDeviceProperties");
    m_errorMonitor->VerifyFound();
}
#endif

// TODO - Can reproduce locally when setting VK_LAYER_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME
// but need to unset variable and make sure works on Android before having CI run this
TEST_F(VkLayerTest, DISABLED_DisplayApplicationName) {
    TEST_DESCRIPTION("Test message_format_display_application_name");
    const char *name_0 = "first instance";
    app_info_.pApplicationName = name_0;
    RETURN_IF_SKIP(Init());

    const char *name_1 = "second instance";
    app_info_.pApplicationName = name_1;
    const auto instance_create_info = GetInstanceCreateInfo();
    VkInstance instance2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&instance_create_info, nullptr, &instance2));

    uint32_t gpu_count = 0;
    vk::EnumeratePhysicalDevices(instance2, &gpu_count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(gpu_count);
    vk::EnumeratePhysicalDevices(instance2, &gpu_count, physical_devices.data());
    VkPhysicalDevice instance2_physical_device = physical_devices[0];
    // scope so device is destroyed before instance
    {
        vkt::Device device2(instance2_physical_device, m_device_extension_names);

        // VUID-vkCreateImage-pCreateInfo-parameter
        VkImage image;

        m_errorMonitor->SetDesiredError("AppName: first instance");
        vk::CreateImage(device(), nullptr, nullptr, &image);
        m_errorMonitor->VerifyFound();

        // TODO - The second instance is not hooked up to the callback so will crash in corecheck or the driver
        m_errorMonitor->SetDesiredError("AppName: second instance");
        vk::CreateImage(device2.handle(), nullptr, nullptr, &image);
        m_errorMonitor->VerifyFound();
    }
    ASSERT_NO_FATAL_FAILURE(vk::DestroyInstance(instance2, nullptr));
}

TEST_F(VkLayerTest, GetDeviceFaultInfoEXT) {
    TEST_DESCRIPTION("Call vkGetDeviceFaultInfoEXT when no device is lost");
    AddRequiredExtensions(VK_EXT_DEVICE_FAULT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::deviceFault);
    RETURN_IF_SKIP(Init());
    VkDeviceFaultCountsEXT fault_count = vku::InitStructHelper();
    VkDeviceFaultInfoEXT fault_info = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-vkGetDeviceFaultInfoEXT-device-07336");
    vk::GetDeviceFaultInfoEXT(device(), &fault_count, &fault_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PhysicalDeviceLayeredApiVulkanPropertiesKHR) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_7_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance7);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceVulkan12Properties vulkan_12_props = vku::InitStructHelper();  // not allowed
    VkPhysicalDeviceDriverProperties driver_props = vku::InitStructHelper(&vulkan_12_props);

    VkPhysicalDeviceLayeredApiVulkanPropertiesKHR api_vulkan_props = vku::InitStructHelper();
    api_vulkan_props.properties.pNext = &driver_props;

    VkPhysicalDeviceLayeredApiPropertiesKHR api_props = vku::InitStructHelper(&api_vulkan_props);

    VkPhysicalDeviceLayeredApiPropertiesListKHR api_prop_lists = vku::InitStructHelper();
    api_prop_lists.layeredApiCount = 1;
    api_prop_lists.pLayeredApis = &api_props;

    VkPhysicalDeviceProperties2 phys_dev_props_2 = vku::InitStructHelper(&api_prop_lists);

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceLayeredApiVulkanPropertiesKHR-pNext-10011");
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedEnumExtension) {
    RETURN_IF_SKIP(Init());
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-format-parameter");
    vkt::Image image(*m_device, 4, 4, 1, VK_FORMAT_A4B4G4R4_UNORM_PACK16, VK_IMAGE_USAGE_SAMPLED_BIT);
    m_errorMonitor->VerifyFound();
}