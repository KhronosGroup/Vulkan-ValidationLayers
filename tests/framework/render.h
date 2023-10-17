/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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

#pragma once

#include "generated/vk_function_pointers.h"
#include "error_monitor.h"
#include "test_framework.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/log.h>
#include <android_native_app_glue.h>
#endif

#include <algorithm>
#include <array>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using vkt::MakeVkHandles;

static constexpr uint64_t kWaitTimeout{10000000000};  // 10 seconds in ns
static constexpr VkDeviceSize kZeroDeviceSize{0};

class VkImageObj;

struct SurfaceContext {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    HWND m_win32Window{};
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    Display *m_surface_dpy{};
    Window m_surface_window{};
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_connection_t *m_surface_xcb_conn{};
#endif
};

struct SurfaceInformation {
    VkSurfaceCapabilitiesKHR surface_capabilities{};
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> surface_present_modes;
    VkPresentModeKHR surface_non_shared_present_mode{};
    VkCompositeAlphaFlagBitsKHR surface_composite_alpha{};
};

class VkRenderFramework : public VkTestFramework {
  public:
    VkInstance instance() const { return instance_; }
    VkDevice device() const { return m_device->device(); }
    vkt::Device *DeviceObj() const { return m_device; }
    VkPhysicalDevice gpu() const;
    VkRenderPass renderPass() const { return m_renderPass; }
    const VkRenderPassCreateInfo &RenderPassInfo() const { return m_renderPass_info; };
    VkFramebuffer framebuffer() const { return m_framebuffer; }
    ErrorMonitor &Monitor();
    const VkPhysicalDeviceProperties &physDevProps() const;

    bool InstanceLayerSupported(const char *layer_name, uint32_t spec_version = 0, uint32_t impl_version = 0);
    bool InstanceExtensionSupported(const char *extension_name, uint32_t spec_version = 0);

    VkInstanceCreateInfo GetInstanceCreateInfo() const;
    void InitFramework(void *instance_pnext = NULL);
    void ShutdownFramework();

