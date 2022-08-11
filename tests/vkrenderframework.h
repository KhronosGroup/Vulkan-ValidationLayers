/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 */

#ifndef VKRENDERFRAMEWORK_H
#define VKRENDERFRAMEWORK_H

#include "lvt_function_pointers.h"

#ifdef ANDROID
#include "vktestframeworkandroid.h"
class VkImageObj;
#else
#include "vktestframework.h"
#endif

#if defined(ANDROID)
#include <android/log.h>
#if defined(VALIDATION_APK)
#include <android_native_app_glue.h>
#endif
#endif

#include <algorithm>
#include <array>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using vk_testing::MakeVkHandles;

template <class Dst, class Src>
std::vector<Dst *> MakeTestbindingHandles(const std::vector<Src *> &v) {
    std::vector<Dst *> handles;
    handles.reserve(v.size());
    std::transform(v.begin(), v.end(), std::back_inserter(handles), [](const Src *o) { return static_cast<Dst *>(o); });
    return handles;
}

typedef vk_testing::Queue VkQueueObj;
class VkDeviceObj : public vk_testing::Device {
  public:
    VkDeviceObj(uint32_t id, VkPhysicalDevice obj);
    VkDeviceObj(uint32_t id, VkPhysicalDevice obj, std::vector<const char *> &extension_names,
                VkPhysicalDeviceFeatures *features = nullptr, void *create_device_pnext = nullptr);

    uint32_t QueueFamilyMatching(VkQueueFlags with, VkQueueFlags without, bool all_bits = true);
    uint32_t QueueFamilyWithoutCapabilities(VkQueueFlags capabilities) {
        // an all_bits match with 0 matches all
        return QueueFamilyMatching(VkQueueFlags(0), capabilities, true /* all_bits with */);
    }

    VkDevice device() { return handle(); }
    void SetDeviceQueue();
    VkQueueObj *GetDefaultQueue();
    VkQueueObj *GetDefaultComputeQueue();

    uint32_t id;
    VkPhysicalDeviceProperties props;
    std::vector<VkQueueFamilyProperties> queue_props;

    VkQueueObj *m_queue_obj = nullptr;
    VkQueue m_queue;
};

// ErrorMonitor Usage:
//
// Call SetDesiredFailureMsg with a string to be compared against all
// encountered log messages, or a validation error enum identifying
// desired error message. Passing NULL or VALIDATION_ERROR_MAX_ENUM
// will match all log messages. logMsg will return true for skipCall
// only if msg is matched or NULL.
//
// Call VerifyFound to determine if all desired failure messages
// were encountered. Call VerifyNotFound to determine if any unexpected
// failure was encountered.
class ErrorMonitor {
  public:
    ErrorMonitor();
    ~ErrorMonitor() NOEXCEPT;

    // Set monitor to pristine state
    void Reset();

    // ErrorMonitor will look for an error message containing the specified string(s)
    void SetDesiredFailureMsg(const VkFlags msgFlags, const std::string msg);
    void SetDesiredFailureMsg(const VkFlags msgFlags, const char *const msgString);

    // ErrorMonitor will look for an error message containing the specified string(s)
    template <typename Iter>
    void SetDesiredFailureMsg(const VkFlags msgFlags, Iter iter, const Iter end) {
        for (; iter != end; ++iter) {
            SetDesiredFailureMsg(msgFlags, *iter);
        }
    }

    // Set an error that the error monitor will ignore. Do not use this function if you are creating a new test.
    // TODO: This is stopgap to block new unexpected errors from being introduced. The long-term goal is to remove the use of this
    // function and its definition.
    void SetUnexpectedError(const char *const msg);

    // Set an error that should not cause a test failure
    void SetAllowedFailureMsg(const char *const msg);

    VkBool32 CheckForDesiredMsg(const char *const msgString);
    VkDebugReportFlagsEXT GetMessageFlags();
    void SetError(const char *const errorString);
    void SetBailout(std::atomic<bool> *bailout);

    // Helpers

    void VerifyFound();
    void Finish() {
        VerifyNotFound();
        Reset();
    }

  private:
    // ExpectSuccess now takes an optional argument allowing a custom combination of debug flags
    void ExpectSuccess(VkDebugReportFlagsEXT const message_flag_mask = kErrorBit);
    bool ExpectingSuccess() const {
        return (desired_message_strings_.size() == 1) &&
               (desired_message_strings_.count("") == 1 && ignore_message_strings_.size() == 0);
    }
    bool NeedCheckSuccess() const { return ExpectingSuccess(); }
    void VerifyNotFound();
    // TODO: This is stopgap to block new unexpected errors from being introduced. The long-term goal is to remove the use of this
    // function and its definition.
    bool IgnoreMessage(std::string const &msg) const;
    std::vector<std::string> GetOtherFailureMsgs() const;
    bool AnyDesiredMsgFound() const;
    bool AllDesiredMsgsFound() const;
    void DumpFailureMsgs() const;
    void MonitorReset();
    std::unique_lock<std::mutex> Lock() const { return std::unique_lock<std::mutex>(mutex_); }

