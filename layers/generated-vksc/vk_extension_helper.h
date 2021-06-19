// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See helper_file_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (c) 2015-2021 Google Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisforbes@google.com>
 * Author: John Zulauf<jzulauf@lunarg.com>
 *
 ****************************************************************************/


#ifndef VK_EXTENSION_HELPER_H_
#define VK_EXTENSION_HELPER_H_
#include <string>
#include <utility>
#include <set>
#include <vector>
#include <cassert>

#include <vulkan/vulkan_sc.h>
#include "vk_layer_data.h"
#define VK_VERSION_1_1_NAME "VK_VERSION_1_1"

// Suppress unused warning on Linux
#if defined(__GNUC__)
#define DECORATE_UNUSED __attribute__((unused))
#else
#define DECORATE_UNUSED
#endif

enum ExtEnabled : unsigned char {
    kNotEnabled,
    kEnabledByCreateinfo,
    kEnabledByApiLevel,
};

static bool DECORATE_UNUSED IsExtEnabled(ExtEnabled extension) {
    return (extension != kNotEnabled);
};

static bool DECORATE_UNUSED IsExtEnabledByCreateinfo(ExtEnabled extension) {
    return (extension == kEnabledByCreateinfo);
};
#define VK_VERSION_1_2_NAME "VK_VERSION_1_2"

struct InstanceExtensions {
    ExtEnabled vk_feature_version_1_1{kNotEnabled};
    ExtEnabled vk_feature_version_1_2{kNotEnabled};
    ExtEnabled vk_ext_debug_utils{kNotEnabled};
    ExtEnabled vk_ext_direct_mode_display{kNotEnabled};
    ExtEnabled vk_ext_display_surface_counter{kNotEnabled};
    ExtEnabled vk_ext_headless_surface{kNotEnabled};
    ExtEnabled vk_ext_swapchain_colorspace{kNotEnabled};
    ExtEnabled vk_ext_validation_features{kNotEnabled};
    ExtEnabled vk_khr_display{kNotEnabled};
    ExtEnabled vk_khr_get_display_properties2{kNotEnabled};
    ExtEnabled vk_khr_get_surface_capabilities2{kNotEnabled};
    ExtEnabled vk_khr_surface{kNotEnabled};

    struct InstanceReq {
        const ExtEnabled InstanceExtensions::* enabled;
        const char *name;
    };
    typedef std::vector<InstanceReq> InstanceReqVec;
    struct InstanceInfo {
       InstanceInfo(ExtEnabled InstanceExtensions::* state_, const InstanceReqVec requirements_): state(state_), requirements(requirements_) {}
       ExtEnabled InstanceExtensions::* state;
       InstanceReqVec requirements;
    };

