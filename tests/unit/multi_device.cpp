
#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"

MultiDeviceTest::~MultiDeviceTest() {
    if (m_second_device) {
        delete m_second_device;
        m_second_device = nullptr;
    }
}

TEST_F(MultiDeviceTest, CommonParentFillBuffer) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    ASSERT_NO_FATAL_FAILURE(Init());
    auto features = m_device->phy().features();
    m_second_device = new VkDeviceObj(0, gpu_, m_device_extension_names, &features, nullptr);

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.queueFamilyIndexCount = 0;
    vk_testing::Buffer buffer(*m_second_device, buffer_ci);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-commonparent");
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, VK_WHOLE_SIZE, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(MultiDeviceTest, CommonParentBindBuffer) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    auto features = m_device->phy().features();
    m_second_device = new VkDeviceObj(0, gpu_, m_device_extension_names, &features, nullptr);

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_ci.queueFamilyIndexCount = 0;
    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_ci);

    VkMemoryRequirements mem_reqs;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferMemoryRequirements-buffer-parent");
    vk::GetBufferMemoryRequirements(m_second_device->device(), buffer.handle(), &mem_reqs);
    m_errorMonitor->VerifyFound();
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.allocationSize = mem_reqs.size;
    m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    vk_testing::DeviceMemory memory(*m_second_device, mem_alloc);

    VkBindBufferMemoryInfo bind_buffer_info = LvlInitStruct<VkBindBufferMemoryInfo>();
    bind_buffer_info.buffer = buffer.handle();
    bind_buffer_info.memory = memory.handle();
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryInfo-commonparent");
    vk::BindBufferMemory2KHR(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(MultiDeviceTest, CommonParentBindImage) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    auto features = m_device->phy().features();
    m_second_device = new VkDeviceObj(0, gpu_, m_device_extension_names, &features, nullptr);

    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                  VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);

    VkMemoryRequirements mem_reqs;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageMemoryRequirements-image-parent");
    vk::GetImageMemoryRequirements(m_second_device->device(), image.handle(), &mem_reqs);
    m_errorMonitor->VerifyFound();
    vk::GetImageMemoryRequirements(device(), image.handle(), &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.allocationSize = mem_reqs.size;
    m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    vk_testing::DeviceMemory memory(*m_second_device, mem_alloc);

    VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>();
    bind_image_info.image = image.handle();
    bind_image_info.memory = memory.handle();
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-commonparent");
    vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(MultiDeviceTest, CommonParentImageView) {
    TEST_DESCRIPTION("Test VUID-*-commonparent checks not sharing the same Device");

    ASSERT_NO_FATAL_FAILURE(Init());
    auto features = m_device->phy().features();
    m_second_device = new VkDeviceObj(0, gpu_, m_device_extension_names, &features, nullptr);

    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT,
                                                  VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);

    VkImageView image_view;
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-commonparent");
    vk::CreateImageView(m_second_device->device(), &ivci, nullptr, &image_view);
    m_errorMonitor->VerifyFound();
}
