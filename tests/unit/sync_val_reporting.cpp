/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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
 */

#include "../framework/layer_validation_tests.h"

struct NegativeSyncValReporting : public VkSyncValTest {};

TEST_F(NegativeSyncValReporting, DebugRegion) {
    TEST_DESCRIPTION("Prior access debug region reporting: single debug region");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    m_command_buffer.Begin();
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    m_command_buffer.Copy(buffer_a, buffer_b);

    m_errorMonitor->SetDesiredError("RegionA");
    m_command_buffer.Copy(buffer_c, buffer_a);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message

    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(NegativeSyncValReporting, DebugRegion2) {
    TEST_DESCRIPTION("Prior access debug region reporting: two nested debug regions");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    m_command_buffer.Begin();

    // Command buffer can start with end label command. It can close debug region
    // started in the previous command buffer (that's why primary command buffer labels
    // are validated at sumbit time). Start command buffer with EndLabel command to
    // check it is handled properly.
    //
    // NOTE: the following command crashes CI Linux-Mesa-6800 driver but works
    // in other configurations. Disabled for now.
    // vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);

    label.pLabelName = "RegionB";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    m_command_buffer.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    m_errorMonitor->SetDesiredError("RegionA::RegionB");
    m_command_buffer.Copy(buffer_c, buffer_a);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message

    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(NegativeSyncValReporting, DebugRegion3) {
    TEST_DESCRIPTION(
        "Prior access debug region reporting: there is a nested region but prior access happens in the top level region");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_d(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    m_command_buffer.Begin();
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);

    label.pLabelName = "RegionB";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    m_command_buffer.Copy(buffer_a, buffer_d);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    // this is where prior access happens for the reported hazard
    m_command_buffer.Copy(buffer_a, buffer_b);

    m_errorMonitor->SetDesiredError("RegionA");
    m_command_buffer.Copy(buffer_c, buffer_a);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message

    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    m_command_buffer.End();
}

TEST_F(NegativeSyncValReporting, DebugRegion4) {
    TEST_DESCRIPTION("Prior access debug region reporting: multiple nested debug regions");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::Event event(*m_device);
    vkt::Event event2(*m_device);

    m_command_buffer.Begin();
    label.pLabelName = "VulkanFrame";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);

    label.pLabelName = "ResetEvent";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    vk::CmdResetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    label.pLabelName = "FirstPass";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);

    label.pLabelName = "CopyAToB";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    m_command_buffer.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    label.pLabelName = "ResetEvent2";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    vk::CmdResetEvent(m_command_buffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);

    m_errorMonitor->SetDesiredError("VulkanFrame::FirstPass::CopyAToB");
    m_command_buffer.Copy(buffer_c, buffer_a);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message

    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);  // FirstPass
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);  // VulkanFrame

    m_command_buffer.End();
}

TEST_F(NegativeSyncValReporting, DebugRegion_Secondary) {
    TEST_DESCRIPTION("Prior access debug region reporting: secondary command buffer");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb.Begin();
    label.pLabelName = "RegionB";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb, &label);
    secondary_cb.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb);
    secondary_cb.End();

    m_command_buffer.Begin();
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary_cb.handle());
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    m_errorMonitor->SetDesiredError("RegionA::RegionB");
    m_command_buffer.Copy(buffer_c, buffer_a);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message
    m_command_buffer.End();
}

TEST_F(NegativeSyncValReporting, DebugRegion_Secondary2) {
    TEST_DESCRIPTION("Prior access debug region reporting: secondary command buffer first access validation");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::CommandBuffer secondary_cb0(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb0.Begin();
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb0, &label);
    secondary_cb0.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb0);
    secondary_cb0.End();

    vkt::CommandBuffer secondary_cb1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb1.Begin();
    label.pLabelName = "RegionB";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb1, &label);
    secondary_cb1.Copy(buffer_c, buffer_a);
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb1);
    secondary_cb1.End();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("RegionA");
    VkCommandBuffer secondary_cbs[2] = {secondary_cb0, secondary_cb1};
    vk::CmdExecuteCommands(m_command_buffer, 2, secondary_cbs);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message
    m_command_buffer.End();
}

