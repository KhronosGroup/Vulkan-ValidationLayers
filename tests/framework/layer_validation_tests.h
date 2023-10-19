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
#include <vulkan/layer/vk_layer_settings_ext.h>

#include "../layers/vk_lunarg_device_profile_api_layer.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/log.h>
#include <android_native_app_glue.h>
#endif

#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/utility/vk_struct_helper.hpp>

#include "test_common.h"
#include "containers/custom_containers.h"
#include "generated/vk_extension_helper.h"
#include "render.h"
#include "utils/convert_utils.h"
#include "shader_templates.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <condition_variable>

using std::string;
using std::vector;

// MSVC and GCC define __SANITIZE_ADDRESS__ when compiling with address sanitization
// However, clang doesn't. Instead you have to use __has_feature to check.
#if defined(__clang__)
#if __has_feature(address_sanitizer)
#define VVL_ENABLE_ASAN 1
#endif
#elif defined(__SANITIZE_ADDRESS__)
#define VVL_ENABLE_ASAN 1
#endif

#if defined(VVL_ENABLE_ASAN)
#if __has_include(<sanitizer/lsan_interface.h>)
#include <sanitizer/lsan_interface.h>
#else
#error The lsan_interface.h header was not found!
#endif
#endif

#define OBJECT_LAYER_NAME "VK_LAYER_KHRONOS_validation"

//--------------------------------------------------------------------------------------
// Mesh and VertexFormat Data
//--------------------------------------------------------------------------------------

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
bool FormatIsSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                       VkFormatFeatureFlags features = ~VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT);

// Returns true if format and *all* requested features are available.
bool FormatFeaturesAreSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features);

// Returns true if format and *all* requested features are available.
bool ImageFormatIsSupported(const VkInstance inst, const VkPhysicalDevice phy, const VkImageCreateInfo info,
                            const VkFormatFeatureFlags features);

// Returns true if format and *all* requested features are available.
bool BufferFormatAndFeaturesSupported(VkPhysicalDevice phy, VkFormat format, VkFormatFeatureFlags features);

// Simple sane SamplerCreateInfo boilerplate
VkSamplerCreateInfo SafeSaneSamplerCreateInfo();

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

// Defining VVL_TESTS_USE_CUSTOM_TEST_FRAMEWORK allows downstream users
// to inject custom test framework changes. This includes the ability
// to override the the base class of the VkLayerTest class so that
// appropriate test framework customizations can be injected into the
// class hierarchy at the closest possible place to the base class used
// by all validation layer tests. Downstream users can provide their
// own version of custom_test_framework.h to define the appropriate
// custom base class to use through the VkLayerTestBase type identifier.
#ifdef VVL_TESTS_USE_CUSTOM_TEST_FRAMEWORK
#include "framework/custom_test_framework.h"
#else
using VkLayerTestBase = VkRenderFramework;
#endif

// VkLayerTest is the main GTest test class
// It is the root for all other test class variations
class VkLayerTest : public VkLayerTestBase {
  public:
    const char *kValidationLayerName = "VK_LAYER_KHRONOS_validation";
    const char *kSynchronization2LayerName = "VK_LAYER_KHRONOS_synchronization2";

    void Init(VkPhysicalDeviceFeatures *features = nullptr, VkPhysicalDeviceFeatures2 *features2 = nullptr,
              void *instance_pnext = nullptr);
    void AddSurfaceExtension();
    vkt::CommandBuffer *CommandBuffer();

    template <typename Features>
    VkPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures2(Features &feature_query) {
        VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&feature_query);
        return GetPhysicalDeviceFeatures2(features2);
    }

    template <typename Properties>
    VkPhysicalDeviceProperties2 GetPhysicalDeviceProperties2(Properties &props_query) {
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&props_query);
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

// TODO - Want to remove - don't add to any new tests
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
};
class VkNvidiaBestPracticesLayerTest : public VkBestPracticesLayerTest {};

class VkGpuAssistedLayerTest : public virtual VkLayerTest {
  public:
    void InitGpuAvFramework();

