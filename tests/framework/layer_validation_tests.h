/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include <vulkan/vulkan.h>

#include "../layers/vk_lunarg_device_profile_api_layer.h"
#include "vk_layer_settings_ext.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/log.h>
#include <android_native_app_glue.h>
#endif

#include "icd-spv.h"
#include "test_common.h"
#include "vk_layer_config.h"
#include "containers/custom_containers.h"
#include "generated/vk_format_utils.h"
#include "generated/vk_extension_helper.h"
#include "render.h"
#include "generated/vk_typemap_helper.h"
#include "utils/convert_to_renderpass2.h"

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

#if defined(VVL_ENABLE_ASAN)
#if __has_include(<sanitizer/lsan_interface.h>)
#include <sanitizer/lsan_interface.h>
#else
#error The lsan_interface.h header was not found!
#endif
#endif

//--------------------------------------------------------------------------------------
// Mesh and VertexFormat Data
//--------------------------------------------------------------------------------------

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
    #version 460
    void main() {}
)glsl";

static const char bindStateVertShaderText[] = R"glsl(
    #version 460
    void main() {
       gl_Position = vec4(1);
    }
)glsl";

static const char bindStateVertPointSizeShaderText[] = R"glsl(
    #version 460
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
    #version 460
    layout(triangles) in;
    layout(triangle_strip, max_vertices=3) out;
    void main() {
       gl_Position = vec4(1);
       EmitVertex();
    }
)glsl";

static char const bindStateGeomPointSizeShaderText[] = R"glsl(
    #version 460
    layout (points) in;
    layout (points) out;
    layout (max_vertices = 1) out;
    in gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
    };
    void main() {
       gl_Position = vec4(1);
       gl_PointSize = 1.0;
       EmitVertex();
    }
)glsl";

static const char bindStateTscShaderText[] = R"glsl(
    #version 460
    layout(vertices=3) out;
    void main() {
       gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
       gl_TessLevelInner[0] = 1;
    }
)glsl";

static const char bindStateTeshaderText[] = R"glsl(
    #version 460
    layout(triangles, equal_spacing, cw) in;
    void main() { gl_Position = vec4(1); }
)glsl";

static const char bindStateFragShaderText[] = R"glsl(
    #version 460
    layout(location = 0) out vec4 uFragColor;
    void main(){
       uFragColor = vec4(0,1,0,1);
    }
)glsl";

static const char bindStateFragSamplerShaderText[] = R"glsl(
    #version 460
    layout(set=0, binding=0) uniform sampler2D s;
    layout(location=0) out vec4 x;
    void main(){
       x = texture(s, vec2(1));
    }
)glsl";

static const char bindStateFragUniformShaderText[] = R"glsl(
    #version 460
    layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;
    layout(location=0) out vec4 x;
    void main(){
       x = vec4(bar.y);
    }
)glsl";

static char const bindStateFragSubpassLoadInputText[] = R"glsl(
    #version 460
    layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
    void main() {
        vec4 color = subpassLoad(x);
    }
)glsl";

static char const bindStateFragColorOutputText[] = R"glsl(
    #version 460
    layout(location=0) out vec4 color;
    void main() {
        color = vec4(1.0f);
    }
)glsl";

[[maybe_unused]] static const char *bindStateMeshShaderText = R"glsl(
    #version 460
    #extension GL_EXT_mesh_shader : require // Requires SPIR-V 1.5 (Vulkan 1.2)
    layout(max_vertices = 3, max_primitives=1) out;
    layout(triangles) out;
    void main() {}
)glsl";

[[maybe_unused]] static const char *bindStateRTShaderText = R"glsl(
    #version 460
    #extension GL_EXT_ray_tracing : require // Requires SPIR-V 1.5 (Vulkan 1.2)
    void main() {}
)glsl";

[[maybe_unused]] static const char *bindStateRTNVShaderText = R"glsl(
    #version 460
    #extension GL_NV_ray_tracing : require
    void main() {}
)glsl";

