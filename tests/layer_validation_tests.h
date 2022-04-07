/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
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

#ifndef VKLAYERTEST_H
#define VKLAYERTEST_H

#ifdef ANDROID
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif

#include "layers/vk_device_profile_api_layer.h"
#include "vk_layer_settings_ext.h"

#if defined(ANDROID)
#include <android/log.h>
#if defined(VALIDATION_APK)
#include <android_native_app_glue.h>
#endif
#endif

#include "icd-spv.h"
#include "test_common.h"
#include "vk_layer_config.h"
#include "vk_format_utils.h"
#include "vkrenderframework.h"
#include "vk_typemap_helper.h"
#include "convert_to_renderpass2.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

using std::string;
using std::vector;

//--------------------------------------------------------------------------------------
// Mesh and VertexFormat Data
//--------------------------------------------------------------------------------------

static const char kSkipPrefix[] = "             TEST SKIPPED:";

enum BsoFailSelect {
    BsoFailNone,
    BsoFailLineWidth,
    BsoFailDepthBias,
    BsoFailViewport,
    BsoFailScissor,
    BsoFailBlend,
    BsoFailDepthBounds,
    BsoFailStencilReadMask,
    BsoFailStencilWriteMask,
    BsoFailStencilReference,
    BsoFailCmdClearAttachments,
    BsoFailIndexBuffer,
    BsoFailIndexBufferBadSize,
    BsoFailIndexBufferBadOffset,
    BsoFailIndexBufferBadMapSize,
    BsoFailIndexBufferBadMapOffset,
    BsoFailLineStipple,
};

static const char bindStateMinimalShaderText[] = R"glsl(
    #version 450
    void main() {}
)glsl";

static const char bindStateVertShaderText[] = R"glsl(
    #version 450
    void main() {
       gl_Position = vec4(1);
    }
)glsl";

static const char bindStateVertPointSizeShaderText[] = R"glsl(
    #version 450
    out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
    };
    void main() {
        gl_Position = vec4(1);
        gl_PointSize = 1.0;
    }
)glsl";

static char const bindStateGeomShaderText[] = R"glsl(
    #version 450
    layout(triangles) in;
    layout(triangle_strip, max_vertices=3) out;
    void main() {
       gl_Position = vec4(1);
       EmitVertex();
    }
)glsl";

static char const bindStateGeomPointSizeShaderText[] = R"glsl(
    #version 450
    layout (points) in;
    layout (points) out;
    layout (max_vertices = 1) out;
    void main() {
       gl_Position = vec4(1);
       gl_PointSize = 1.0;
       EmitVertex();
    }
)glsl";

static const char bindStateTscShaderText[] = R"glsl(
    #version 450
    layout(vertices=3) out;
    void main() {
       gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
       gl_TessLevelInner[0] = 1;
    }
)glsl";

static const char bindStateTeshaderText[] = R"glsl(
    #version 450
    layout(triangles, equal_spacing, cw) in;
    void main() { gl_Position = vec4(1); }
)glsl";

static const char bindStateFragShaderText[] = R"glsl(
    #version 450
    layout(location = 0) out vec4 uFragColor;
    void main(){
       uFragColor = vec4(0,1,0,1);
    }
)glsl";

static const char bindStateFragSamplerShaderText[] = R"glsl(
    #version 450
    layout(set=0, binding=0) uniform sampler2D s;
    layout(location=0) out vec4 x;
    void main(){
       x = texture(s, vec2(1));
    }
)glsl";

static const char bindStateFragUniformShaderText[] = R"glsl(
    #version 450
    layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;
    layout(location=0) out vec4 x;
    void main(){
       x = vec4(bar.y);
    }
)glsl";

// Static arrays helper
template <class ElementT, size_t array_size>
size_t size(ElementT (&)[array_size]) {
    return array_size;
}

// Format search helper
VkFormat FindSupportedDepthOnlyFormat(VkPhysicalDevice phy);
VkFormat FindSupportedStencilOnlyFormat(VkPhysicalDevice phy);
VkFormat FindSupportedDepthStencilFormat(VkPhysicalDevice phy);

// Returns true if *any* requested features are available.
// Assumption is that the framework can successfully create an image as
// long as at least one of the feature bits is present (excepting VTX_BUF).
bool ImageFormatIsSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                            VkFormatFeatureFlags features = ~VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT);