    VkFlags message_flags_;
    std::unordered_multiset<std::string> desired_message_strings_;
    std::unordered_multiset<std::string> failure_message_strings_;
    std::vector<std::string> ignore_message_strings_;
    std::vector<std::string> allowed_message_strings_;
    std::vector<std::string> other_messages_;
    mutable std::mutex mutex_;
    std::atomic<bool> *bailout_;
    bool message_found_;
};

struct DebugReporter {
    void Create(VkInstance instance) NOEXCEPT;
    void Destroy(VkInstance instance) NOEXCEPT;

    ErrorMonitor error_monitor_;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT message_flags, VkDebugReportObjectTypeEXT, uint64_t,
                                                        size_t, int32_t, const char *, const char *msg, void *user_data);

    const char *debug_extension_name = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    VkDebugReportCallbackCreateInfoEXT debug_create_info_ = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT, nullptr,
                                                             VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                                                 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                                                 VK_DEBUG_REPORT_INFORMATION_BIT_EXT,
                                                             &DebugCallback, &error_monitor_};
    using DebugCreateFnType = PFN_vkCreateDebugReportCallbackEXT;
    const char *debug_create_fn_name_ = "vkCreateDebugReportCallbackEXT";
    using DebugDestroyFnType = PFN_vkDestroyDebugReportCallbackEXT;
    const char *debug_destroy_fn_name_ = "vkDestroyDebugReportCallbackEXT";
    VkDebugReportCallbackEXT debug_obj_ = VK_NULL_HANDLE;
#else
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                        VkDebugUtilsMessageTypeFlagsEXT message_types,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

    const char *debug_extension_name = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info_ = {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        nullptr,
        0,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        &DebugCallback,
        &error_monitor_};
    using DebugCreateFnType = PFN_vkCreateDebugUtilsMessengerEXT;
    const char *debug_create_fn_name_ = "vkCreateDebugUtilsMessengerEXT";
    using DebugDestroyFnType = PFN_vkDestroyDebugUtilsMessengerEXT;
    const char *debug_destroy_fn_name_ = "vkDestroyDebugUtilsMessengerEXT";
    VkDebugUtilsMessengerEXT debug_obj_ = VK_NULL_HANDLE;
#endif
};

class VkCommandPoolObj;
class VkCommandBufferObj;
class VkDepthStencilObj;

typedef enum {
    kGalaxyS10,
    kPixel3,
    kPixelC,
    kNexusPlayer,
    kShieldTV,
    kShieldTVb,
    kPixel3aXL,
    kPixel2XL,
    kMockICD,
} PlatformType;

const std::unordered_map<PlatformType, std::string, std::hash<int>> vk_gpu_table = {
    {kGalaxyS10, "Mali-G76"},
    {kPixel3, "Adreno (TM) 630"},
    {kPixelC, "NVIDIA Tegra X1"},
    {kNexusPlayer, "PowerVR Rogue G6430"},
    {kShieldTV, "NVIDIA Tegra X1 (nvgpu)"},
    {kShieldTVb, "NVIDIA Tegra X1 (rev B) (nvgpu)"},
    {kPixel3aXL, "Adreno (TM) 615"},
    {kPixel2XL, "Adreno (TM) 540"},
    {kMockICD, "Vulkan Mock Device"},
};

class VkRenderFramework : public VkTestFramework {
  public:
    VkInstance instance() { return instance_; }
    VkDevice device() { return m_device->device(); }
    VkDeviceObj *DeviceObj() const { return m_device; }
    VkPhysicalDevice gpu();
    VkRenderPass renderPass() { return m_renderPass; }
    const VkRenderPassCreateInfo &RenderPassInfo() const { return m_renderPass_info; };
    VkFramebuffer framebuffer() { return m_framebuffer; }
    ErrorMonitor &Monitor();
    VkPhysicalDeviceProperties physDevProps();

    bool InstanceLayerSupported(const char *layer_name, uint32_t spec_version = 0, uint32_t impl_version = 0);
    bool InstanceExtensionSupported(const char *extension_name, uint32_t spec_version = 0);

    VkInstanceCreateInfo GetInstanceCreateInfo() const;
    void InitFramework(void * /*unused compatibility parameter*/ = NULL, void *instance_pnext = NULL);
    void ShutdownFramework();