static char const bindShaderTileImageDepthReadSpv[] = R"(
               OpCapability Shader
               OpCapability TileImageDepthReadAccessEXT
               OpExtension "SPV_EXT_shader_tile_image"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %depth_output
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main EarlyFragmentTests
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_tile_image"
               OpName %main "main"
               OpName %depth "depth"
               OpName %depth_output "depth_output"
               OpDecorate %depth_output Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%depth_output = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
      %depth = OpVariable %_ptr_Function_float Function
          %9 = OpDepthAttachmentReadEXT %float
               OpStore %depth %9
         %13 = OpLoad %float %depth
         %14 = OpCompositeConstruct %v4float %13 %13 %13 %13
               OpStore %depth_output %14
               OpReturn
               OpFunctionEnd
        )";

static char const bindShaderTileImageStencilReadSpv[] = R"(
               OpCapability Shader
               OpCapability TileImageStencilReadAccessEXT
               OpExtension "SPV_EXT_shader_tile_image"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %stencil_output
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main EarlyFragmentTests
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_tile_image"
               OpName %main "main"
               OpName %stencil "stencil"
               OpName %stencil_output "stencil_output"
               OpDecorate %9 RelaxedPrecision
               OpDecorate %stencil_output Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%stencil_output = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
    %stencil = OpVariable %_ptr_Function_uint Function
          %9 = OpStencilAttachmentReadEXT %uint
               OpStore %stencil %9
         %14 = OpLoad %uint %stencil
         %15 = OpConvertUToF %float %14
         %16 = OpCompositeConstruct %v4float %15 %15 %15 %15
               OpStore %stencil_output %16
               OpReturn
               OpFunctionEnd
        )";

static char const bindShaderTileImageColorReadSpv[] = R"(
               OpCapability Shader
               OpCapability TileImageColorReadAccessEXT
               OpExtension "SPV_EXT_shader_tile_image"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color_output
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_tile_image"
               OpName %main "main"
               OpName %color_output "color_output"
               OpName %color_f "color_f"
               OpDecorate %color_output Location 0
               OpDecorate %color_f Location 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%color_output = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float TileImageDataEXT 0 0 0 2 Unknown
%_ptr_TileImageEXT_10 = OpTypePointer TileImageEXT %10
    %color_f = OpVariable %_ptr_TileImageEXT_10 TileImageEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %color_f
         %14 = OpColorAttachmentReadEXT %v4float %13
               OpStore %color_output %14
               OpReturn
               OpFunctionEnd
        )";

static char const bindShaderTileImageDepthStencilReadSpv[] = R"(
               OpCapability Shader
               OpCapability TileImageDepthReadAccessEXT
               OpCapability TileImageStencilReadAccessEXT
               OpExtension "SPV_EXT_shader_tile_image"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %uFragColor
               OpExecutionMode %main OriginUpperLeft
               OpExecutionMode %main EarlyFragmentTests
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_shader_tile_image"
               OpName %main "main"
               OpName %depth "depth"
               OpName %stencil "stencil"
               OpName %uFragColor "uFragColor"
               OpDecorate %13 RelaxedPrecision
               OpDecorate %uFragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %uFragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %3
          %5 = OpLabel
      %depth = OpVariable %_ptr_Function_float Function
    %stencil = OpVariable %_ptr_Function_uint Function
          %9 = OpDepthAttachmentReadEXT %float
               OpStore %depth %9
         %13 = OpStencilAttachmentReadEXT %uint
               OpStore %stencil %13
         %17 = OpLoad %float %depth
         %18 = OpLoad %uint %stencil
         %19 = OpConvertUToF %float %18
         %22 = OpCompositeConstruct %v4float %17 %19 %float_0 %float_1
               OpStore %uFragColor %22
               OpReturn
               OpFunctionEnd
        )";

// Static arrays helper
template <class ElementT, size_t array_size>
size_t size(ElementT (&)[array_size]) {
    return array_size;
}