// Returns true if format and *all* requested features are available.
bool ImageFormatAndFeaturesSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features);

// Returns true if format and *all* requested features are available.
bool ImageFormatAndFeaturesSupported(const VkInstance inst, const VkPhysicalDevice phy, const VkImageCreateInfo info,
                                     const VkFormatFeatureFlags features);

// Returns true if format and *all* requested features are available.
bool BufferFormatAndFeaturesSupported(VkPhysicalDevice phy, VkFormat format, VkFormatFeatureFlags features);

// Simple sane SamplerCreateInfo boilerplate
VkSamplerCreateInfo SafeSaneSamplerCreateInfo();

VkImageViewCreateInfo SafeSaneImageViewCreateInfo(VkImage image, VkFormat format, VkImageAspectFlags aspect_mask);

VkImageViewCreateInfo SafeSaneImageViewCreateInfo(const VkImageObj &image, VkFormat format, VkImageAspectFlags aspect_mask);

// Helper for checking createRenderPass2 support and adding related extensions.
bool CheckCreateRenderPass2Support(VkRenderFramework *renderFramework, std::vector<const char *> &device_extension_names);

// Helper for checking descriptor_indexing support and adding related extensions.
bool CheckDescriptorIndexingSupportAndInitFramework(VkRenderFramework *renderFramework,
                                                    std::vector<const char *> &instance_extension_names,
                                                    std::vector<const char *> &device_extension_names,
                                                    VkValidationFeaturesEXT *features, void *userData);

// Helper for checking timeline semaphore support and initializing
bool CheckTimelineSemaphoreSupportAndInitState(VkRenderFramework *renderFramework);

bool CheckSynchronization2SupportAndInitState(VkRenderFramework *renderFramework);

// Dependent "false" type for the static assert, as GCC will evaluate
// non-dependent static_asserts even for non-instantiated templates
template <typename T>
struct AlwaysFalse : std::false_type {};

// Helpers to get nearest greater or smaller value (of float) -- useful for testing the boundary cases of Vulkan limits
template <typename T>
T NearestGreater(const T from) {
    using Lim = std::numeric_limits<T>;
    const auto positive_direction = Lim::has_infinity ? Lim::infinity() : Lim::max();

    return std::nextafter(from, positive_direction);
}

template <typename T>
T NearestSmaller(const T from) {
    using Lim = std::numeric_limits<T>;
    const auto negative_direction = Lim::has_infinity ? -Lim::infinity() : Lim::lowest();

    return std::nextafter(from, negative_direction);
}

class VkLayerTest : public VkRenderFramework {
  public:
    const char *kValidationLayerName = "VK_LAYER_KHRONOS_validation";
    const char *kSynchronization2LayerName = "VK_LAYER_KHRONOS_synchronization2";

    void VKTriangleTest(BsoFailSelect failCase);

    void GenericDrawPreparation(VkCommandBufferObj *commandBuffer, VkPipelineObj &pipelineobj, VkDescriptorSetObj &descriptorSet,
                                BsoFailSelect failCase);

    void Init(VkPhysicalDeviceFeatures *features = nullptr, VkPhysicalDeviceFeatures2 *features2 = nullptr,
              const VkCommandPoolCreateFlags flags = 0, void *instance_pnext = nullptr);
    bool AddSurfaceInstanceExtension();
    bool AddSwapchainDeviceExtension();
    VkCommandBufferObj *CommandBuffer();
    void OOBRayTracingShadersTestBody(bool gpu_assisted);

  protected:
    uint32_t m_instance_api_version = 0;
    uint32_t m_target_api_version = 0;
    bool m_enableWSI;

    uint32_t SetTargetApiVersion(uint32_t target_api_version);
    uint32_t DeviceValidationVersion();
    bool LoadDeviceProfileLayer(
        PFN_vkSetPhysicalDeviceFormatPropertiesEXT &fpvkSetPhysicalDeviceFormatPropertiesEXT,
        PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT &fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT);
    bool LoadDeviceProfileLayer(
        PFN_vkSetPhysicalDeviceFormatProperties2EXT &fpvkSetPhysicalDeviceFormatProperties2EXT,
        PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT &fpvkGetOriginalPhysicalDeviceFormatProperties2EXT);
    bool LoadDeviceProfileLayer(PFN_vkSetPhysicalDeviceLimitsEXT &fpvkSetPhysicalDeviceLimitsEXT,
                                PFN_vkGetOriginalPhysicalDeviceLimitsEXT &fpvkGetOriginalPhysicalDeviceLimitsEXT);