    VkValidationFeaturesEXT GetValidationFeatures();
    void ShaderBufferSizeTest(VkDeviceSize buffer_size, VkDeviceSize binding_offset, VkDeviceSize binding_range,
                              VkDescriptorType descriptor_type, const char *fragment_shader, const char *expected_error, bool shader_objects = false);

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

class AndroidHardwareBufferTest : public VkLayerTest {};
class NegativeAndroidHardwareBuffer : public AndroidHardwareBufferTest {};
class PositiveAndroidHardwareBuffer : public AndroidHardwareBufferTest {};

class AndroidExternalResolveTest : public VkLayerTest {
  public:
    void InitBasicAndroidExternalResolve(void *pNextFeatures = nullptr);
    bool nullColorAttachmentWithExternalFormatResolve;
};
class NegativeAndroidExternalResolve : public AndroidExternalResolveTest {};
class PositiveAndroidExternalResolve : public AndroidExternalResolveTest {};

class AtomicTest : public VkLayerTest {};
class NegativeAtomic : public AtomicTest {};
class PositiveAtomic : public AtomicTest {};

class BufferTest : public VkLayerTest {};
class NegativeBuffer : public BufferTest {};
class PositiveBuffer : public BufferTest {};

class CommandTest : public VkLayerTest {};
class NegativeCommand : public CommandTest {};
class PositiveCommand : public CommandTest {};

class DescriptorsTest : public VkLayerTest {};
class NegativeDescriptors : public DescriptorsTest {};
class PositiveDescriptors : public DescriptorsTest {};

class PushDescriptorTest : public VkLayerTest {};
class NegativePushDescriptor : public PushDescriptorTest {};
class PositivePushDescriptor : public PushDescriptorTest {};

class DescriptorBufferTest : public VkLayerTest {
  public:
    void InitBasicDescriptorBuffer(void *pNextFeatures = nullptr);
};
class NegativeDescriptorBuffer : public DescriptorBufferTest {};
class PositiveDescriptorBuffer : public DescriptorBufferTest {};

class DescriptorIndexingTest : public VkLayerTest {
  public:
    void InitBasicDescriptorIndexing(void *pNextFeatures = nullptr);
    VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features;
    void ComputePipelineShaderTest(const char *shader, std::vector<VkDescriptorSetLayoutBinding> &bindings);
};
class NegativeDescriptorIndexing : public DescriptorIndexingTest {};
class PositiveDescriptorIndexing : public DescriptorIndexingTest {};

class NegativeDeviceQueue : public VkLayerTest {};

class DynamicRenderingTest : public VkLayerTest {
  public:
    void InitBasicDynamicRendering(void *pNextFeatures = nullptr);
};
class NegativeDynamicRendering : public DynamicRenderingTest {};
class PositiveDynamicRendering : public DynamicRenderingTest {};

class DynamicStateTest : public VkLayerTest {
  public:
    void InitBasicExtendedDynamicState();  // enables VK_EXT_extended_dynamic_state
    void InitBasicExtendedDynamicState3(VkPhysicalDeviceExtendedDynamicState3FeaturesEXT &features);
};
class NegativeDynamicState : public DynamicStateTest {
    // helper functions for tests in this file
  public:
    // VK_EXT_extended_dynamic_state - not calling vkCmdSet before draw
    void ExtendedDynamicStateDrawNotSet(VkDynamicState dynamic_state, const char *vuid);
    // VK_EXT_extended_dynamic_state3 - Create a pipeline with dynamic state, but the feature disabled
    void ExtendedDynamicState3PipelineFeatureDisabled(VkDynamicState dynamic_state, const char *vuid);
    // VK_EXT_line_rasterization - Init with LineRasterization features off
    void InitLineRasterizationFeatureDisabled();
};
class PositiveDynamicState : public DynamicStateTest {};

class ExternalMemorySyncTest : public VkLayerTest {
  protected:
#ifdef VK_USE_PLATFORM_WIN32_KHR
    using ExternalHandle = HANDLE;
#else
    using ExternalHandle = int;
#endif
};
class NegativeExternalMemorySync : public ExternalMemorySyncTest {};
class PositiveExternalMemorySync : public ExternalMemorySyncTest {};

class FragmentShadingRateTest : public VkLayerTest {};
class NegativeFragmentShadingRate : public FragmentShadingRateTest {};
class PositiveFragmentShadingRate : public FragmentShadingRateTest {};

class NegativeGeometryTessellation : public VkLayerTest {};
class PositiveGeometryTessellation : public VkLayerTest {};

class GraphicsLibraryTest : public VkLayerTest {
  public:
    void InitBasicGraphicsLibrary(void *pNextFeatures = nullptr);
};
class NegativeGraphicsLibrary : public GraphicsLibraryTest {};
class PositiveGraphicsLibrary : public GraphicsLibraryTest {};

class HostImageCopyTest : public VkLayerTest {
  public:
    void InitHostImageCopyTest(const VkImageCreateInfo &image_ci);
    bool CopyLayoutSupported(const std::vector<VkImageLayout> &copy_src_layouts, const std::vector<VkImageLayout> &copy_dst_layouts,
                             VkImageLayout layout);
    VkFormat compressed_format = VK_FORMAT_UNDEFINED;
    bool separate_depth_stencil = false;
    std::vector<VkImageLayout> copy_src_layouts;
    std::vector<VkImageLayout> copy_dst_layouts;
};
class NegativeHostImageCopy : public HostImageCopyTest {};
class PositiveHostImageCopy : public HostImageCopyTest {};

class ImageTest : public VkLayerTest {
  public:
    VkImageCreateInfo DefaultImageInfo();
};
class NegativeImage : public ImageTest {};
class PositiveImage : public ImageTest {};

class ImageDrmTest : public VkLayerTest {
  public:
    void InitBasicImageDrm(void *pNextFeatures = nullptr);
    std::vector<uint64_t> GetFormatModifier(VkFormat format, VkFormatFeatureFlags2 features, uint32_t plane_count = 1);
};
class NegativeImageDrm : public ImageDrmTest {};
class PositiveImageDrm : public ImageDrmTest {};

class ImagelessFramebufferTest : public VkLayerTest {};
class NegativeImagelessFramebuffer : public ImagelessFramebufferTest {};
class PositiveImagelessFramebuffer : public ImagelessFramebufferTest {};

class NegativeInstanceless : public VkLayerTest {};

class PositiveInstance : public VkLayerTest {};

class MemoryTest : public VkLayerTest {};
class NegativeMemory : public MemoryTest {};
class PositiveMemory : public MemoryTest {};

class MeshTest : public VkLayerTest {};
class NegativeMesh : public MeshTest {};
class PositiveMesh : public MeshTest {};

class NegativeMultiview : public VkLayerTest {};

class NegativeObjectLifetime : public VkLayerTest {};

class NegativePipelineAdvancedBlend : public VkLayerTest {};

class PipelineLayoutTest : public VkLayerTest {};
class NegativePipelineLayout : public PipelineLayoutTest {};
class PositivePipelineLayout : public PipelineLayoutTest {};

class PipelineTopologyTest : public VkLayerTest {};
class NegativePipelineTopology : public PipelineTopologyTest {};
class PositivePipelineTopology : public PipelineTopologyTest {};

class PipelineTest : public VkLayerTest {};
class NegativePipeline : public PipelineTest {};
class PositivePipeline : public PipelineTest {};

class NegativePortabilitySubset : public VkLayerTest {};

class ProtectedMemoryTest : public VkLayerTest {};
class NegativeProtectedMemory : public ProtectedMemoryTest {};
class PositiveProtectedMemory : public ProtectedMemoryTest {};

class QueryTest : public VkLayerTest {
  public:
    bool HasZeroTimestampValidBits();
};
class NegativeQuery : public QueryTest {};
class PositiveQuery : public QueryTest {};

class RayTracingTest : public virtual VkLayerTest {
  public:
    void InitFrameworkForRayTracingTest(bool is_khr, VkPhysicalDeviceFeatures2KHR *features2 = nullptr,
                                        VkValidationFeaturesEXT *enabled_features = nullptr);