     // Functions to modify the VkRenderFramework surface & swapchain variables
    bool InitSurface();
    void DestroySurface();
    void InitSwapchainInfo();
    bool InitSwapchain(VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                       VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    void DestroySwapchain();
    // Functions to create surfaces and swapchains that *aren't* member variables of VkRenderFramework
    bool CreateSurface(SurfaceContext &surface_context, VkSurfaceKHR &surface, VkInstance custom_instance = VK_NULL_HANDLE);
    void DestroySurface(VkSurfaceKHR& surface);
    void DestroySurfaceContext(SurfaceContext& surface_context);
    SurfaceInformation GetSwapchainInfo(const VkSurfaceKHR surface);
    bool CreateSwapchain(VkSurfaceKHR &surface, VkImageUsageFlags imageUsage,  VkSurfaceTransformFlagBitsKHR preTransform, VkSwapchainKHR &swapchain, VkSwapchainKHR oldSwapchain = 0);
    std::vector<VkImage> GetSwapchainImages(const VkSwapchainKHR swapchain);

    void InitRenderTarget();
    void InitRenderTarget(uint32_t targets);
    void InitRenderTarget(VkImageView *dsBinding);
    void InitRenderTarget(uint32_t targets, VkImageView *dsBinding);
    void InitDynamicRenderTarget(VkFormat format = VK_FORMAT_UNDEFINED);
    VkImageView GetDynamicRenderTarget() const;
    void DestroyRenderTarget();

    static bool IgnoreDisableChecks();
    bool IsPlatformMockICD();
    void GetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures *features);
    void GetPhysicalDeviceProperties(VkPhysicalDeviceProperties *props);
    VkFormat GetRenderTargetFormat();
    // default to CommandPool Reset flag to allow recording multiple command buffers simpler
    void InitState(VkPhysicalDeviceFeatures *features = nullptr, void *create_device_pnext = nullptr,
                   const VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    const VkRenderPassBeginInfo &renderPassBeginInfo() const { return m_renderPassBeginInfo; }

    bool DeviceExtensionSupported(const char *extension_name, uint32_t spec_version = 0) const;
    bool DeviceExtensionSupported(VkPhysicalDevice, const char *, const char *name,
                                  uint32_t spec_version = 0) const {  // deprecated
        return DeviceExtensionSupported(name, spec_version);
    }

    // Tracks ext_name to be enabled at device creation time and attempts to enable any required instance extensions.
    // Does not return anything as will be checked when creating the framework
    // `ext_name` can refer to a device or instance extension.
    void AddRequiredExtensions(const char *ext_name);
    // Ensures at least 1 WSI instance extension is enabled
    void AddWsiExtensions(const char *ext_name);
    // Same as AddRequiredExtensions but won't skip test if not found
    void AddOptionalExtensions(const char *ext_name);
    // After instance and physical device creation (e.g., after InitFramework), returns if extension was enabled
    bool IsExtensionsEnabled(const char *ext_name) const;
    // if requested extensions are not supported, helper function to get string to print out
    std::string RequiredExtensionsNotSupported() const;

    void *SetupValidationSettings(void *first_pnext);

    template <typename GLSLContainer>
    std::vector<uint32_t> GLSLToSPV(VkShaderStageFlagBits stage, const GLSLContainer &code, const char *entry_point = "main",
                                    const VkSpecializationInfo *spec_info = nullptr, const spv_target_env env = SPV_ENV_VULKAN_1_0,
                                    bool debug = false) {
        std::vector<uint32_t> spv;
        GLSLtoSPV(&m_device->phy().limits_, stage, code, spv, debug, env);
        return spv;
    }

  protected:
    APIVersion m_instance_api_version = 0;
    APIVersion m_target_api_version = 0;
    APIVersion m_attempted_api_version = 0;

    VkRenderFramework();
    virtual ~VkRenderFramework() = 0;

    std::vector<VkLayerProperties> available_layers_; // allow caching of available layers
    std::vector<VkExtensionProperties> available_extensions_; // allow caching of available instance extensions

    ErrorMonitor monitor_ = ErrorMonitor(m_print_vu);
    ErrorMonitor *m_errorMonitor = &monitor_;  // TODO: Removing this properly is it's own PR. It's a big change.

    VkApplicationInfo app_info_;
    std::vector<const char *> instance_layers_;
    std::vector<const char *> m_instance_extension_names;
    VkInstance instance_;
    VkPhysicalDevice gpu_ = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties physDevProps_;

    uint32_t m_gpu_index;
    vkt::Device *m_device;
    vkt::CommandPool *m_commandPool;
    vkt::CommandBuffer *m_commandBuffer;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkRenderPassCreateInfo m_renderPass_info = {};
    std::vector<VkAttachmentDescription> m_renderPass_attachments;
    std::vector<VkSubpassDescription> m_renderPass_subpasses;
    std::vector<VkSubpassDependency> m_renderPass_dependencies;

    VkFramebuffer m_framebuffer;
    VkFramebufferCreateInfo m_framebuffer_info;
    std::vector<VkImageView> m_framebuffer_attachments;

    // WSI items
    SurfaceContext m_surface_context{};
    VkSurfaceKHR m_surface{};
    VkSwapchainKHR m_swapchain{};
    VkSurfaceCapabilitiesKHR m_surface_capabilities{};
    std::vector<VkSurfaceFormatKHR> m_surface_formats;
    std::vector<VkPresentModeKHR> m_surface_present_modes;
    VkPresentModeKHR m_surface_non_shared_present_mode{};
    VkCompositeAlphaFlagBitsKHR m_surface_composite_alpha{};

    std::vector<VkViewport> m_viewports;
    std::vector<VkRect2D> m_scissors;
    bool m_addRenderPassSelfDependency;
    std::vector<VkSubpassDependency> m_additionalSubpassDependencies;
    std::vector<VkClearValue> m_renderPassClearValues;
    VkRenderPassBeginInfo m_renderPassBeginInfo;
    std::vector<std::unique_ptr<VkImageObj>> m_renderTargets;
    uint32_t m_width, m_height;
    VkFormat m_render_target_fmt;
    VkFormat m_depth_stencil_fmt;
    VkImageLayout m_depth_stencil_layout;
    VkClearColorValue m_clear_color;
    bool m_clear_via_load_op;
    float m_depth_clear_color;
    uint32_t m_stencil_clear_color;
    VkImageObj *m_depthStencil;
    // first graphics queue, used must often, don't overwrite, use Device class
    VkQueue m_default_queue;

    // Requested extensions to enable at device creation time
    std::vector<const char *> m_required_extensions;
    // Optional extensions to try and enable at device creation time
    std::vector<const char *> m_optional_extensions;
    // wsi extensions to try and enable
    std::vector<const char *> m_wsi_extensions;
    // Device extensions to enable
    std::vector<const char *> m_device_extension_names;

    VkValidationFeaturesEXT m_validation_features;
    VkValidationFeatureEnableEXT validation_enable_all[4] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                                             VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
                                                             VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
                                                             VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
    VkValidationFeatureDisableEXT validation_disable_all = VK_VALIDATION_FEATURE_DISABLE_ALL_EXT;

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
    // `dev_ext_name` must be an instance extension name; false is returned for all instance extension names.
    bool CanEnableDeviceExtension(const std::string &dev_ext_name) const;
};

class VkDescriptorSetObj;

class VkImageObj : public vkt::Image {
  public:
    VkImageObj(vkt::Device *dev);
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
    void init_no_mem(const vkt::Device &dev, const VkImageCreateInfo &info);