    VkLayerTest();
};

class VkPositiveLayerTest : public VkLayerTest {
  public:
  protected:
};

class VkBestPracticesLayerTest : public VkLayerTest {
  public:
    void InitBestPracticesFramework();
    void InitBestPracticesFramework(const char* ValidationChecksToEnable);

  protected:
    VkValidationFeatureEnableEXT enables_[1] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};
    VkValidationFeatureDisableEXT disables_[4] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features_ = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr, 1, enables_, 4, disables_};
};

class VkAmdBestPracticesLayerTest : public VkBestPracticesLayerTest {};
class VkArmBestPracticesLayerTest : public VkBestPracticesLayerTest {
  public:
    std::unique_ptr<VkImageObj> CreateImage(VkFormat format, const uint32_t width, const uint32_t height,
                                            VkImageUsageFlags attachment_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkRenderPass CreateRenderPass(VkFormat format, VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                  VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE);
    VkFramebuffer CreateFramebuffer(const uint32_t width, const uint32_t height, VkImageView image_view, VkRenderPass renderpass);
    VkSampler CreateDefaultSampler();
};

class VkWsiEnabledLayerTest : public VkLayerTest {
  public:
  protected:
    VkWsiEnabledLayerTest() { m_enableWSI = true; }
};

class VkGpuAssistedLayerTest : public VkLayerTest {
  public:
    bool InitGpuAssistedFramework(bool request_descriptor_indexing);
    void ShaderBufferSizeTest(VkDeviceSize buffer_size, VkDeviceSize binding_offset, VkDeviceSize binding_range,
                              VkDescriptorType descriptor_type, const char *fragment_shader, const char *expected_error);

  protected:
};

class VkDebugPrintfTest : public VkLayerTest {
  public:
    void InitDebugPrintfFramework();

  protected:
};

class VkSyncValTest : public VkLayerTest {
  public:
    void InitSyncValFramework();

  protected:
    VkValidationFeatureEnableEXT enables_[1] = {VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
    VkValidationFeatureDisableEXT disables_[4] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features_ = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr, 1, enables_, 4, disables_};
};

class VkBufferTest {
  public:
    enum eTestEnFlags {
        eDoubleDelete,
        eInvalidDeviceOffset,
        eInvalidMemoryOffset,
        eBindNullBuffer,
        eBindFakeBuffer,
        eFreeInvalidHandle,
        eNone,
    };

    enum eTestConditions { eOffsetAlignment = 1 };

    static bool GetTestConditionValid(VkDeviceObj *aVulkanDevice, eTestEnFlags aTestFlag, VkBufferUsageFlags aBufferUsage = 0);
    // A constructor which performs validation tests within construction.
    VkBufferTest(VkDeviceObj *aVulkanDevice, VkBufferUsageFlags aBufferUsage, eTestEnFlags aTestFlag = eNone);
    ~VkBufferTest();
    bool GetBufferCurrent();
    const VkBuffer &GetBuffer();
    void TestDoubleDestroy();

  protected:
    bool AllocateCurrent;
    bool BoundCurrent;
    bool CreateCurrent;
    bool InvalidDeleteEn;

    VkBuffer VulkanBuffer;
    VkDevice VulkanDevice;
    VkDeviceMemory VulkanMemory;
};

struct CreatePipelineHelper;
class VkVerticesObj {
  public:
    VkVerticesObj(VkDeviceObj *aVulkanDevice, unsigned aAttributeCount, unsigned aBindingCount, unsigned aByteStride,
                  VkDeviceSize aVertexCount, const float *aVerticies);
    ~VkVerticesObj();
    bool AddVertexInputToPipe(VkPipelineObj &aPipelineObj);
    bool AddVertexInputToPipeHelpr(CreatePipelineHelper *pipelineHelper);
    void BindVertexBuffers(VkCommandBuffer aCommandBuffer, unsigned aOffsetCount = 0, VkDeviceSize *aOffsetList = nullptr);

  protected:
    static uint32_t BindIdGenerator;

    bool BoundCurrent;
    unsigned AttributeCount;
    unsigned BindingCount;
    uint32_t BindId;

    VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo;
    VkVertexInputAttributeDescription *VertexInputAttributeDescription;
    VkVertexInputBindingDescription *VertexInputBindingDescription;
    VkConstantBufferObj VulkanMemoryBuffer;
};

struct OneOffDescriptorSet {
    VkDeviceObj *device_;
    VkDescriptorPool pool_;
    VkDescriptorSetLayoutObj layout_;
    VkDescriptorSet set_;
    typedef std::vector<VkDescriptorSetLayoutBinding> Bindings;
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorImageInfo> image_infos;
    std::vector<VkBufferView> buffer_views;
    std::vector<VkWriteDescriptorSet> descriptor_writes;

    OneOffDescriptorSet(VkDeviceObj *device, const Bindings &bindings, VkDescriptorSetLayoutCreateFlags layout_flags = 0,
                        void *layout_pnext = NULL, VkDescriptorPoolCreateFlags poolFlags = 0, void *allocate_pnext = NULL,
                        int buffer_info_size = 10, int image_info_size = 10, int buffer_view_size = 10);
    ~OneOffDescriptorSet();
    bool Initialized();
    void Clear();
    void WriteDescriptorBufferInfo(int binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range,
                                   VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t arrayElement = 0,
                                   uint32_t count = 1);
    void WriteDescriptorBufferView(int binding, VkBufferView buffer_view,
                                   VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                                   uint32_t arrayElement = 0, uint32_t count = 1);
    void WriteDescriptorImageInfo(int binding, VkImageView image_view, VkSampler sampler,
                                  VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                  VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t arrayElement = 0,
                                  uint32_t count = 1);
    void UpdateDescriptorSets();
};

template <typename T>
bool IsValidVkStruct(const T &s) {
    return LvlTypeMap<T>::kSType == s.sType;
}

// Helper class for tersely creating create pipeline tests
//
// Designed with minimal error checking to ensure easy error state creation
// See OneshotTest for typical usage
struct CreatePipelineHelper {
  public:
    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings_;
    std::unique_ptr<OneOffDescriptorSet> descriptor_set_;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
    VkPipelineVertexInputStateCreateInfo vi_ci_ = {};
    VkPipelineInputAssemblyStateCreateInfo ia_ci_ = {};
    VkPipelineTessellationStateCreateInfo tess_ci_ = {};
    VkViewport viewport_ = {};
    VkRect2D scissor_ = {};
    VkPipelineViewportStateCreateInfo vp_state_ci_ = {};
    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci_ = {};
    VkPipelineLayoutCreateInfo pipeline_layout_ci_ = {};
    VkPipelineLayoutObj pipeline_layout_;
    VkPipelineDynamicStateCreateInfo dyn_state_ci_ = {};
    VkPipelineRasterizationStateCreateInfo rs_state_ci_ = {};
    VkPipelineRasterizationLineStateCreateInfoEXT line_state_ci_ = {};
    std::vector<VkPipelineColorBlendAttachmentState> cb_attachments_ = {};
    VkPipelineColorBlendStateCreateInfo cb_ci_ = {};
    VkPipelineDepthStencilStateCreateInfo ds_ci_ = {};
    VkGraphicsPipelineCreateInfo gp_ci_ = {};
    VkPipelineCacheCreateInfo pc_ci_ = {};
    VkPipeline pipeline_ = VK_NULL_HANDLE;
    VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
    std::unique_ptr<VkShaderObj> vs_;
    std::unique_ptr<VkShaderObj> fs_;
    VkLayerTest &layer_test_;
    layer_data::optional<VkGraphicsPipelineLibraryCreateInfoEXT> gpl_info;
    CreatePipelineHelper(VkLayerTest &test, uint32_t color_attachments_count = 1u);
    ~CreatePipelineHelper();

    void InitDescriptorSetInfo();
    void InitInputAndVertexInfo();
    void InitMultisampleInfo();
    void InitPipelineLayoutInfo();
    void InitViewportInfo();
    void InitDynamicStateInfo();
    void InitShaderInfo();
    void InitRasterizationInfo();
    void InitLineRasterizationInfo();
    void InitBlendStateInfo();
    void InitGraphicsPipelineInfo();
    void InitPipelineCacheInfo();

    // Not called by default during init_info
    void InitTesselationState();

    // TDB -- add control for optional and/or additional initialization
    void InitInfo();
    void InitState();
    void LateBindPipelineInfo();
    VkResult CreateGraphicsPipeline(bool implicit_destroy = true, bool do_late_bind = true);