    typedef layer_data::unordered_map<std::string,InstanceInfo> InstanceInfoMap;
    static const InstanceInfoMap &get_info_map() {
        static const InstanceInfoMap info_map = {
            {"VK_VERSION_1_1", InstanceInfo(&InstanceExtensions::vk_feature_version_1_1, {})},
            {"VK_VERSION_1_2", InstanceInfo(&InstanceExtensions::vk_feature_version_1_2, {})},
            {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_debug_utils, {})},
            {VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_direct_mode_display, {{
                           {&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_display_surface_counter, {{
                           {&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_headless_surface, {{
                           {&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_swapchain_colorspace, {{
                           {&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_validation_features, {})},
            {VK_KHR_DISPLAY_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_khr_display, {{
                           {&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_khr_get_display_properties2, {{
                           {&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_khr_get_surface_capabilities2, {{
                           {&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_KHR_SURFACE_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_khr_surface, {})},
        };

        return info_map;
    }

    static const InstanceInfo &get_info(const char *name) {
        static const InstanceInfo empty_info {nullptr, InstanceReqVec()};
        const auto &ext_map = InstanceExtensions::get_info_map();
        const auto info = ext_map.find(name);
        if ( info != ext_map.cend()) {
            return info->second;
        }
        return empty_info;
    }

    uint32_t NormalizeApiVersion(uint32_t specified_version) {
        if (specified_version < VK_API_VERSION_1_1)
            return VK_API_VERSION_1_0;
        else if (specified_version < VK_API_VERSION_1_2)
            return VK_API_VERSION_1_1;
        else
            return VK_API_VERSION_1_2;
    }

    uint32_t InitFromInstanceCreateInfo(uint32_t requested_api_version, const VkInstanceCreateInfo *pCreateInfo) {

        static const std::vector<const char *> V_1_1_promoted_instance_apis = {
        };
        static const std::vector<const char *> V_1_2_promoted_instance_apis = {
        };

        // Initialize struct data, robust to invalid pCreateInfo
        uint32_t api_version = NormalizeApiVersion(requested_api_version);
        if (api_version >= VK_API_VERSION_1_1) {
            auto info = get_info("VK_VERSION_1_1");
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            for (auto promoted_ext : V_1_1_promoted_instance_apis) {
                info = get_info(promoted_ext);
                assert(info.state);
                if (info.state) this->*(info.state) = kEnabledByApiLevel;
            }
        }
        if (api_version >= VK_API_VERSION_1_2) {
            auto info = get_info("VK_VERSION_1_2");
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            for (auto promoted_ext : V_1_2_promoted_instance_apis) {
                info = get_info(promoted_ext);
                assert(info.state);
                if (info.state) this->*(info.state) = kEnabledByApiLevel;
            }
        }
        // CreateInfo takes precedence over promoted
        if (pCreateInfo->ppEnabledExtensionNames) {
            for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
                auto info = get_info(pCreateInfo->ppEnabledExtensionNames[i]);
                if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            }
        }
        return api_version;
    }
};

static const std::set<std::string> kInstanceExtensionNames = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
    VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME,
    VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME,
    VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
    VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
    VK_KHR_DISPLAY_EXTENSION_NAME,
    VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
    VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
    VK_KHR_SURFACE_EXTENSION_NAME,
};

struct DeviceExtensions : public InstanceExtensions {
    ExtEnabled vk_feature_version_1_1{kNotEnabled};
    ExtEnabled vk_feature_version_1_2{kNotEnabled};
    ExtEnabled vk_ext_4444_formats{kNotEnabled};
    ExtEnabled vk_ext_astc_decode_mode{kNotEnabled};
    ExtEnabled vk_ext_blend_operation_advanced{kNotEnabled};
    ExtEnabled vk_ext_calibrated_timestamps{kNotEnabled};
    ExtEnabled vk_ext_color_write_enable{kNotEnabled};
    ExtEnabled vk_ext_conservative_rasterization{kNotEnabled};
    ExtEnabled vk_ext_custom_border_color{kNotEnabled};
    ExtEnabled vk_ext_depth_clip_enable{kNotEnabled};
    ExtEnabled vk_ext_depth_range_unrestricted{kNotEnabled};
    ExtEnabled vk_ext_discard_rectangles{kNotEnabled};
    ExtEnabled vk_ext_display_control{kNotEnabled};
    ExtEnabled vk_ext_extended_dynamic_state{kNotEnabled};
    ExtEnabled vk_ext_extended_dynamic_state2{kNotEnabled};
    ExtEnabled vk_ext_external_memory_dma_buf{kNotEnabled};
    ExtEnabled vk_ext_external_memory_host{kNotEnabled};
    ExtEnabled vk_ext_filter_cubic{kNotEnabled};
    ExtEnabled vk_ext_fragment_shader_interlock{kNotEnabled};
    ExtEnabled vk_ext_global_priority{kNotEnabled};
    ExtEnabled vk_ext_hdr_metadata{kNotEnabled};
    ExtEnabled vk_ext_image_drm_format_modifier{kNotEnabled};
    ExtEnabled vk_ext_image_robustness{kNotEnabled};
    ExtEnabled vk_ext_index_type_uint8{kNotEnabled};
    ExtEnabled vk_ext_line_rasterization{kNotEnabled};
    ExtEnabled vk_ext_memory_budget{kNotEnabled};
    ExtEnabled vk_ext_pci_bus_info{kNotEnabled};
    ExtEnabled vk_ext_post_depth_coverage{kNotEnabled};
    ExtEnabled vk_ext_queue_family_foreign{kNotEnabled};
    ExtEnabled vk_ext_robustness2{kNotEnabled};
    ExtEnabled vk_ext_sample_locations{kNotEnabled};
    ExtEnabled vk_ext_shader_atomic_float{kNotEnabled};
    ExtEnabled vk_ext_shader_demote_to_helper_invocation{kNotEnabled};
    ExtEnabled vk_ext_shader_image_atomic_int64{kNotEnabled};
    ExtEnabled vk_ext_shader_stencil_export{kNotEnabled};
    ExtEnabled vk_ext_subgroup_size_control{kNotEnabled};
    ExtEnabled vk_ext_texel_buffer_alignment{kNotEnabled};
    ExtEnabled vk_ext_texture_compression_astc_hdr{kNotEnabled};
    ExtEnabled vk_ext_vertex_attribute_divisor{kNotEnabled};
    ExtEnabled vk_ext_vertex_input_dynamic_state{kNotEnabled};
    ExtEnabled vk_ext_ycbcr_2plane_444_formats{kNotEnabled};
    ExtEnabled vk_ext_ycbcr_image_arrays{kNotEnabled};
    ExtEnabled vk_khr_copy_commands2{kNotEnabled};
    ExtEnabled vk_khr_display_swapchain{kNotEnabled};
    ExtEnabled vk_khr_external_fence_fd{kNotEnabled};
    ExtEnabled vk_khr_external_memory_fd{kNotEnabled};
    ExtEnabled vk_khr_external_semaphore_fd{kNotEnabled};
    ExtEnabled vk_khr_fragment_shading_rate{kNotEnabled};
    ExtEnabled vk_khr_incremental_present{kNotEnabled};
    ExtEnabled vk_khr_object_refresh{kNotEnabled};
    ExtEnabled vk_khr_performance_query{kNotEnabled};
    ExtEnabled vk_khr_shader_clock{kNotEnabled};
    ExtEnabled vk_khr_shader_terminate_invocation{kNotEnabled};
    ExtEnabled vk_khr_shared_presentable_image{kNotEnabled};
    ExtEnabled vk_khr_swapchain{kNotEnabled};
    ExtEnabled vk_khr_swapchain_mutable_format{kNotEnabled};
    ExtEnabled vk_khr_synchronization2{kNotEnabled};

    struct DeviceReq {
        const ExtEnabled DeviceExtensions::* enabled;
        const char *name;
    };
    typedef std::vector<DeviceReq> DeviceReqVec;
    struct DeviceInfo {
       DeviceInfo(ExtEnabled DeviceExtensions::* state_, const DeviceReqVec requirements_): state(state_), requirements(requirements_) {}
       ExtEnabled DeviceExtensions::* state;
       DeviceReqVec requirements;
    };

    typedef layer_data::unordered_map<std::string,DeviceInfo> DeviceInfoMap;
    static const DeviceInfoMap &get_info_map() {
        static const DeviceInfoMap info_map = {
            {"VK_VERSION_1_1", DeviceInfo(&DeviceExtensions::vk_feature_version_1_1, {})},
            {"VK_VERSION_1_2", DeviceInfo(&DeviceExtensions::vk_feature_version_1_2, {})},
            {VK_EXT_4444_FORMATS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_4444_formats, {})},
            {VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_astc_decode_mode, {})},
            {VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_blend_operation_advanced, {})},
            {VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_calibrated_timestamps, {})},
            {VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_color_write_enable, {})},
            {VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_conservative_rasterization, {})},
            {VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_custom_border_color, {})},
            {VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_depth_clip_enable, {})},
            {VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_depth_range_unrestricted, {})},
            {VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_discard_rectangles, {})},
            {VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_display_control, {{
                           {&DeviceExtensions::vk_ext_display_surface_counter, VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME},
                           {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_extended_dynamic_state, {})},
            {VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_extended_dynamic_state2, {})},
            {VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_external_memory_dma_buf, {{
                           {&DeviceExtensions::vk_khr_external_memory_fd, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME}}})},
            {VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_external_memory_host, {})},
            {VK_EXT_FILTER_CUBIC_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_filter_cubic, {})},
            {VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_fragment_shader_interlock, {})},
            {VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_global_priority, {})},
            {VK_EXT_HDR_METADATA_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_hdr_metadata, {{
                           {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_image_drm_format_modifier, {})},
            {VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_image_robustness, {})},
            {VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_index_type_uint8, {})},
            {VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_line_rasterization, {})},
            {VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_memory_budget, {})},
            {VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_pci_bus_info, {})},
            {VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_post_depth_coverage, {})},
            {VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_queue_family_foreign, {})},
            {VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_robustness2, {})},
            {VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_sample_locations, {})},
            {VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_shader_atomic_float, {})},
            {VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_shader_demote_to_helper_invocation, {})},
            {VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_shader_image_atomic_int64, {})},
            {VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_shader_stencil_export, {})},
            {VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_subgroup_size_control, {{
                           {&DeviceExtensions::vk_feature_version_1_1, VK_VERSION_1_1_NAME}}})},
            {VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_texel_buffer_alignment, {})},
            {VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_texture_compression_astc_hdr, {})},
            {VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_vertex_attribute_divisor, {})},
            {VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_vertex_input_dynamic_state, {})},
            {VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_ycbcr_2plane_444_formats, {})},
            {VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_ycbcr_image_arrays, {})},
            {VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_copy_commands2, {})},
            {VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_display_swapchain, {{
                           {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                           {&DeviceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_external_fence_fd, {})},
            {VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_external_memory_fd, {})},
            {VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_external_semaphore_fd, {})},
            {VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_fragment_shading_rate, {})},
            {VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_incremental_present, {{
                           {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_KHR_OBJECT_REFRESH_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_object_refresh, {})},
            {VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_performance_query, {{
                           {&DeviceExtensions::vk_feature_version_1_1, VK_VERSION_1_1_NAME}}})},
            {VK_KHR_SHADER_CLOCK_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_shader_clock, {})},
            {VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_shader_terminate_invocation, {})},
            {VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_shared_presentable_image, {{
                           {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                           {&DeviceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME}}})},
            {VK_KHR_SWAPCHAIN_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_swapchain, {{
                           {&DeviceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_swapchain_mutable_format, {{
                           {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_synchronization2, {})},
        };

        return info_map;
    }

    static const DeviceInfo &get_info(const char *name) {
        static const DeviceInfo empty_info {nullptr, DeviceReqVec()};
        const auto &ext_map = DeviceExtensions::get_info_map();
        const auto info = ext_map.find(name);
        if ( info != ext_map.cend()) {
            return info->second;
        }
        return empty_info;
    }

    DeviceExtensions() = default;
    DeviceExtensions(const InstanceExtensions& instance_ext) : InstanceExtensions(instance_ext) {}

    uint32_t InitFromDeviceCreateInfo(const InstanceExtensions *instance_extensions, uint32_t requested_api_version,
                                      const VkDeviceCreateInfo *pCreateInfo) {
        // Initialize: this to defaults,  base class fields to input.
        assert(instance_extensions);
        *this = DeviceExtensions(*instance_extensions);


        static const std::vector<const char *> V_1_1_promoted_device_apis = {
        };
        static const std::vector<const char *> V_1_2_promoted_device_apis = {
        };

        // Initialize struct data, robust to invalid pCreateInfo
        uint32_t api_version = NormalizeApiVersion(requested_api_version);
        if (api_version >= VK_API_VERSION_1_1) {
            auto info = get_info("VK_VERSION_1_1");
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            for (auto promoted_ext : V_1_1_promoted_device_apis) {
                info = get_info(promoted_ext);
                assert(info.state);
                if (info.state) this->*(info.state) = kEnabledByApiLevel;
            }
        }
        if (api_version >= VK_API_VERSION_1_2) {
            auto info = get_info("VK_VERSION_1_2");
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            for (auto promoted_ext : V_1_2_promoted_device_apis) {
                info = get_info(promoted_ext);
                assert(info.state);
                if (info.state) this->*(info.state) = kEnabledByApiLevel;
            }
        }
        // CreateInfo takes precedence over promoted
        if (pCreateInfo->ppEnabledExtensionNames) {
            for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
                auto info = get_info(pCreateInfo->ppEnabledExtensionNames[i]);
                if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            }
        }
        return api_version;
    }
};

static const std::set<std::string> kDeviceExtensionNames = {
    VK_EXT_4444_FORMATS_EXTENSION_NAME,
    VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME,
    VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME,
    VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME,
    VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME,
    VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,
    VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME,
    VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME,
    VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME,
    VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME,
    VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME,
    VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
    VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
    VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
    VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME,
    VK_EXT_FILTER_CUBIC_EXTENSION_NAME,
    VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME,
    VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME,
    VK_EXT_HDR_METADATA_EXTENSION_NAME,
    VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
    VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME,
    VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME,
    VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME,
    VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
    VK_EXT_PCI_BUS_INFO_EXTENSION_NAME,
    VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME,
    VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
    VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
    VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME,
    VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME,
    VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
    VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME,
    VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME,
    VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME,
    VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME,
    VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME,
    VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME,
    VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
    VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME,
    VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME,
    VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
    VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME,
    VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
    VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
    VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME,
    VK_KHR_OBJECT_REFRESH_EXTENSION_NAME,
    VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME,
    VK_KHR_SHADER_CLOCK_EXTENSION_NAME,
    VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME,
    VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
};


#endif // VK_EXTENSION_HELPER_H_