    void OOBRayTracingShadersTestBody(bool gpu_assisted);
};
class NegativeRayTracing : public RayTracingTest {};
class PositiveRayTracing : public RayTracingTest {};
class NegativeRayTracingNV : public NegativeRayTracing {};
class PositiveRayTracingNV : public PositiveRayTracing {};

class RayTracingPipelineTest : public RayTracingTest {};
class NegativeRayTracingPipeline : public RayTracingPipelineTest {};
class PositiveRayTracingPipeline : public RayTracingPipelineTest {};
class NegativeRayTracingPipelineNV : public NegativeRayTracingPipeline {};
class PositiveRayTracingPipelineNV : public PositiveRayTracingPipeline {};

class GpuAssistedRayTracingTest : public VkGpuAssistedLayerTest, public RayTracingTest {};
class NegativeGpuAssistedRayTracing : public GpuAssistedRayTracingTest {};
class NegativeGpuAssistedRayTracingNV : public NegativeGpuAssistedRayTracing {};

class RenderPassTest : public VkLayerTest {};
class NegativeRenderPass : public RenderPassTest {};
class PositiveRenderPass : public RenderPassTest {};

class RobustnessTest : public VkLayerTest {};
class NegativeRobustness : public RobustnessTest {};
class PositiveRobustness : public RobustnessTest {};

class SamplerTest : public VkLayerTest {};
class NegativeSampler : public SamplerTest {};
class PositiveSampler : public SamplerTest {};

class ShaderComputeTest : public VkLayerTest {};
class NegativeShaderCompute : public ShaderComputeTest {};
class PositiveShaderCompute : public ShaderComputeTest {};

class ShaderObjectTest : public VkLayerTest {
    vkt::Buffer vertexBuffer;