TEST_F(NegativeSyncValReporting, QSDebugRegion) {
    TEST_DESCRIPTION("Prior access debug region reporting: single debug region per command buffer");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    label.pLabelName = "RegionA";
    cb0.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(cb0);
    cb0.End();

    label.pLabelName = "RegionB";
    cb1.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb1, &label);
    cb1.Copy(buffer_c, buffer_a);
    vk::CmdEndDebugUtilsLabelEXT(cb1);
    cb1.End();

    std::array command_buffers = {&cb0, &cb1};
    m_errorMonitor->SetDesiredError("RegionA");
    m_default_queue->Submit(command_buffers);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, QSDebugRegion2) {
    TEST_DESCRIPTION("Prior access debug region reporting: previous access is in the previous submission");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "RegionA";
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(cb0);
    cb0.End();
    m_default_queue->Submit(cb0);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    cb1.Copy(buffer_c, buffer_a);
    cb1.End();
    m_errorMonitor->SetDesiredError("RegionA");
    m_default_queue->Submit(cb1);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, QSDebugRegion3) {
    TEST_DESCRIPTION("Prior access debug region reporting: multiple nested debug regions");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    vkt::CommandBuffer cb1(*m_device, m_command_pool);

    vkt::Event event(*m_device);  // events are not used for some specific functionality, only to create additional debug regions
    vkt::Event event2(*m_device);

    // CommandBuffer0
    label.pLabelName = "VulkanFrame_CommandBuffer0";
    cb0.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);

    label.pLabelName = "ResetEvent";
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    vk::CmdResetEvent(cb0, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vk::CmdEndDebugUtilsLabelEXT(cb0);

    label.pLabelName = "FirstPass";
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);

    label.pLabelName = "CopyAToB";
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(cb0);

    vk::CmdEndDebugUtilsLabelEXT(cb0);  // FirstPass
    vk::CmdEndDebugUtilsLabelEXT(cb0);  // VulkanFrame_CommandBuffer0
    cb0.End();

    // CommandBuffer1
    label.pLabelName = "VulkanFrame_CommandBuffer1";
    cb1.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb1, &label);

    label.pLabelName = "ResetEvent2";
    vk::CmdBeginDebugUtilsLabelEXT(cb1, &label);
    vk::CmdResetEvent(cb1, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    vk::CmdEndDebugUtilsLabelEXT(cb1);

    label.pLabelName = "SecondPass";
    vk::CmdBeginDebugUtilsLabelEXT(cb1, &label);

    label.pLabelName = "CopyCToA";
    vk::CmdBeginDebugUtilsLabelEXT(cb1, &label);
    cb1.Copy(buffer_c, buffer_a);
    vk::CmdEndDebugUtilsLabelEXT(cb1);

    vk::CmdEndDebugUtilsLabelEXT(cb1);  // SecondPass
    vk::CmdEndDebugUtilsLabelEXT(cb1);  // VulkanFrame_CommandBuffer1
    cb1.End();

    std::array command_buffers = {&cb0, &cb1};
    m_errorMonitor->SetDesiredError("VulkanFrame_CommandBuffer0::FirstPass::CopyAToB");
    m_default_queue->Submit(command_buffers);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, QSDebugRegion4) {
    TEST_DESCRIPTION("Prior access debug region reporting: debug region is formed by two command buffers");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.End();
    m_default_queue->Submit(cb0);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    label.pLabelName = "RegionB";
    vk::CmdBeginDebugUtilsLabelEXT(cb1, &label);
    cb1.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(cb1);  // RegionB
    cb1.End();
    m_default_queue->Submit(cb1);

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    cb2.Copy(buffer_c, buffer_a);
    vk::CmdEndDebugUtilsLabelEXT(cb2);  // RegionA
    cb2.End();
    m_errorMonitor->SetDesiredError("RegionA::RegionB");
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, QSDebugRegion5) {
    TEST_DESCRIPTION("Prior access debug region reporting: debug region is formed by two command buffers from the same submit");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.End();

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    label.pLabelName = "RegionB";
    vk::CmdBeginDebugUtilsLabelEXT(cb1, &label);
    cb1.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(cb1);  // RegionB
    vk::CmdEndDebugUtilsLabelEXT(cb1);  // RegionA
    cb1.End();

    std::array command_buffers = {&cb0, &cb1};
    m_default_queue->Submit(command_buffers);

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    cb2.Copy(buffer_c, buffer_a);
    cb2.End();
    m_errorMonitor->SetDesiredError("RegionA::RegionB");
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, QSDebugRegion6) {
    TEST_DESCRIPTION("Prior access debug region reporting: debug region is formed by two batches from the same submit");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.End();

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    label.pLabelName = "RegionB";
    vk::CmdBeginDebugUtilsLabelEXT(cb1, &label);
    cb1.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(cb1);  // RegionB
    vk::CmdEndDebugUtilsLabelEXT(cb1);  // RegionA
    cb1.End();

    VkSubmitInfo submit_infos[2];
    submit_infos[0] = vku::InitStructHelper();
    submit_infos[0].commandBufferCount = 1;
    submit_infos[0].pCommandBuffers = &cb0.handle();
    submit_infos[1] = vku::InitStructHelper();
    submit_infos[1].commandBufferCount = 1;
    submit_infos[1].pCommandBuffers = &cb1.handle();
    vk::QueueSubmit(*m_default_queue, 2, submit_infos, VK_NULL_HANDLE);

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    cb2.Copy(buffer_c, buffer_a);
    cb2.End();
    m_errorMonitor->SetDesiredError("RegionA::RegionB");
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message
    m_default_queue->Wait();
}