    void InitVertexInputLibInfo(void *p_next = nullptr);

    template <typename StageContainer>
    void InitPreRasterLibInfoFromContainer(const StageContainer &stages, void *p_next = nullptr) {
        InitPreRasterLibInfo(static_cast<uint32_t>(stages.size()), stages.data(), p_next);
    }
    void InitPreRasterLibInfo(uint32_t count, const VkPipelineShaderStageCreateInfo *info, void *p_next = nullptr);

    template <typename StageContainer>
    void InitFragmentLibInfoFromContainer(const StageContainer &stages, void *p_next = nullptr) {
        InitFragmentLibInfo(static_cast<uint32_t>(stages.size()), stages.data(), p_next);
    }
    void InitFragmentLibInfo(uint32_t count, const VkPipelineShaderStageCreateInfo *info, void *p_next = nullptr);

    void InitFragmentOutputLibInfo(void *p_next = nullptr);

    // Helper function to create a simple test case (positive or negative)
    //
    // info_override can be any callable that takes a CreatePipelineHeper &
    // flags, error can be any args accepted by "SetDesiredFailure".
    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const std::vector<Error> &errors,
                            bool positive_test = false) {
        CreatePipelineHelper helper(test);
        helper.InitInfo();
        info_override(helper);
        helper.InitState();

        for (const auto &error : errors) test.Monitor().SetDesiredFailureMsg(flags, error);
        helper.CreateGraphicsPipeline();

        if (positive_test || (errors.size() == 0)) {
            test.Monitor().VerifyNotFound();
        } else {
            test.Monitor().VerifyFound();
        }
    }

    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, Error error,
                            bool positive_test = false) {
        OneshotTest(test, info_override, flags, std::vector<Error>(1, error), positive_test);
    }
};

struct CreateComputePipelineHelper {
  public:
    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings_;
    std::unique_ptr<OneOffDescriptorSet> descriptor_set_;
    VkPipelineLayoutCreateInfo pipeline_layout_ci_ = {};
    VkPipelineLayoutObj pipeline_layout_;
    VkComputePipelineCreateInfo cp_ci_ = {};
    VkPipelineCacheCreateInfo pc_ci_ = {};
    VkPipeline pipeline_ = VK_NULL_HANDLE;
    VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
    std::unique_ptr<VkShaderObj> cs_;
    bool override_skip_ = false;
    VkLayerTest &layer_test_;
    CreateComputePipelineHelper(VkLayerTest &test);
    ~CreateComputePipelineHelper();

    void InitDescriptorSetInfo();
    void InitPipelineLayoutInfo();
    void InitShaderInfo();
    void InitComputePipelineInfo();
    void InitPipelineCacheInfo();

    // TDB -- add control for optional and/or additional initialization
    void InitInfo();
    void InitState();
    void LateBindPipelineInfo();
    VkResult CreateComputePipeline(bool implicit_destroy = true, bool do_late_bind = true);

    // Helper function to create a simple test case (positive or negative)
    //
    // info_override can be any callable that takes a CreatePipelineHeper &
    // flags, error can be any args accepted by "SetDesiredFailure".
    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const std::vector<Error> &errors,
                            bool positive_test = false) {
        CreateComputePipelineHelper helper(test);
        helper.InitInfo();
        info_override(helper);
        // Allow lambda to decide if to skip trying to compile pipeline to prevent crashing
        if (helper.override_skip_) {
            helper.override_skip_ = false;  // reset
            return;
        }
        helper.InitState();

        for (const auto &error : errors) test.Monitor().SetDesiredFailureMsg(flags, error);
        helper.CreateComputePipeline();

        if (positive_test) {
            test.Monitor().VerifyNotFound();
        } else {
            test.Monitor().VerifyFound();
        }
    }

    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, Error error,
                            bool positive_test = false) {
        OneshotTest(test, info_override, flags, std::vector<Error>(1, error), positive_test);
    }
};