    void InitViewport(float width, float height);
    void InitViewport();
    bool InitSurface();
    bool InitSurface(float width, float height);
    bool InitSurface(float width, float height, VkSurfaceKHR &surface);
    void InitSwapchainInfo();
    bool InitSwapchain(VkSurfaceKHR &surface, VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                       VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    bool InitSwapchain(VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                       VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    bool InitSwapchain(VkSurfaceKHR &surface, VkImageUsageFlags imageUsage,  VkSurfaceTransformFlagBitsKHR preTransform, VkSwapchainKHR &swapchain, VkSwapchainKHR oldSwapchain = 0);
    void DestroySwapchain();
    void InitRenderTarget();
    void InitRenderTarget(uint32_t targets);
    void InitRenderTarget(VkImageView *dsBinding);
    void InitRenderTarget(uint32_t targets, VkImageView *dsBinding);
    void DestroyRenderTarget();
    bool InitFrameworkAndRetrieveFeatures(VkPhysicalDeviceFeatures2KHR &features2);

    static bool IgnoreDisableChecks();
    bool IsPlatform(PlatformType platform);
    void GetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures *features);
    void GetPhysicalDeviceProperties(VkPhysicalDeviceProperties *props);
    void InitState(VkPhysicalDeviceFeatures *features = nullptr, void *create_device_pnext = nullptr,
                   const VkCommandPoolCreateFlags flags = 0);

    const VkRenderPassBeginInfo &renderPassBeginInfo() const { return m_renderPassBeginInfo; }

    bool OverrideDevsimForDeviceProfileLayer();
    bool InstanceExtensionEnabled(const char *name);
    bool DeviceExtensionSupported(const char *extension_name, uint32_t spec_version = 0) const;
    bool DeviceExtensionSupported(VkPhysicalDevice, const char *, const char *name,
                                  uint32_t spec_version = 0) const {  // deprecated
        return DeviceExtensionSupported(name, spec_version);
    }
    bool DeviceExtensionEnabled(const char *name);
    bool DeviceSimulation();

    // Tracks ext_name to be enabled at device creation time and attempts to enable any required instance extensions.
    // Does not return anything as the caller should use AreRequiredExtensionsEnabled or AddOptionalExtensions then
    // `ext_name` can refer to a device or instance extension.
    void AddRequiredExtensions(const char *ext_name);
    // Same as AddRequiredExtensions but won't fail a check to AreRequiredExtensionsEnabled
    void AddOptionalExtensions(const char *ext_name);
    // After instance and physical device creation (e.g., after InitFramework), returns true if all required extensions are
    // available, false otherwise
    bool AreRequiredExtensionsEnabled() const;
    // After instance and physical device creation (e.g., after InitFramework), returns if extension was enabled
    bool IsExtensionsEnabled(const char *ext_name) const;
    // if requested extensions are not supported, helper function to get string to print out
    std::string RequiredExtensionsNotSupported() const;

    template <typename GLSLContainer>
    std::vector<uint32_t> GLSLToSPV(VkShaderStageFlagBits stage, const GLSLContainer &code, const char *entry_point = "main",
                                    const VkSpecializationInfo *spec_info = nullptr, const spv_target_env env = SPV_ENV_VULKAN_1_0,
                                    bool debug = false) {
        std::vector<uint32_t> spv;
        GLSLtoSPV(&m_device->props.limits, stage, code, spv, debug, env);
        return spv;
    }

  protected:
    VkRenderFramework();
    virtual ~VkRenderFramework() = 0;

    std::vector<VkLayerProperties> available_layers_; // allow caching of available layers
    std::vector<VkExtensionProperties> available_extensions_; // allow caching of available instance extensions

    DebugReporter debug_reporter_;
    ErrorMonitor *m_errorMonitor = &debug_reporter_.error_monitor_;  // compatibility alias name

    VkApplicationInfo app_info_;
    std::vector<const char *> instance_layers_;
    std::vector<const char *> instance_extensions_;
    std::vector<const char *> &m_instance_extension_names = instance_extensions_;  // compatibility alias name
    VkInstance instance_;
    VkPhysicalDevice gpu_ = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties physDevProps_;

    uint32_t m_gpu_index;
    VkDeviceObj *m_device;
    VkCommandPoolObj *m_commandPool;
    VkCommandBufferObj *m_commandBuffer;
    VkRenderPass m_renderPass;
    VkRenderPassCreateInfo m_renderPass_info = {};
    std::vector<VkAttachmentDescription> m_renderPass_attachments;
    std::vector<VkSubpassDescription> m_renderPass_subpasses;
    std::vector<VkSubpassDependency> m_renderPass_dependencies;

    VkFramebuffer m_framebuffer;
    VkFramebufferCreateInfo m_framebuffer_info;
    std::vector<VkImageView> m_framebuffer_attachments;

    // WSI items
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    Display *m_surface_dpy;
    Window m_surface_window;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_connection_t *m_surface_xcb_conn;
#endif
    VkSwapchainKHR m_swapchain;
    VkSurfaceCapabilitiesKHR m_surface_capabilities;
    std::vector<VkSurfaceFormatKHR> m_surface_formats;
    std::vector<VkPresentModeKHR> m_surface_present_modes;
    VkPresentModeKHR m_surface_non_shared_present_mode;
    VkCompositeAlphaFlagBitsKHR m_surface_composite_alpha;

    std::vector<VkViewport> m_viewports;
    std::vector<VkRect2D> m_scissors;
    float m_lineWidth;
    float m_depthBiasConstantFactor;
    float m_depthBiasClamp;
    float m_depthBiasSlopeFactor;
    float m_blendConstants[4];
    float m_minDepthBounds;
    float m_maxDepthBounds;
    uint32_t m_compareMask;
    uint32_t m_writeMask;
    uint32_t m_reference;
    bool m_addRenderPassSelfDependency;
    std::vector<VkSubpassDependency> m_additionalSubpassDependencies;
    std::vector<VkClearValue> m_renderPassClearValues;
    VkRenderPassBeginInfo m_renderPassBeginInfo;
    std::vector<std::unique_ptr<VkImageObj>> m_renderTargets;
    float m_width, m_height;
    VkFormat m_render_target_fmt;
    VkFormat m_depth_stencil_fmt;
    VkClearColorValue m_clear_color;
    bool m_clear_via_load_op;
    float m_depth_clear_color;
    uint32_t m_stencil_clear_color;
    VkDepthStencilObj *m_depthStencil;

    // Requested extensions to enable at device creation time
    std::vector<const char *> m_required_extensions;
    // Optional extensions to try and enable at device creation time
    std::vector<const char *> m_optional_extensions;
    // Device extensions to enable
    std::vector<const char *> m_device_extension_names;

  private:
    // Add ext_name, the names of all instance extensions required by ext_name, and return true if ext_name is supported. If the
    // extension is not supported, no extension names are added for instance creation. `ext_name` can refer to a device or instance
    // extension.
    bool AddRequestedInstanceExtensions(const char *ext_name);
    // Returns true if the instance extension inst_ext_name is enabled. This call is only valid _after_ previous
    // `AddRequired*Extensions` calls and InitFramework has been called. `inst_ext_name` must be an instance extension name; false
    // is returned for all device extension names.
    bool CanEnableInstanceExtension(const std::string &inst_ext_name) const;
    // Add dev_ext_name, then names of _device_ extensions required by dev_ext_name, and return true if dev_ext_name is supported.
    // If the extension is not supported, no extension names are added for device creation. This function has no effect if
    // dev_ext_name refers to an instance extension.
    bool AddRequestedDeviceExtensions(const char *dev_ext_name);
    // Returns true if the device extension is enabled. This call is only valid _after_ previous `AddRequired*Extensions` calls and
    // InitFramework has been called.
    // `dev_ext_name` msut be an instance extension name; false is returned for all instance extension names.
    bool CanEnableDeviceExtension(const std::string &dev_ext_name) const;
};

class VkDescriptorSetObj;
class VkConstantBufferObj;
class VkPipelineObj;
typedef vk_testing::Event VkEventObj;
typedef vk_testing::Fence VkFenceObj;
typedef vk_testing::Buffer VkBufferObj;
typedef vk_testing::AccelerationStructure VkAccelerationStructureObj;
typedef vk_testing::AccelerationStructureKHR VkAccelerationStructurekhrObj;
class VkCommandPoolObj : public vk_testing::CommandPool {
  public:
    VkCommandPoolObj() : vk_testing::CommandPool(){};
    void Init(VkDeviceObj *device, uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
    VkCommandPoolObj(VkDeviceObj *device, uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
};

class VkCommandBufferObj : public vk_testing::CommandBuffer {
  public:
    VkCommandBufferObj() : vk_testing::CommandBuffer() {}
    VkCommandBufferObj(VkDeviceObj *device, VkCommandPoolObj *pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                       VkQueueObj *queue = nullptr);
    void Init(VkDeviceObj *device, VkCommandPoolObj *pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
              VkQueueObj *queue = nullptr);
    void PipelineBarrier(VkPipelineStageFlags src_stages, VkPipelineStageFlags dest_stages, VkDependencyFlags dependencyFlags,
                         uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                         const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                         const VkImageMemoryBarrier *pImageMemoryBarriers);
    void PipelineBarrier2KHR(const VkDependencyInfoKHR *pDependencyInfo);
    void ClearAllBuffers(const std::vector<std::unique_ptr<VkImageObj>> &color_objs, VkClearColorValue clear_color,
                         VkDepthStencilObj *depth_stencil_obj, float depth_clear_value, uint32_t stencil_clear_value);
    void PrepareAttachments(const std::vector<std::unique_ptr<VkImageObj>> &color_atts, VkDepthStencilObj *depth_stencil_att);
    void BindDescriptorSet(VkDescriptorSetObj &descriptorSet);
    void BindIndexBuffer(VkBufferObj *indexBuffer, VkDeviceSize offset, VkIndexType indexType);
    void BindVertexBuffer(VkConstantBufferObj *vertexBuffer, VkDeviceSize offset, uint32_t binding);
    void BeginRenderPass(const VkRenderPassBeginInfo &info, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void NextSubpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void EndRenderPass();
    void BeginRendering(const VkRenderingInfoKHR &renderingInfo);
    void EndRendering();
    void FillBuffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize fill_size, uint32_t data);
    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                     uint32_t firstInstance);
    void QueueCommandBuffer(bool check_success = true);
    void QueueCommandBuffer(const VkFenceObj &fence, bool check_success = true, bool submit_2 = false);
    void SetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport *pViewports);
    void SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference);
    void UpdateBuffer(VkBuffer buffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData);
    void CopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                   uint32_t regionCount, const VkImageCopy *pRegions);
    void ResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                      uint32_t regionCount, const VkImageResolve *pRegions);
    void ClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue *pColor, uint32_t rangeCount,
                         const VkImageSubresourceRange *pRanges);
    void ClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue *pColor,
                                uint32_t rangeCount, const VkImageSubresourceRange *pRanges);
    void BuildAccelerationStructure(VkAccelerationStructureObj *as, VkBuffer scratchBuffer);
    void BuildAccelerationStructure(VkAccelerationStructureObj *as, VkBuffer scratchBuffer, VkBuffer instanceData);
    void SetEvent(VkEventObj &event, VkPipelineStageFlags stageMask) { event.cmd_set(*this, stageMask); }
    void ResetEvent(VkEventObj &event, VkPipelineStageFlags stageMask) { event.cmd_reset(*this, stageMask); }
    void WaitEvents(uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                    VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
        vk::CmdWaitEvents(handle(), eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                          bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }

  protected:
    VkDeviceObj *m_device;
    VkQueueObj *m_queue;
};