template <class ElementT, size_t array_size>
uint32_t size32(ElementT (&)[array_size]) {
    return static_cast<uint32_t>(array_size);
}

template <class Container>
uint32_t size32(const Container &c) {
    return static_cast<uint32_t>(c.size());
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

bool CheckSynchronization2SupportAndInitState(VkRenderFramework *render_framework, void *phys_dev_pnext = nullptr);

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
    void AddSurfaceExtension();
    VkCommandBufferObj *CommandBuffer();
    void OOBRayTracingShadersTestBody(bool gpu_assisted);

    template <typename Features>
    VkPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures2(Features &feature_query) {
        auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&feature_query);
        return GetPhysicalDeviceFeatures2(features2);
    }

    template <typename Properties>
    VkPhysicalDeviceProperties2 GetPhysicalDeviceProperties2(Properties &props_query) {
        auto props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&props_query);
        return GetPhysicalDeviceProperties2(props2);
    }

    template <typename Proc, bool assert_proc = true>
    [[nodiscard]] const Proc GetInstanceProcAddr(const char *proc_name) const noexcept {
        static_assert(std::is_pointer_v<Proc>);

        auto proc = reinterpret_cast<Proc>(vk::GetInstanceProcAddr(instance(), proc_name));
        if constexpr (assert_proc) {
            assert(proc);
        }
        return proc;
    }

    template <typename Proc, bool assert_proc = true>
    [[nodiscard]] const Proc GetDeviceProcAddr(const char *proc_name) noexcept {
        static_assert(std::is_pointer_v<Proc>);

        auto proc = reinterpret_cast<Proc>(vk::GetDeviceProcAddr(device(), proc_name));
        if constexpr (assert_proc) {
            assert(proc);
        }
        return proc;
    }

    bool IsDriver(VkDriverId driver_id);

  protected:
    APIVersion m_instance_api_version = 0;
    APIVersion m_target_api_version = 0;
    APIVersion m_attempted_api_version = 0;

    void SetTargetApiVersion(APIVersion target_api_version);
    APIVersion DeviceValidationVersion() const;
    bool LoadDeviceProfileLayer(
        PFN_vkSetPhysicalDeviceFormatPropertiesEXT &fpvkSetPhysicalDeviceFormatPropertiesEXT,
        PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT &fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT);
    bool LoadDeviceProfileLayer(
        PFN_vkSetPhysicalDeviceFormatProperties2EXT &fpvkSetPhysicalDeviceFormatProperties2EXT,
        PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT &fpvkGetOriginalPhysicalDeviceFormatProperties2EXT);
    bool LoadDeviceProfileLayer(PFN_vkSetPhysicalDeviceLimitsEXT &fpvkSetPhysicalDeviceLimitsEXT,
                                PFN_vkGetOriginalPhysicalDeviceLimitsEXT &fpvkGetOriginalPhysicalDeviceLimitsEXT);
    bool LoadDeviceProfileLayer(PFN_vkSetPhysicalDeviceFeaturesEXT &fpvkSetPhysicalDeviceFeaturesEXT,
                                PFN_vkGetOriginalPhysicalDeviceFeaturesEXT &fpvkGetOriginalPhysicalDeviceFeaturesEXT);
    bool LoadDeviceProfileLayer(PFN_VkSetPhysicalDeviceProperties2EXT &fpvkSetPhysicalDeviceProperties2EXT);

    VkLayerTest();
};

template <>
VkPhysicalDeviceFeatures2 VkLayerTest::GetPhysicalDeviceFeatures2(VkPhysicalDeviceFeatures2 &feature_query);

template <>
VkPhysicalDeviceProperties2 VkLayerTest::GetPhysicalDeviceProperties2(VkPhysicalDeviceProperties2 &props2);

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
class VkNvidiaBestPracticesLayerTest : public VkBestPracticesLayerTest {};