  public:
    void InitBasicShaderObject(void *pNextFeatures = nullptr, APIVersion targetApiVersion = VK_API_VERSION_1_1,
                               bool coreFeatures = true);
    void InitBasicMeshShaderObject(void *pNextFeatures = nullptr, APIVersion targetApiVersion = VK_API_VERSION_1_1,
                                   bool taskShader = true, bool meshShader = true);
    void BindVertFragShader(const vkt::Shader &vertShader, const vkt::Shader &fragShader);
    void BindCompShader(const vkt::Shader &compShader);
    void SetDefaultDynamicStates(const std::vector<VkDynamicState>& exclude = {}, bool tessellation = false, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
};
class NegativeShaderObject : public ShaderObjectTest {};
class PositiveShaderObject : public ShaderObjectTest {};

class ShaderInterfaceTest : public VkLayerTest {};
class NegativeShaderInterface : public ShaderInterfaceTest {};
class PositiveShaderInterface : public ShaderInterfaceTest {};

class PositiveShaderImageAccess : public VkLayerTest {};

class ShaderLimitsTest : public VkLayerTest {};
class NegativeShaderLimits : public ShaderLimitsTest {};
class PositiveShaderLimits : public ShaderLimitsTest {};

class NegativeShaderMesh : public VkLayerTest {};

class ShaderPushConstantsTest : public VkLayerTest {};
class NegativeShaderPushConstants : public ShaderPushConstantsTest {};
class PositiveShaderPushConstants : public ShaderPushConstantsTest {};

class ShaderSpirvTest : public VkLayerTest {};
class NegativeShaderSpirv : public ShaderSpirvTest {};
class PositiveShaderSpirv : public ShaderSpirvTest {};

class ShaderStorageImageTest : public VkLayerTest {};
class NegativeShaderStorageImage : public ShaderStorageImageTest {};
class PositiveShaderStorageImage : public ShaderStorageImageTest {};

class ShaderStorageTexelTest : public VkLayerTest {};
class NegativeShaderStorageTexel : public ShaderStorageTexelTest {};
class PositiveShaderStorageTexel : public ShaderStorageTexelTest {};

class SparseTest : public VkLayerTest {};
class NegativeSparseImage : public SparseTest {};
class PositiveSparseImage : public SparseTest {};
class NegativeSparseBuffer : public SparseTest {};
class PositiveSparseBuffer : public SparseTest {};

class NegativeSubgroup : public VkLayerTest {};

class SubpassTest : public VkLayerTest {};
class NegativeSubpass : public SubpassTest {};
class PositiveSubpass : public SubpassTest {};

class SyncObjectTest : public VkLayerTest {
  protected:
#ifdef VK_USE_PLATFORM_WIN32_KHR
    using ExternalHandle = HANDLE;
#else
    using ExternalHandle = int;
#endif
};
class NegativeSyncObject : public SyncObjectTest {};
class PositiveSyncObject : public SyncObjectTest {};

class NegativeTransformFeedback : public VkLayerTest {
  public:
    void InitBasicTransformFeedback(void *pNextFeatures = nullptr);
};

class PositiveTooling : public VkLayerTest {};

class VertexInputTest : public VkLayerTest {};
class NegativeVertexInput : public VertexInputTest {};
class PositiveVertexInput : public VertexInputTest {};

class NegativeViewportInheritance : public VkLayerTest {};

class WsiTest : public VkLayerTest {};
class NegativeWsi : public WsiTest {};
class PositiveWsi : public WsiTest {};

class YcbcrTest : public VkLayerTest {
  public:
    void InitBasicYcbcr(void *pNextFeatures = nullptr);
};
class NegativeYcbcr : public YcbcrTest {};
class PositiveYcbcr : public YcbcrTest {};

class CooperativeMatrixTest : public VkLayerTest {};
class NegativeShaderCooperativeMatrix : public CooperativeMatrixTest {};
class PositiveShaderCooperativeMatrix : public CooperativeMatrixTest {};

class ParentTest : public VkLayerTest {
  public:
    ~ParentTest();
    vkt::Device *m_second_device = nullptr;
};
class NegativeParent : public ParentTest {};

// Thread safety tests and other tests that implement non-trivial threading scenarios
class ThreadingTest : public VkLayerTest {};
class NegativeThreading : public ThreadingTest {};
class PositiveThreading : public ThreadingTest {};

template <typename T>
bool IsValidVkStruct(const T &s) {
    return vku::GetSType<T>() == s.sType;
}

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

void ReleaseNullFence(ThreadTestData *);

void TestRenderPassCreate(ErrorMonitor *error_monitor, const vkt::Device &device, const VkRenderPassCreateInfo &create_info,
                          bool rp2_supported, const char *rp1_vuid, const char *rp2_vuid);
void PositiveTestRenderPassCreate(ErrorMonitor *error_monitor, const vkt::Device &device, const VkRenderPassCreateInfo &create_info,
                                  bool rp2_supported);
void PositiveTestRenderPass2KHRCreate(const vkt::Device &device, const VkRenderPassCreateInfo2KHR &create_info);
void TestRenderPass2KHRCreate(ErrorMonitor &error_monitor, const vkt::Device &device, const VkRenderPassCreateInfo2KHR &create_info,
                              const std::initializer_list<const char *> &vuids);
void TestRenderPassBegin(ErrorMonitor *error_monitor, const VkDevice device, const VkCommandBuffer command_buffer,
                         const VkRenderPassBeginInfo *begin_info, bool rp2Supported, const char *rp1_vuid, const char *rp2_vuid);

// Helpers for the tests below
void ValidOwnershipTransferOp(ErrorMonitor *monitor, vkt::CommandBuffer *cb, VkPipelineStageFlags src_stages,
                              VkPipelineStageFlags dst_stages, const VkBufferMemoryBarrier *buf_barrier,
                              const VkImageMemoryBarrier *img_barrier);

void ValidOwnershipTransferOp(ErrorMonitor *monitor, vkt::CommandBuffer *cb, const VkBufferMemoryBarrier2KHR *buf_barrier,
                              const VkImageMemoryBarrier2KHR *img_barrier);

void ValidOwnershipTransfer(ErrorMonitor *monitor, vkt::CommandBuffer *cb_from, vkt::CommandBuffer *cb_to,
                            VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages,
                            const VkBufferMemoryBarrier *buf_barrier, const VkImageMemoryBarrier *img_barrier);

void ValidOwnershipTransfer(ErrorMonitor *monitor, vkt::CommandBuffer *cb_from, vkt::CommandBuffer *cb_to,
                            const VkBufferMemoryBarrier2KHR *buf_barrier, const VkImageMemoryBarrier2KHR *img_barrier);

VkResult GPDIFPHelper(VkPhysicalDevice dev, const VkImageCreateInfo *ci, VkImageFormatProperties *limits = nullptr);

VkFormat FindFormatWithoutFeatures(VkPhysicalDevice gpu, VkImageTiling tiling,
                                   VkFormatFeatureFlags undesired_features = vvl::kU32Max);

VkFormat FindFormatWithoutFeatures2(VkPhysicalDevice gpu, VkImageTiling tiling, VkFormatFeatureFlags2 undesired_features);

bool SemaphoreExportImportSupported(VkPhysicalDevice gpu, VkExternalSemaphoreHandleTypeFlagBits handle_type);

void SetImageLayout(vkt::Device *device, VkImageAspectFlags aspect, VkImage image, VkImageLayout image_layout);

void AllocateDisjointMemory(vkt::Device *device, PFN_vkGetImageMemoryRequirements2KHR fp, VkImage mp_image,
                            VkDeviceMemory *mp_image_mem, VkImageAspectFlagBits plane);

void CreateSamplerTest(VkLayerTest &test, const VkSamplerCreateInfo *pCreateInfo, const std::string &code = "");

void CreateBufferTest(VkLayerTest &test, const VkBufferCreateInfo *pCreateInfo, const std::string &code = "");

void CreateImageTest(VkLayerTest &test, const VkImageCreateInfo *pCreateInfo, const std::string &code = "");

void CreateBufferViewTest(VkLayerTest &test, const VkBufferViewCreateInfo *pCreateInfo, const std::vector<std::string> &codes);

void CreateImageViewTest(VkLayerTest &test, const VkImageViewCreateInfo *pCreateInfo, const std::string &code = "");

void print_android(const char *c);