    void InitNoLayout(uint32_t const width, uint32_t const height, uint32_t const mipLevels, VkFormat const format,
                      VkFlags const usage, VkImageTiling tiling = VK_IMAGE_TILING_LINEAR, VkMemoryPropertyFlags reqs = 0,
                      const std::vector<uint32_t> *queue_families = nullptr, bool memory = true);

    void InitNoLayout(const VkImageCreateInfo &create_info, VkMemoryPropertyFlags reqs = 0, bool memory = true);

    //    void clear( CommandBuffer*, uint32_t[4] );

    void Layout(VkImageLayout const layout) { m_descriptorImageInfo.imageLayout = layout; }

    VkDeviceMemory Memory() const { return Image::memory().handle(); }

    void *MapMemory() { return Image::memory().map(); }

    void UnmapMemory() { Image::memory().unmap(); }

    void ImageMemoryBarrier(vkt::CommandBuffer *cmd, VkImageAspectFlags aspect, VkFlags output_mask, VkFlags input_mask,
                            VkImageLayout image_layout, VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED);

    VkResult CopyImage(VkImageObj &src_image);

    VkResult CopyImageOut(VkImageObj &dst_image);

    std::array<std::array<uint32_t, 16>, 16> Read();

    VkImage image() const { return handle(); }

    VkImageViewCreateInfo BasicViewCreatInfo(VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT) const {
        VkImageViewCreateInfo ci = vku::InitStructHelper();
        ci.image = handle();
        ci.format = format();
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.components.r = VK_COMPONENT_SWIZZLE_R;
        ci.components.g = VK_COMPONENT_SWIZZLE_G;
        ci.components.b = VK_COMPONENT_SWIZZLE_B;
        ci.components.a = VK_COMPONENT_SWIZZLE_A;
        ci.subresourceRange = {aspect_mask, 0, 1, 0, 1};
        ci.flags = 0;
        return ci;
    }

    const VkImageView &targetView(VkImageViewCreateInfo ci) {
        if (!m_targetView.initialized()) {
            ci.image = handle();
            m_targetView.init(*m_device, ci);
        }
        return m_targetView.handle();
    }