// Helper class for tersely creating create ray tracing pipeline tests
//
// Designed with minimal error checking to ensure easy error state creation
// See OneshotTest for typical usage
struct CreateNVRayTracingPipelineHelper {
  public:
    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings_;
    std::unique_ptr<OneOffDescriptorSet> descriptor_set_;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
    VkPipelineLayoutCreateInfo pipeline_layout_ci_ = {};
    VkPipelineLayoutObj pipeline_layout_;
    VkRayTracingPipelineCreateInfoNV rp_ci_ = {};
    VkRayTracingPipelineCreateInfoKHR rp_ci_KHR_ = {};
    VkPipelineCacheCreateInfo pc_ci_ = {};
    VkPipeline pipeline_ = VK_NULL_HANDLE;
    VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
    std::vector<VkRayTracingShaderGroupCreateInfoNV> groups_;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups_KHR_;
    std::unique_ptr<VkShaderObj> rgs_;
    std::unique_ptr<VkShaderObj> chs_;
    std::unique_ptr<VkShaderObj> mis_;
    VkLayerTest &layer_test_;
    CreateNVRayTracingPipelineHelper(VkLayerTest &test);
    ~CreateNVRayTracingPipelineHelper();

    static bool InitInstanceExtensions(VkLayerTest &test, std::vector<const char *> &instance_extension_names);
    static bool InitDeviceExtensions(VkLayerTest &test, std::vector<const char *> &device_extension_names);
    void InitShaderGroups();
    void InitShaderGroupsKHR();
    void InitDescriptorSetInfo();
    void InitDescriptorSetInfoKHR();
    void InitPipelineLayoutInfo();
    void InitShaderInfo();
    void InitShaderInfoKHR();
    void InitNVRayTracingPipelineInfo();
    void InitKHRRayTracingPipelineInfo();
    void InitPipelineCacheInfo();
    void InitInfo(bool isKHR = false);
    void InitState();
    void LateBindPipelineInfo(bool isKHR = false);
    VkResult CreateNVRayTracingPipeline(bool implicit_destroy = true, bool do_late_bind = true);
    VkResult CreateKHRRayTracingPipeline(bool implicit_destroy = true, bool do_late_bind = true);
    // Helper function to create a simple test case (positive or negative)
    //
    // info_override can be any callable that takes a CreateNVRayTracingPipelineHelper &
    // flags, error can be any args accepted by "SetDesiredFailure".
    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const std::vector<Error> &errors,
                            const VkFlags flags = kErrorBit) {
        CreateNVRayTracingPipelineHelper helper(test);
        helper.InitInfo();
        info_override(helper);
        helper.InitState();

        for (const auto &error : errors) test.Monitor().SetDesiredFailureMsg(flags, error);
        helper.CreateNVRayTracingPipeline();
        test.Monitor().VerifyFound();
    }

    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, Error error, const VkFlags flags = kErrorBit) {
        OneshotTest(test, info_override, std::vector<Error>(1, error), flags);
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotPositiveTest(Test &test, const OverrideFunc &info_override, const VkFlags message_flag_mask = kErrorBit) {
        CreateNVRayTracingPipelineHelper helper(test);
        helper.InitInfo();
        info_override(helper);
        helper.InitState();

        test.Monitor().ExpectSuccess(message_flag_mask);
        ASSERT_VK_SUCCESS(helper.CreateNVRayTracingPipeline());
        test.Monitor().VerifyNotFound();
    }
};

namespace chain_util {
template <typename T>
T Init(const void *pnext_in = nullptr) {
    T pnext_obj = {};
    pnext_obj.sType = LvlTypeMap<T>::kSType;
    pnext_obj.pNext = pnext_in;
    return pnext_obj;
}

class ExtensionChain {
    const void *head_ = nullptr;
    typedef std::function<bool(const char *)> AddIfFunction;
    AddIfFunction add_if_;
    typedef std::vector<const char *> List;
    List *list_;

  public:
    template <typename F>
    ExtensionChain(F &add_if, List *list) : add_if_(add_if), list_(list) {}

    template <typename T>
    void Add(const char *name, T &obj) {
        if (add_if_(name)) {
            if (list_) {
                list_->push_back(name);
            }
            obj.pNext = head_;
            head_ = &obj;
        }
    }

    const void *Head() const;
};
}  // namespace chain_util

// PushDescriptorProperties helper
VkPhysicalDevicePushDescriptorPropertiesKHR GetPushDescriptorProperties(VkInstance instance, VkPhysicalDevice gpu);

// Subgroup properties helper
VkPhysicalDeviceSubgroupProperties GetSubgroupProperties(VkInstance instance, VkPhysicalDevice gpu);