class VkGpuAssistedLayerTest : public VkLayerTest {
  public:
    VkValidationFeaturesEXT GetValidationFeatures();
    void ShaderBufferSizeTest(VkDeviceSize buffer_size, VkDeviceSize binding_offset, VkDeviceSize binding_range,
                              VkDescriptorType descriptor_type, const char *fragment_shader, const char *expected_error);

  protected:
    bool CanEnableGpuAV();
};

class NegativeDebugPrintf : public VkLayerTest {
  public:
    void InitDebugPrintfFramework();

  protected:
};

class VkSyncValTest : public VkLayerTest {
  public:
    void InitSyncValFramework(bool enable_queue_submit_validation = false);

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
    std::optional<VkGraphicsPipelineLibraryCreateInfoEXT> gpl_info;
    CreatePipelineHelper(VkLayerTest &test, uint32_t color_attachments_count = 1u);
    ~CreatePipelineHelper();

    void InitDescriptorSetInfo();
    void InitInputAndVertexInfo();
    void InitMultisampleInfo();
    void InitPipelineLayoutInfo();
    void InitViewportInfo();
    void InitDynamicStateInfo();
    void InitShaderInfo();
    void ResetShaderInfo(const char *vertex_shader_text, const char *fragment_shader_text);
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
    void InitPipelineCache();
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
    template <typename Test, typename OverrideFunc, typename ErrorContainer>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const ErrorContainer &errors) {
        CreatePipelineHelper helper(test);
        helper.InitInfo();
        info_override(helper);
        helper.InitState();

        for (const auto &error : errors) test.Monitor().SetDesiredFailureMsg(flags, error);
        helper.CreateGraphicsPipeline();

        if (!errors.empty()) {
            test.Monitor().VerifyFound();
        }
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const char *error) {
        std::array errors = {error};
        OneshotTest(test, info_override, flags, errors);
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const std::string &error) {
        std::array errors = {error};
        OneshotTest(test, info_override, flags, errors);
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags) {
        std::array<const char *, 0> errors;
        OneshotTest(test, info_override, flags, errors);
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
    void InitPipelineCache();
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

        if (!errors.empty()) {
            test.Monitor().VerifyFound();
        }
    }

    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, Error error) {
        OneshotTest(test, info_override, flags, std::vector<Error>(1, error));
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags) {
        OneshotTest(test, info_override, flags, std::vector<std::string>{});
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
    void InitPipelineCache();
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

        ASSERT_VK_SUCCESS(helper.CreateNVRayTracingPipeline());
    }
};

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

    static const uint32_t kInvalidQueueFamily = vvl::kU32Max;
    Context *context_;
    VkImageObj image_;
    VkBufferObj buffer_;
};

class BarrierQueueFamilyTestHelper : public BarrierQueueFamilyBase {
  public:
    BarrierQueueFamilyTestHelper(Context *context) : BarrierQueueFamilyBase(context) {}
    // Init with queue families non-null for CONCURRENT sharing mode (which requires them)
    void Init(std::vector<uint32_t> *families, bool image_memory = true, bool buffer_memory = true);

    void operator()(const std::string &img_err, const std::string &buf_err = "", uint32_t src = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t dst = VK_QUEUE_FAMILY_IGNORED, uint32_t queue_family_index = kInvalidQueueFamily,
                    Modifier mod = Modifier::NONE);

    void operator()(uint32_t src = VK_QUEUE_FAMILY_IGNORED, uint32_t dst = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t queue_family_index = kInvalidQueueFamily, Modifier mod = Modifier::NONE) {
        (*this)("", "", src, dst, queue_family_index, mod);
    }

    VkImageMemoryBarrier image_barrier_;
    VkBufferMemoryBarrier buffer_barrier_;
};

class Barrier2QueueFamilyTestHelper : public BarrierQueueFamilyBase {
  public:
    Barrier2QueueFamilyTestHelper(Context *context) : BarrierQueueFamilyBase(context) {}
    // Init with queue families non-null for CONCURRENT sharing mode (which requires them)
    void Init(std::vector<uint32_t> *families, bool image_memory = true, bool buffer_memory = true);