// Regression test for part 1 of https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7502
TEST_F(NegativeSyncValReporting, QSDebugRegion7) {
    TEST_DESCRIPTION("Prior access debug region reporting: command buffer has labels but hazardous command is not in debug region");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    cb0.Copy(buffer_a, buffer_b);
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    vk::CmdEndDebugUtilsLabelEXT(cb0);
    cb0.End();
    m_default_queue->Submit(cb0);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    cb1.Copy(buffer_c, buffer_a);
    cb1.End();
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    m_default_queue->Submit(cb1);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, QSDebugRegion_Secondary) {
    TEST_DESCRIPTION("Prior access debug region reporting: secondary command buffer");

    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncValFramework());
    RETURN_IF_SKIP(InitState());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_c(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    vkt::CommandBuffer secondary_cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary_cb.Begin();
    label.pLabelName = "RegionB";
    vk::CmdBeginDebugUtilsLabelEXT(secondary_cb, &label);
    secondary_cb.Copy(buffer_a, buffer_b);
    vk::CmdEndDebugUtilsLabelEXT(secondary_cb);
    secondary_cb.End();

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    vk::CmdExecuteCommands(cb0, 1, &secondary_cb.handle());
    vk::CmdEndDebugUtilsLabelEXT(cb0);
    cb0.End();
    m_default_queue->Submit(cb0);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    cb1.Copy(buffer_c, buffer_a);
    cb1.End();
    m_errorMonitor->SetDesiredError("RegionA::RegionB");
    m_default_queue->Submit(cb1);
    m_errorMonitor->VerifyFound();  // SYNC-HAZARD-WRITE-AFTER-READ error message
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, StaleLabelCommand) {
    TEST_DESCRIPTION("Try to access stale label command when core validation error breaks state invariants");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    const VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer_a(*m_device, 256, buffer_usage);
    vkt::Buffer buffer_b(*m_device, 256, buffer_usage);
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();

    m_command_buffer.Begin();
    // Issue a bunch of label commands
    label.pLabelName = "RegionA";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    label.pLabelName = "RegionB";
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    // At this point 4 label commands were recorded.
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    m_default_queue->Submit(m_command_buffer);

    // Suppress error because we reset command buffer that is still in use.
    // Simulate situation when core validation is disabled and synchronization
    // validation runs with existing core validation error. This should not lead
    // to syncval crashes (but does not guarantee correct syncval behavior).
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkBeginCommandBuffer-commandBuffer-00049");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkQueueSubmit-pCommandBuffers-00071");
    m_command_buffer.Begin();
    m_command_buffer.Copy(buffer_a, buffer_b);
    m_command_buffer.End();

    // This will detect write-after-write hazard. During error reporting debug label
    // information says that prior read occured after the 4th label command. Attempt
    // to access those label commands can lead to crash if out of bounds check is missing.
    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}