    const VkImageView &targetView(VkFormat format, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t baseMipLevel = 0,
                                  uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseArrayLayer = 0,
                                  uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D) {
        if (!m_targetView.initialized()) {
            VkImageViewCreateInfo createView = vku::InitStructHelper();
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

    void SetLayout(vkt::CommandBuffer *cmd_buf, VkImageAspectFlags aspect, VkImageLayout image_layout);
    void SetLayout(VkImageAspectFlags aspect, VkImageLayout image_layout);
    void SetLayout(VkImageLayout image_layout) { SetLayout(aspect_mask(), image_layout); };

    VkImageLayout Layout() const { return m_descriptorImageInfo.imageLayout; }
    uint32_t width() const { return extent().width; }
    uint32_t height() const { return extent().height; }
    vkt::Device *device() const { return m_device; }

  protected:
    vkt::Device *m_device;

    vkt::ImageView m_targetView;
    VkDescriptorImageInfo m_descriptorImageInfo;
    uint32_t m_mipLevels;
    uint32_t m_arrayLayers;
};

class VkDescriptorSetObj : public vkt::DescriptorPool {
  public:
    VkDescriptorSetObj(vkt::Device *device);
    ~VkDescriptorSetObj() noexcept;

    int AppendDummy();
    int AppendSamplerTexture(VkDescriptorImageInfo &image_info);
    void CreateVKDescriptorSet(vkt::CommandBuffer *commandBuffer);

    VkDescriptorSet GetDescriptorSetHandle() const { return m_set ? m_set->handle() : VK_NULL_HANDLE; }
    VkPipelineLayout GetPipelineLayout() const { return m_pipeline_layout.handle(); }
    VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_layout.handle(); }

  protected:
    vkt::Device *m_device;
    std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings;
    std::map<VkDescriptorType, int> m_type_counts;
    int m_nextSlot;

    std::vector<VkDescriptorImageInfo> m_imageSamplerDescriptors;
    std::vector<VkWriteDescriptorSet> m_writes;

    vkt::DescriptorSetLayout m_layout;
    vkt::PipelineLayout m_pipeline_layout;
    vkt::DescriptorSet *m_set = NULL;
};

// What is the incoming source to be turned into VkShaderModuleCreateInfo::pCode
typedef enum {
    SPV_SOURCE_GLSL,
    SPV_SOURCE_ASM,
    // TRY == Won't try in contructor as need to be called as function that can return the VkResult
    SPV_SOURCE_GLSL_TRY,
    SPV_SOURCE_ASM_TRY,
} SpvSourceType;

class VkShaderObj : public vkt::ShaderModule {
  public:
    // optional arguments listed order of most likely to be changed manually by a test
    VkShaderObj(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage,
                const spv_target_env env = SPV_ENV_VULKAN_1_0, SpvSourceType source_type = SPV_SOURCE_GLSL,
                const VkSpecializationInfo *spec_info = nullptr, char const *entry_point = "main", bool debug = false,
                const void *pNext = nullptr);
    VkPipelineShaderStageCreateInfo const &GetStageCreateInfo() const;

    bool InitFromGLSL(bool debug = false, const void *pNext = nullptr);
    VkResult InitFromGLSLTry(bool debug = false, const vkt::Device *custom_device = nullptr);
    bool InitFromASM();
    VkResult InitFromASMTry();

    // These functions return a pointer to a newly created _and initialized_ VkShaderObj if initialization was successful.
    // Otherwise, {} is returned.
    static std::unique_ptr<VkShaderObj> CreateFromGLSL(VkRenderFramework *framework, const char *source,
                                                       VkShaderStageFlagBits stage, const spv_target_env = SPV_ENV_VULKAN_1_0,
                                                       const VkSpecializationInfo *spec_info = nullptr,
                                                       const char *entry_point = "main", bool debug = false);
    static std::unique_ptr<VkShaderObj> CreateFromASM(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage,
                                                      const spv_target_env spv_env = SPV_ENV_VULKAN_1_0,
                                                      const VkSpecializationInfo *spec_info = nullptr,
                                                      const char *entry_point = "main");

  protected:
    VkPipelineShaderStageCreateInfo m_stage_info;
    VkRenderFramework &m_framework;
    vkt::Device &m_device;
    const char *m_source;
    spv_target_env m_spv_env;
};