    void operator()(const std::string &img_err, const std::string &buf_err = "", uint32_t src = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t dst = VK_QUEUE_FAMILY_IGNORED, uint32_t queue_family_index = kInvalidQueueFamily,
                    Modifier mod = Modifier::NONE);

    void operator()(uint32_t src = VK_QUEUE_FAMILY_IGNORED, uint32_t dst = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t queue_family_index = kInvalidQueueFamily, Modifier mod = Modifier::NONE) {
        (*this)("", "", src, dst, queue_family_index, mod);
    }

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
struct ThreadTestData {
    VkCommandBuffer commandBuffer;
    VkDevice device;
    VkEvent event;
    VkDescriptorSet descriptorSet;
    VkBuffer buffer;
    uint32_t binding;
    std::atomic<bool> *bailout;
};

void AddToCommandBuffer(ThreadTestData *);
void UpdateDescriptor(ThreadTestData *);
#endif  // GTEST_IS_THREADSAFE

// Helper class that fails the tests with a stuck worker thread.
// Its purpose is similar to an assert. We assume the code is correct.
// Then, in case of a bug or regression, the CI will continue to operate.
// Usage Example:
//  TEST_F(VkLayerTest, MyTrickyTestWithThreads) {
//      // The constructor parameter is the number of the worker threads
//      ThreadTimeoutHelper timeout_helper(2);
//      auto worker = [&]() {
//          auto timeout_guard = timeout_helper.ThreadGuard();
//          // Some code here
//      };
//      std::thread t0(worker);
//      std::thread t1(worker);
//      if (!timeout_helper.WaitForThreads(60)) ADD_FAILURE() << "It's time to move on";
//      t0.join();
//      t1.join();
//  }
class ThreadTimeoutHelper {
  public:
    explicit ThreadTimeoutHelper(int thread_count = 1) : active_threads_(thread_count) {}
    bool WaitForThreads(int timeout_in_seconds);

    struct Guard {
        Guard(ThreadTimeoutHelper &timeout_helper) : timeout_helper_(timeout_helper) {}
        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;

        ~Guard() { timeout_helper_.OnThreadDone(); }

        ThreadTimeoutHelper &timeout_helper_;
    };
    // Mandatory elision of copy/move operations guarantees the destructor is not called
    // (even in the presence of copy/move constructor) and the object is constructed directly
    // into the destination storage: https://en.cppreference.com/w/cpp/language/copy_elision
    Guard ThreadGuard() { return Guard(*this); }

  private:
    void OnThreadDone();

    int active_threads_;
    std::condition_variable cv_;
    std::mutex mutex_;
};

void ReleaseNullFence(ThreadTestData *);

void TestRenderPassCreate(ErrorMonitor *error_monitor, const vk_testing::Device &device, const VkRenderPassCreateInfo &create_info,
                          bool rp2_supported, const char *rp1_vuid, const char *rp2_vuid);
void PositiveTestRenderPassCreate(ErrorMonitor *error_monitor, const vk_testing::Device &device,
                                  const VkRenderPassCreateInfo &create_info, bool rp2_supported);
void PositiveTestRenderPass2KHRCreate(const vk_testing::Device &device, const VkRenderPassCreateInfo2KHR &create_info);
void TestRenderPass2KHRCreate(ErrorMonitor &error_monitor, const vk_testing::Device &device,
                              const VkRenderPassCreateInfo2KHR &create_info, const std::initializer_list<const char *> &vuids);
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
                                   VkFormatFeatureFlags undesired_features = vvl::kU32Max);

VkExternalMemoryHandleTypeFlags FindSupportedExternalMemoryHandleTypes(VkPhysicalDevice gpu,
                                                                       const VkBufferCreateInfo &buffer_create_info,
                                                                       VkExternalMemoryFeatureFlags requested_features);