class VkConstantBufferObj : public VkBufferObj {
  public:
    VkConstantBufferObj(VkDeviceObj *device,
                        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkConstantBufferObj(VkDeviceObj *device, VkDeviceSize size, const void *data,
                        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkDescriptorBufferInfo m_descriptorBufferInfo;

  protected:
    VkDeviceObj *m_device;
};

class VkRenderpassObj : public vk_testing::RenderPass {
  public:
    VkRenderpassObj(VkDeviceObj *device, VkFormat format = VK_FORMAT_B8G8R8A8_UNORM);
    VkRenderpassObj(VkDeviceObj *device, VkFormat format, bool depthStencil);
};

class VkImageObj : public vk_testing::Image {
  public:
    VkImageObj(VkDeviceObj *dev);
    bool IsCompatible(VkImageUsageFlags usages, VkFormatFeatureFlags2 features);

  public:
    static VkImageCreateInfo ImageCreateInfo2D(uint32_t const width, uint32_t const height, uint32_t const mipLevels,
                                               uint32_t const layers, VkFormat const format, VkFlags const usage,
                                               VkImageTiling const requested_tiling = VK_IMAGE_TILING_LINEAR,
                                               const std::vector<uint32_t> *queue_families = nullptr);
    void Init(uint32_t const width, uint32_t const height, uint32_t const mipLevels, VkFormat const format, VkFlags const usage,
              VkImageTiling const tiling = VK_IMAGE_TILING_LINEAR, VkMemoryPropertyFlags const reqs = 0,
              const std::vector<uint32_t> *queue_families = nullptr, bool memory = true);
    void Init(const VkImageCreateInfo &create_info, VkMemoryPropertyFlags const reqs = 0, bool memory = true);

    void init(const VkImageCreateInfo *create_info);
    void init_no_mem(const vk_testing::Device &dev, const VkImageCreateInfo &info);

    void InitNoLayout(uint32_t const width, uint32_t const height, uint32_t const mipLevels, VkFormat const format,
                      VkFlags const usage, VkImageTiling tiling = VK_IMAGE_TILING_LINEAR, VkMemoryPropertyFlags reqs = 0,
                      const std::vector<uint32_t> *queue_families = nullptr, bool memory = true);

    void InitNoLayout(const VkImageCreateInfo &create_info, VkMemoryPropertyFlags reqs = 0, bool memory = true);

    //    void clear( CommandBuffer*, uint32_t[4] );

    void Layout(VkImageLayout const layout) { m_descriptorImageInfo.imageLayout = layout; }

    VkDeviceMemory memory() const { return Image::memory().handle(); }

    void *MapMemory() { return Image::memory().map(); }

    void UnmapMemory() { Image::memory().unmap(); }

    void ImageMemoryBarrier(VkCommandBufferObj *cmd, VkImageAspectFlags aspect, VkFlags output_mask, VkFlags input_mask,
                            VkImageLayout image_layout, VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED);

    VkResult CopyImage(VkImageObj &src_image);

    VkResult CopyImageOut(VkImageObj &dst_image);

    std::array<std::array<uint32_t, 16>, 16> Read();

    VkImage image() const { return handle(); }

    VkImageViewCreateInfo TargetViewCI(VkFormat format) const {
        auto ci = LvlInitStruct<VkImageViewCreateInfo>();
        ci.format = format;
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.components.r = VK_COMPONENT_SWIZZLE_R;
        ci.components.g = VK_COMPONENT_SWIZZLE_G;
        ci.components.b = VK_COMPONENT_SWIZZLE_B;
        ci.components.a = VK_COMPONENT_SWIZZLE_A;
        ci.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,                          // base mip level
            VK_REMAINING_MIP_LEVELS,    // level count
            0,                          // base array layer
            VK_REMAINING_ARRAY_LAYERS,  // layer count
        };
        ci.flags = 0;
        return ci;
    }

    VkImageView targetView(VkImageViewCreateInfo ci) {
        if (!m_targetView.initialized()) {
            ci.image = handle();
            m_targetView.init(*m_device, ci);
        }
        return m_targetView.handle();
    }

    VkImageView targetView(VkFormat format, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t baseMipLevel = 0,
                           uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseArrayLayer = 0,
                           uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D) {
        if (!m_targetView.initialized()) {
            VkImageViewCreateInfo createView = LvlInitStruct<VkImageViewCreateInfo>();
            createView.image = handle();
            createView.viewType = type;
            createView.format = format;
            createView.components.r = VK_COMPONENT_SWIZZLE_R;
            createView.components.g = VK_COMPONENT_SWIZZLE_G;
            createView.components.b = VK_COMPONENT_SWIZZLE_B;
            createView.components.a = VK_COMPONENT_SWIZZLE_A;
            createView.subresourceRange = {aspect, baseMipLevel, levelCount, baseArrayLayer, layerCount};
            createView.flags = 0;
            m_targetView.init(*m_device, createView);
        }
        return m_targetView.handle();
    }

    void SetLayout(VkCommandBufferObj *cmd_buf, VkImageAspectFlags aspect, VkImageLayout image_layout);
    void SetLayout(VkImageAspectFlags aspect, VkImageLayout image_layout);
    void SetLayout(VkImageLayout image_layout) { SetLayout(aspect_mask(), image_layout); };

    VkImageLayout Layout() const { return m_descriptorImageInfo.imageLayout; }
    uint32_t width() const { return extent().width; }
    uint32_t height() const { return extent().height; }
    VkDeviceObj *device() const { return m_device; }

  protected:
    VkDeviceObj *m_device;

    vk_testing::ImageView m_targetView;
    VkDescriptorImageInfo m_descriptorImageInfo;
    uint32_t m_mipLevels;
    uint32_t m_arrayLayers;
};

class VkTextureObj : public VkImageObj {
  public:
    VkTextureObj(VkDeviceObj *device, uint32_t *colors = NULL);

