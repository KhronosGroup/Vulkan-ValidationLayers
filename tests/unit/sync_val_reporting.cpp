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
#include "../framework/pipeline_helper.h"

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

TEST_F(NegativeSyncValReporting, ReportBufferResource_SubmitTime) {
    TEST_DESCRIPTION("Test that hazardous buffer is reported");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    buffer_a.SetName("BufferA");
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer_b.SetName("BufferB");
    VkBufferCopy region = {0, 0, 256};

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    vk::CmdCopyBuffer(cb0, buffer_a, buffer_b, 1, &region);
    cb0.End();
    m_default_queue->Submit(cb0);

    // WAW for BufferB. Expect BufferB in the error message but not BufferA.
    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    vk::CmdCopyBuffer(cb1, buffer_a, buffer_b, 1, &region);
    cb1.End();
    const char* contains_b_but_not_a = "(?=.*BufferB)(?!.*BufferA)";
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-WRITE", contains_b_but_not_a);
    m_default_queue->Submit(cb1);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, ReportImageResource_SubmitTime) {
    TEST_DESCRIPTION("Test that hazardous image is reported");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer(*m_device, 64 * 64 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    buffer.SetName("BufferA");

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_B8G8R8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image.SetName("ImageB");
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    vkt::Image image2(*m_device, 64, 64, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    image2.SetName("ImageC");
    image2.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageCopy image_copy{};
    image_copy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_copy.extent = {64, 64, 1};

    VkBufferImageCopy buffer_image_copy{};
    buffer_image_copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    buffer_image_copy.imageExtent = {64, 64, 1};

    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    vk::CmdCopyImage(cb0, image, VK_IMAGE_LAYOUT_GENERAL, image2, VK_IMAGE_LAYOUT_GENERAL, 1, &image_copy);
    cb0.End();
    m_default_queue->Submit(cb0);

    // WAR for ImageB. Expect ImageB in the error message but not BufferA or ImageC.
    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    vk::CmdCopyBufferToImage(cb1, buffer, image, VK_IMAGE_LAYOUT_GENERAL, 1, &buffer_image_copy);
    cb1.End();
    const char* contains_b_but_not_a_c = "(?=.*ImageB)(?!.*BufferA)(?!.*ImageC)";
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-READ", contains_b_but_not_a_c);
    m_default_queue->Submit(cb1);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, ReportDescriptorBuffer_SubmitTime) {
    TEST_DESCRIPTION("Test that hazardous buffer is reported");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer_a.SetName("BufferA");
    vkt::Buffer buffer_b(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer_b.SetName("BufferB");

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer_b, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer buf_a { uint values_a[]; };
        layout(set=0, binding=1) buffer buf_b { uint values_b[]; };
        void main(){
            values_b[0] = values_a[0];
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    // Submit dispatch that writes to buffer_b
    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    vk::CmdBindPipeline(cb.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb.handle(), 1, 1, 1);
    cb.End();
    m_default_queue->Submit(cb);

    // One more dispatch that writes to the same buffer (WAW hazard). Expect BufferB in the error message but not BufferA.
    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    vk::CmdBindPipeline(cb2.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb2.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb2.handle(), 1, 1, 1);
    cb2.End();
    const char* contains_b_but_not_a = "(?=.*BufferB)(?!.*BufferA)";
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-WRITE", contains_b_but_not_a);
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, ReportDescriptorBuffer2_SubmitTime) {
    TEST_DESCRIPTION("Test that hazardous buffer is reported");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_a(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer_a.SetName("BufferA");
    vkt::Buffer buffer_c(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer_c.SetName("BufferC");
    vkt::Buffer buffer_e(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer_e.SetName("BufferD");

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer_c, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer_e, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer buf_a { uint values_a[]; };
        layout(set=0, binding=1) buffer buf_c { uint values_c[]; };
        layout(set=0, binding=2) buffer buf_e { uint values_e[]; };
        void main(){
            values_c[0] = values_e[0];
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    // Submit dispatch that writes to buffer_b
    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    vk::CmdBindPipeline(cb.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb.handle(), 1, 1, 1);
    cb.End();
    m_default_queue->Submit(cb);

    // Submit copy that writes to the same buffer (WAW hazard). Expect BufferC in the error message but not BufferA/BufferE
    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    cb2.Copy(buffer_a, buffer_c);
    cb2.End();
    const char* contains_c_but_not_a_and_e = "(?=.*BufferC)(?!.*BufferA)(?!.*BufferE)";
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-WRITE", contains_c_but_not_a_and_e);
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, ReportDescriptorBuffer3_SubmitTime) {
    TEST_DESCRIPTION("Different buffer of the same shader is reported depending on the previous access");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer_copy_src(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    buffer_copy_src.SetName("BufferSrc");
    vkt::Buffer buffer_a(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer_a.SetName("BufferA");
    vkt::Buffer buffer_b(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer_b.SetName("BufferB");
    vkt::Buffer buffer_c(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer_c.SetName("BufferC");
    vkt::Buffer buffer_d(*m_device, 128, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer_d.SetName("BufferD");

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer_b, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer_c, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(3, buffer_d, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer buf_a { uint values_a[]; };
        layout(set=0, binding=1) buffer buf_b { uint values_b[]; };
        layout(set=0, binding=2) buffer buf_c { uint values_c[]; };
        layout(set=0, binding=3) buffer buf_d { uint values_d[]; };
        void main(){
            // Depending on which buffer was written before (B or D) one of the following writes causes a WAW hazard
            values_b[0] = values_a[0];
            values_d[0] = values_c[0];

        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    // Create WAW with bufferB. Copy-write then dispatch-write.
    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    cb.Copy(buffer_copy_src, buffer_b);
    cb.End();
    m_default_queue->Submit(cb);

    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    vk::CmdBindPipeline(cb2.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb2.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb2.handle(), 1, 1, 1);
    cb2.End();
    const char* contains_b_but_not_d = "(?=.*BufferB)(?!.*BufferD)";
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-WRITE", contains_b_but_not_d);
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();

    // Create WAW with bufferD. Copy-write then dispatch-write.
    vkt::CommandBuffer cb3(*m_device, m_command_pool);
    cb3.Begin();
    cb3.Copy(buffer_copy_src, buffer_d);
    cb3.End();
    m_default_queue->Submit(cb3);
    const char* contains_buffer_d_but_not_b = "(?=.*BufferD)(?!.*BufferB)";
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-WRITE", contains_buffer_d_but_not_b);
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeSyncValReporting, ReportDescriptorImage_SubmitTime) {
    TEST_DESCRIPTION("Test that hazardous image is reported");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitSyncVal());

    vkt::Buffer buffer(*m_device, 128, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    buffer.SetName("BufferA");

    vkt::Image image(*m_device, 64, 64, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    image.SetName("ImageB");

    vkt::ImageView view = image.CreateView();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.UpdateDescriptorSets();

    const char* cs_source = R"glsl(
        #version 450
        layout(set=0, binding=0) buffer buf_a { uint values_a[]; };
        layout(set=0, binding=1, rgba8) uniform image2D image_b;
        void main(){
            imageStore(image_b, ivec2(2, 5), vec4(1.0, 0.5, 0.5, 1.0));
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    // Submit dispatch that writes to image
    vkt::CommandBuffer cb(*m_device, m_command_pool);
    cb.Begin();
    vk::CmdBindPipeline(cb.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb.handle(), 1, 1, 1);
    cb.End();
    m_default_queue->Submit(cb);

    // One more dispatch that writes to the same image (WAW hazard). Expect ImageB in the error message but not BufferA.
    vkt::CommandBuffer cb2(*m_device, m_command_pool);
    cb2.Begin();
    vk::CmdBindPipeline(cb2.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(cb2.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(cb2.handle(), 1, 1, 1);
    cb2.End();
    const char* contains_b_but_not_a = "(?=.*ImageB)(?!.*BufferA)";
    m_errorMonitor->SetDesiredErrorRegex("SYNC-HAZARD-WRITE-AFTER-WRITE", contains_b_but_not_a);
    m_default_queue->Submit(cb2);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}