bool HandleTypeNeedsDedicatedAllocation(VkPhysicalDevice gpu, const VkBufferCreateInfo &buffer_create_info,
                                        VkExternalMemoryHandleTypeFlagBits handle_type);

VkExternalMemoryHandleTypeFlags FindSupportedExternalMemoryHandleTypes(VkPhysicalDevice gpu,
                                                                       const VkImageCreateInfo &image_create_info,
                                                                       VkExternalMemoryFeatureFlags requested_features);

VkExternalMemoryHandleTypeFlagsNV FindSupportedExternalMemoryHandleTypesNV(const VkLayerTest &test,
                                                                           const VkImageCreateInfo &image_create_info,
                                                                           VkExternalMemoryFeatureFlagsNV requested_features);

bool HandleTypeNeedsDedicatedAllocation(VkPhysicalDevice gpu, const VkImageCreateInfo &image_create_info,
                                        VkExternalMemoryHandleTypeFlagBits handle_type);

VkExternalFenceHandleTypeFlags FindSupportedExternalFenceHandleTypes(VkPhysicalDevice gpu,
                                                                     VkExternalFenceFeatureFlags requested_features);

VkExternalSemaphoreHandleTypeFlags FindSupportedExternalSemaphoreHandleTypes(VkPhysicalDevice gpu,
                                                                             VkExternalSemaphoreFeatureFlags requested_features);

VkExternalMemoryHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu, const VkBufferCreateInfo &buffer_create_info,
                                                         VkExternalMemoryHandleTypeFlagBits handle_type);

VkExternalMemoryHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu, const VkImageCreateInfo &image_create_info,
                                                         VkExternalMemoryHandleTypeFlagBits handle_type);

VkExternalFenceHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu, VkExternalFenceHandleTypeFlagBits handle_type);

VkExternalSemaphoreHandleTypeFlags GetCompatibleHandleTypes(VkPhysicalDevice gpu,
                                                            VkExternalSemaphoreHandleTypeFlagBits handle_type);

void SetImageLayout(VkDeviceObj *device, VkImageAspectFlags aspect, VkImage image, VkImageLayout image_layout);

void AllocateDisjointMemory(VkDeviceObj *device, PFN_vkGetImageMemoryRequirements2KHR fp, VkImage mp_image,
                            VkDeviceMemory *mp_image_mem, VkImageAspectFlagBits plane);

void NegHeightViewportTests(VkDeviceObj *m_device, VkCommandBufferObj *m_commandBuffer, ErrorMonitor *m_errorMonitor);

void CreateSamplerTest(VkLayerTest &test, const VkSamplerCreateInfo *pCreateInfo, const std::string &code = "");

void CreateBufferTest(VkLayerTest &test, const VkBufferCreateInfo *pCreateInfo, const std::string &code = "");

void CreateImageTest(VkLayerTest &test, const VkImageCreateInfo *pCreateInfo, const std::string &code = "");

void CreateBufferViewTest(VkLayerTest &test, const VkBufferViewCreateInfo *pCreateInfo, const std::vector<std::string> &codes);

void CreateImageViewTest(VkLayerTest &test, const VkImageViewCreateInfo *pCreateInfo, const std::string &code = "");

bool InitFrameworkForRayTracingTest(VkRenderFramework *framework, bool is_khr, VkPhysicalDeviceFeatures2KHR *features2 = nullptr,
                                    VkValidationFeaturesEXT *enabled_features = nullptr);

void GetSimpleGeometryForAccelerationStructureTests(const VkDeviceObj &device, VkBufferObj *vbo, VkBufferObj *ibo,
                                                    VkGeometryNV *geometry, VkDeviceSize offset = 0, bool buffer_device_address = false);

std::pair<VkBufferObj &&, VkAccelerationStructureGeometryKHR> GetSimpleAABB(const VkDeviceObj &device,
                                                                            APIVersion vk_api_version = VK_API_VERSION_1_2);

void print_android(const char *c);