// Descriptor Indexing properties helper
VkPhysicalDeviceDescriptorIndexingProperties GetDescriptorIndexingProperties(VkInstance instance, VkPhysicalDevice gpu);

class BarrierQueueFamilyBase {
  public:
    struct QueueFamilyObjs {
        uint32_t index;
        // We would use std::unique_ptr, but this triggers a compiler error on older compilers
        VkQueueObj *queue = nullptr;
        VkCommandPoolObj *command_pool = nullptr;
        VkCommandBufferObj *command_buffer = nullptr;
        VkCommandBufferObj *command_buffer2 = nullptr;
        ~QueueFamilyObjs();
        void Init(VkDeviceObj *device, uint32_t qf_index, VkQueue qf_queue, VkCommandPoolCreateFlags cp_flags);
    };

    struct Context {
        VkLayerTest *layer_test;
        uint32_t default_index;
        std::unordered_map<uint32_t, QueueFamilyObjs> queue_families;
        Context(VkLayerTest *test, const std::vector<uint32_t> &queue_family_indices);
        void Reset();
    };

    BarrierQueueFamilyBase(Context *context) : context_(context), image_(context->layer_test->DeviceObj()) {}

    QueueFamilyObjs *GetQueueFamilyInfo(Context *context, uint32_t qfi);

    enum Modifier {
        NONE,
        DOUBLE_RECORD,
        DOUBLE_COMMAND_BUFFER,
    };

    static const uint32_t kInvalidQueueFamily = UINT32_MAX;
    Context *context_;
    VkImageObj image_;
    VkBufferObj buffer_;
};

class BarrierQueueFamilyTestHelper : public BarrierQueueFamilyBase {
  public:
    BarrierQueueFamilyTestHelper(Context *context) : BarrierQueueFamilyBase(context) {}
    // Init with queue families non-null for CONCURRENT sharing mode (which requires them)
    void Init(std::vector<uint32_t> *families, bool image_memory = true, bool buffer_memory = true);

    void operator()(std::string img_err, std::string buf_err = "", uint32_t src = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t dst = VK_QUEUE_FAMILY_IGNORED, bool positive = false,
                    uint32_t queue_family_index = kInvalidQueueFamily, Modifier mod = Modifier::NONE);

    VkImageMemoryBarrier image_barrier_;
    VkBufferMemoryBarrier buffer_barrier_;
};

class Barrier2QueueFamilyTestHelper : public BarrierQueueFamilyBase {
  public:
    Barrier2QueueFamilyTestHelper(Context *context) : BarrierQueueFamilyBase(context) {}
    // Init with queue families non-null for CONCURRENT sharing mode (which requires them)
    void Init(std::vector<uint32_t> *families, bool image_memory = true, bool buffer_memory = true);

    void operator()(std::string img_err, std::string buf_err = "", uint32_t src = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t dst = VK_QUEUE_FAMILY_IGNORED, bool positive = false,
                    uint32_t queue_family_index = kInvalidQueueFamily, Modifier mod = Modifier::NONE);

    VkImageMemoryBarrier2KHR image_barrier_;
    VkBufferMemoryBarrier2KHR buffer_barrier_;
};

struct DebugUtilsLabelCheckData {
    std::function<void(const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *)> callback;
    size_t count;
};

bool operator==(const VkDebugUtilsLabelEXT &rhs, const VkDebugUtilsLabelEXT &lhs);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                  VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

#if GTEST_IS_THREADSAFE
struct thread_data_struct {
    VkCommandBuffer commandBuffer;
    VkDevice device;
    VkEvent event;
    VkDescriptorSet descriptorSet;
    VkBuffer buffer;
    uint32_t binding;
    bool *bailout;
};

extern "C" void *AddToCommandBuffer(void *arg);
extern "C" void *UpdateDescriptor(void *arg);
#endif  // GTEST_IS_THREADSAFE

extern "C" void *ReleaseNullFence(void *arg);

void TestRenderPassCreate(ErrorMonitor *error_monitor, const VkDevice device, const VkRenderPassCreateInfo *create_info,
                          bool rp2_supported, const char *rp1_vuid, const char *rp2_vuid);
void PositiveTestRenderPassCreate(ErrorMonitor *error_monitor, const VkDevice device, const VkRenderPassCreateInfo *create_info,
                                  bool rp2_supported);