    const VkDescriptorImageInfo &DescriptorImageInfo() const { return m_descriptorImageInfo; }

  protected:
    VkDeviceObj *m_device;
    vk_testing::ImageView m_textureView;
};

class VkDepthStencilObj : public VkImageObj {
  public:
    VkDepthStencilObj(VkDeviceObj *device);
    void Init(VkDeviceObj *device, int32_t width, int32_t height, VkFormat format,
              VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VkImageAspectFlags aspect = 0);
    bool Initialized();
    VkImageView *BindInfo();

    VkFormat Format() const;

  protected:
    VkDeviceObj *m_device;
    bool m_initialized;
    vk_testing::ImageView m_imageView;
    VkFormat m_depth_stencil_fmt;
    VkImageView m_attachmentBindInfo;
};

class VkSamplerObj : public vk_testing::Sampler {
  public:
    VkSamplerObj(VkDeviceObj *device);

  protected:
    VkDeviceObj *m_device;
};

class VkDescriptorSetLayoutObj : public vk_testing::DescriptorSetLayout {
  public:
    VkDescriptorSetLayoutObj() = default;
    VkDescriptorSetLayoutObj(const VkDeviceObj *device,
                             const std::vector<VkDescriptorSetLayoutBinding> &descriptor_set_bindings = {},
                             VkDescriptorSetLayoutCreateFlags flags = 0, void *pNext = NULL);

    // Move constructor and move assignment operator for Visual Studio 2013
    VkDescriptorSetLayoutObj(VkDescriptorSetLayoutObj &&src) NOEXCEPT : DescriptorSetLayout(std::move(src)){};
    VkDescriptorSetLayoutObj &operator=(VkDescriptorSetLayoutObj &&src) NOEXCEPT {
        DescriptorSetLayout::operator=(std::move(src));
        return *this;
    }
};

class VkDescriptorSetObj : public vk_testing::DescriptorPool {
  public:
    VkDescriptorSetObj(VkDeviceObj *device);
    ~VkDescriptorSetObj() NOEXCEPT;

    int AppendDummy();
    int AppendBuffer(VkDescriptorType type, VkConstantBufferObj &constantBuffer);
    int AppendSamplerTexture(VkSamplerObj *sampler, VkTextureObj *texture);
    void CreateVKDescriptorSet(VkCommandBufferObj *commandBuffer);

    VkDescriptorSet GetDescriptorSetHandle() const;
    VkPipelineLayout GetPipelineLayout() const;
    VkDescriptorSetLayout GetDescriptorSetLayout() const;

  protected:
    VkDeviceObj *m_device;
    std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings;
    std::map<VkDescriptorType, int> m_type_counts;
    int m_nextSlot;