void PositiveTestRenderPass2KHRCreate(ErrorMonitor *error_monitor, const VkDevice device,
                                      const VkRenderPassCreateInfo2KHR *create_info);
void TestRenderPass2KHRCreate(ErrorMonitor *error_monitor, const VkDevice device, const VkRenderPassCreateInfo2KHR *create_info,
                              const char *rp2_vuid);
void TestRenderPassBegin(ErrorMonitor *error_monitor, const VkDevice device, const VkCommandBuffer command_buffer,
                         const VkRenderPassBeginInfo *begin_info, bool rp2Supported, const char *rp1_vuid, const char *rp2_vuid);

// Helpers for the tests below
void ValidOwnershipTransferOp(ErrorMonitor *monitor, VkCommandBufferObj *cb, VkPipelineStageFlags src_stages,
                              VkPipelineStageFlags dst_stages, const VkBufferMemoryBarrier *buf_barrier,
                              const VkImageMemoryBarrier *img_barrier);

void ValidOwnershipTransferOp(ErrorMonitor *monitor, VkCommandBufferObj *cb, const VkBufferMemoryBarrier2KHR *buf_barrier,
                              const VkImageMemoryBarrier2KHR *img_barrier);

void ValidOwnershipTransfer(ErrorMonitor *monitor, VkCommandBufferObj *cb_from, VkCommandBufferObj *cb_to,
                            VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages,
                            const VkBufferMemoryBarrier *buf_barrier, const VkImageMemoryBarrier *img_barrier);

void ValidOwnershipTransfer(ErrorMonitor *monitor, VkCommandBufferObj *cb_from, VkCommandBufferObj *cb_to,
                            const VkBufferMemoryBarrier2KHR *buf_barrier, const VkImageMemoryBarrier2KHR *img_barrier);

VkResult GPDIFPHelper(VkPhysicalDevice dev, const VkImageCreateInfo *ci, VkImageFormatProperties *limits = nullptr);

VkFormat FindFormatLinearWithoutMips(VkPhysicalDevice gpu, VkImageCreateInfo image_ci);

bool FindFormatWithoutSamples(VkPhysicalDevice gpu, VkImageCreateInfo &image_ci);

bool FindUnsupportedImage(VkPhysicalDevice gpu, VkImageCreateInfo &image_ci);

VkFormat FindFormatWithoutFeatures(VkPhysicalDevice gpu, VkImageTiling tiling,
                                   VkFormatFeatureFlags undesired_features = UINT32_MAX);

void SetImageLayout(VkDeviceObj *device, VkImageAspectFlags aspect, VkImage image, VkImageLayout image_layout);

void AllocateDisjointMemory(VkDeviceObj *device, PFN_vkGetImageMemoryRequirements2KHR fp, VkImage mp_image,
                            VkDeviceMemory *mp_image_mem, VkImageAspectFlagBits plane);

void NegHeightViewportTests(VkDeviceObj *m_device, VkCommandBufferObj *m_commandBuffer, ErrorMonitor *m_errorMonitor);

void CreateSamplerTest(VkLayerTest &test, const VkSamplerCreateInfo *pCreateInfo, std::string code = "");

void CreateBufferTest(VkLayerTest &test, const VkBufferCreateInfo *pCreateInfo, std::string code = "");

void CreateImageTest(VkLayerTest &test, const VkImageCreateInfo *pCreateInfo, std::string code = "");

void CreateBufferViewTest(VkLayerTest &test, const VkBufferViewCreateInfo *pCreateInfo, const std::vector<std::string> &codes);

void CreateImageViewTest(VkLayerTest &test, const VkImageViewCreateInfo *pCreateInfo, std::string code = "");

bool InitFrameworkForRayTracingTest(VkRenderFramework *renderFramework, bool isKHR,
                                    std::vector<const char *> &instance_extension_names,
                                    std::vector<const char *> &device_extension_names, void *user_data,
                                    bool need_gpu_validation = false, bool need_push_descriptors = false,
                                    bool deferred_state_init = false, VkPhysicalDeviceFeatures2KHR *features2 = nullptr);

void GetSimpleGeometryForAccelerationStructureTests(const VkDeviceObj &device, VkBufferObj *vbo, VkBufferObj *ibo,
                                                    VkGeometryNV *geometry, VkDeviceSize offset = 0);

void print_android(const char *c);
#endif  // VKLAYERTEST_H