    std::vector<VkDescriptorImageInfo> m_imageSamplerDescriptors;
    std::vector<VkWriteDescriptorSet> m_writes;

    vk_testing::DescriptorSetLayout m_layout;
    vk_testing::PipelineLayout m_pipeline_layout;
    vk_testing::DescriptorSet *m_set = NULL;
};

// What is the incoming source to be turned into VkShaderModuleCreateInfo::pCode
typedef enum {
    SPV_SOURCE_GLSL,
    SPV_SOURCE_ASM,
    // TRY == Won't try in contructor as need to be called as function that can return the VkResult
    SPV_SOURCE_GLSL_TRY,
    SPV_SOURCE_ASM_TRY,
} SpvSourceType;

class VkShaderObj : public vk_testing::ShaderModule {
  public:
    // optional arguments listed order of most likely to be changed manually by a test
    VkShaderObj(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage,
                const spv_target_env env = SPV_ENV_VULKAN_1_0, SpvSourceType source_type = SPV_SOURCE_GLSL,
                const VkSpecializationInfo *spec_info = nullptr, char const *name = "main", bool debug = false);
    VkPipelineShaderStageCreateInfo const &GetStageCreateInfo() const;

    bool InitFromGLSL(bool debug = false);
    VkResult InitFromGLSLTry(bool debug = false, const VkDeviceObj *custom_device = nullptr);
    bool InitFromASM();
    VkResult InitFromASMTry();

    // These functions return a pointer to a newly created _and initialized_ VkShaderObj if initialization was successful.
    // Otherwise, {} is returned.
    static std::unique_ptr<VkShaderObj> CreateFromGLSL(VkRenderFramework &framework, VkShaderStageFlagBits stage,
                                                       const std::string &code, const char *entry_point = "main",
                                                       const VkSpecializationInfo *spec_info = nullptr,
                                                       const spv_target_env = SPV_ENV_VULKAN_1_0, bool debug = false);
    static std::unique_ptr<VkShaderObj> CreateFromASM(VkRenderFramework &framework, VkShaderStageFlagBits stage,
                                                      const std::string &code, const char *entry_point = "main",
                                                      const VkSpecializationInfo *spec_info = nullptr,
                                                      const spv_target_env spv_env = SPV_ENV_VULKAN_1_0);

    // TODO (ncesario) remove ifndef once android build consolidation changes go in
#ifndef __ANDROID__
    struct GlslangTargetEnv {
        GlslangTargetEnv(const spv_target_env env) {
            switch (env) {
                case SPV_ENV_UNIVERSAL_1_0:
                    language_version = glslang::EShTargetSpv_1_0;
                    break;
                case SPV_ENV_UNIVERSAL_1_1:
                    language_version = glslang::EShTargetSpv_1_1;
                    break;
                case SPV_ENV_UNIVERSAL_1_2:
                    language_version = glslang::EShTargetSpv_1_2;
                    break;
                case SPV_ENV_UNIVERSAL_1_3:
                    language_version = glslang::EShTargetSpv_1_3;
                    break;
                case SPV_ENV_UNIVERSAL_1_4:
                    language_version = glslang::EShTargetSpv_1_4;
                    break;
                case SPV_ENV_UNIVERSAL_1_5:
                    language_version = glslang::EShTargetSpv_1_5;
                    break;
                case SPV_ENV_VULKAN_1_0:
                    client_version = glslang::EShTargetVulkan_1_0;
                    break;
                case SPV_ENV_VULKAN_1_1:
                    client_version = glslang::EShTargetVulkan_1_1;
                    language_version = glslang::EShTargetSpv_1_3;
                    break;
                case SPV_ENV_VULKAN_1_2:
                    client_version = glslang::EShTargetVulkan_1_2;
                    language_version = glslang::EShTargetSpv_1_5;
                    break;
                default:
                    break;
            }
        }

        operator glslang::EShTargetLanguageVersion() const {
            return language_version;
        }

        operator glslang::EShTargetClientVersion() const {
            return client_version;
        }

      private:
        glslang::EShTargetLanguageVersion language_version = glslang::EShTargetSpv_1_0;
        glslang::EShTargetClientVersion client_version = glslang::EShTargetVulkan_1_0;
    };
#endif

  protected:
    VkPipelineShaderStageCreateInfo m_stage_info;
    VkRenderFramework &m_framework;
    VkDeviceObj &m_device;
    const char *m_source;
    spv_target_env m_spv_env;
};

class VkPipelineLayoutObj : public vk_testing::PipelineLayout {
  public:
    VkPipelineLayoutObj() = default;
    VkPipelineLayoutObj(VkDeviceObj *device, const std::vector<const VkDescriptorSetLayoutObj *> &descriptor_layouts = {},
                        const std::vector<VkPushConstantRange> &push_constant_ranges = {},
                        VkPipelineLayoutCreateFlags flags = static_cast<VkPipelineLayoutCreateFlags>(0));

    // Move constructor and move assignment operator for Visual Studio 2013
    VkPipelineLayoutObj(VkPipelineLayoutObj &&src) NOEXCEPT : PipelineLayout(std::move(src)) {}
    VkPipelineLayoutObj &operator=(VkPipelineLayoutObj &&src) NOEXCEPT {
        PipelineLayout::operator=(std::move(src));
        return *this;
    }

    void Reset();
};

class VkPipelineObj : public vk_testing::Pipeline {
  public:
    VkPipelineObj(VkDeviceObj *device);
    void AddShader(VkShaderObj *shaderObj);
    void AddShader(VkPipelineShaderStageCreateInfo const &createInfo);
    void AddVertexInputAttribs(VkVertexInputAttributeDescription *vi_attrib, uint32_t count);
    void AddVertexInputBindings(VkVertexInputBindingDescription *vi_binding, uint32_t count);
    void AddColorAttachment(uint32_t binding, const VkPipelineColorBlendAttachmentState &att);
    void MakeDynamic(VkDynamicState state);

    void AddDefaultColorAttachment(VkColorComponentFlags writeMask = 0xf /*=R|G|B|A*/) {
        VkPipelineColorBlendAttachmentState att = {};
        att.blendEnable = VK_FALSE;
        att.colorWriteMask = writeMask;
        AddColorAttachment(0, att);
    }

    void SetDepthStencil(const VkPipelineDepthStencilStateCreateInfo *);
    void SetMSAA(const VkPipelineMultisampleStateCreateInfo *ms_state);
    void SetInputAssembly(const VkPipelineInputAssemblyStateCreateInfo *ia_state);
    void SetRasterization(const VkPipelineRasterizationStateCreateInfo *rs_state);
    void SetTessellation(const VkPipelineTessellationStateCreateInfo *te_state);
    void SetViewport(const std::vector<VkViewport> viewports);
    void SetScissor(const std::vector<VkRect2D> scissors);
    void SetLineState(const VkPipelineRasterizationLineStateCreateInfoEXT *line_state);

    void InitGraphicsPipelineCreateInfo(VkGraphicsPipelineCreateInfo *gp_ci);

    VkResult CreateVKPipeline(VkPipelineLayout layout, VkRenderPass render_pass, VkGraphicsPipelineCreateInfo *gp_ci = nullptr);

  protected:
    VkPipelineVertexInputStateCreateInfo m_vi_state;
    VkPipelineInputAssemblyStateCreateInfo m_ia_state;
    VkPipelineRasterizationStateCreateInfo m_rs_state;
    VkPipelineColorBlendStateCreateInfo m_cb_state;
    VkPipelineDepthStencilStateCreateInfo const *m_ds_state;
    VkPipelineViewportStateCreateInfo m_vp_state;
    VkPipelineMultisampleStateCreateInfo m_ms_state;
    VkPipelineTessellationStateCreateInfo const *m_te_state;
    VkPipelineDynamicStateCreateInfo m_pd_state;
    VkPipelineRasterizationLineStateCreateInfoEXT m_line_state;
    std::vector<VkDynamicState> m_dynamic_state_enables;
    std::vector<VkViewport> m_viewports;
    std::vector<VkRect2D> m_scissors;
    VkDeviceObj *m_device;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
    std::vector<VkPipelineColorBlendAttachmentState> m_colorAttachments;
};

#endif  // VKRENDERFRAMEWORK_H
