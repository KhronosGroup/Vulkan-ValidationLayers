// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See extension_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
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
 ****************************************************************************/
// NOLINTBEGIN
#pragma once

#include <string>
#include <utility>
#include <set>
#include <array>
#include <vector>
#include <cassert>

#include <vulkan/vulkan.h>
#include "containers/custom_containers.h"
#include "generated/vk_api_version.h"

enum ExtEnabled : unsigned char {
    kNotEnabled,
    kEnabledByCreateinfo,
    kEnabledByApiLevel,
    kEnabledByInteraction,
};

/*
This function is a helper to know if the extension is enabled.

Times to use it
- To determine the VUID
- The VU mentions the use of the extension
- Extension exposes property limits being validated
- Checking not enabled
    - if (!IsExtEnabled(...)) { }
- Special extensions that being EXPOSED alters the VUs
    - IsExtEnabled(device_extensions.vk_khr_portability_subset)
- Special extensions that alter behaviour of enabled
    - IsExtEnabled(device_extensions.vk_khr_maintenance*)

Times to NOT use it
    - If checking if a struct or enum is being used. There are a stateless checks
      to make sure the new Structs/Enums are not being used without this enabled.
    - If checking if the extension's feature enable status, because if the feature
      is enabled, then we already validated that extension is enabled.
    - Some variables (ex. viewMask) require the extension to be used if non-zero
*/
[[maybe_unused]] static bool IsExtEnabled(ExtEnabled extension) { return (extension != kNotEnabled); }

[[maybe_unused]] static bool IsExtEnabledByCreateinfo(ExtEnabled extension) { return (extension == kEnabledByCreateinfo); }

struct InstanceExtensions {
    ExtEnabled vk_feature_version_1_1{kNotEnabled};
    ExtEnabled vk_feature_version_1_2{kNotEnabled};
    ExtEnabled vk_feature_version_1_3{kNotEnabled};
    ExtEnabled vk_khr_surface{kNotEnabled};
    ExtEnabled vk_khr_display{kNotEnabled};
    ExtEnabled vk_khr_xlib_surface{kNotEnabled};
    ExtEnabled vk_khr_xcb_surface{kNotEnabled};
    ExtEnabled vk_khr_wayland_surface{kNotEnabled};
    ExtEnabled vk_khr_android_surface{kNotEnabled};
    ExtEnabled vk_khr_win32_surface{kNotEnabled};
    ExtEnabled vk_khr_get_physical_device_properties2{kNotEnabled};
    ExtEnabled vk_khr_device_group_creation{kNotEnabled};
    ExtEnabled vk_khr_external_memory_capabilities{kNotEnabled};
    ExtEnabled vk_khr_external_semaphore_capabilities{kNotEnabled};
    ExtEnabled vk_khr_external_fence_capabilities{kNotEnabled};
    ExtEnabled vk_khr_get_surface_capabilities2{kNotEnabled};
    ExtEnabled vk_khr_get_display_properties2{kNotEnabled};
    ExtEnabled vk_khr_surface_protected_capabilities{kNotEnabled};
    ExtEnabled vk_khr_portability_enumeration{kNotEnabled};
    ExtEnabled vk_ext_debug_report{kNotEnabled};
    ExtEnabled vk_ggp_stream_descriptor_surface{kNotEnabled};
    ExtEnabled vk_nv_external_memory_capabilities{kNotEnabled};
    ExtEnabled vk_ext_validation_flags{kNotEnabled};
    ExtEnabled vk_nn_vi_surface{kNotEnabled};
    ExtEnabled vk_ext_direct_mode_display{kNotEnabled};
    ExtEnabled vk_ext_acquire_xlib_display{kNotEnabled};
    ExtEnabled vk_ext_display_surface_counter{kNotEnabled};
    ExtEnabled vk_ext_swapchain_colorspace{kNotEnabled};
    ExtEnabled vk_mvk_ios_surface{kNotEnabled};
    ExtEnabled vk_mvk_macos_surface{kNotEnabled};
    ExtEnabled vk_ext_debug_utils{kNotEnabled};
    ExtEnabled vk_fuchsia_imagepipe_surface{kNotEnabled};
    ExtEnabled vk_ext_metal_surface{kNotEnabled};
    ExtEnabled vk_ext_validation_features{kNotEnabled};
    ExtEnabled vk_ext_headless_surface{kNotEnabled};
    ExtEnabled vk_ext_surface_maintenance1{kNotEnabled};
    ExtEnabled vk_ext_acquire_drm_display{kNotEnabled};
    ExtEnabled vk_ext_directfb_surface{kNotEnabled};
    ExtEnabled vk_qnx_screen_surface{kNotEnabled};
    ExtEnabled vk_google_surfaceless_query{kNotEnabled};
    ExtEnabled vk_lunarg_direct_driver_loading{kNotEnabled};

    struct InstanceReq {
        const ExtEnabled InstanceExtensions::*enabled;
        const char *name;
    };
    typedef std::vector<InstanceReq> InstanceReqVec;
    struct InstanceInfo {
        InstanceInfo(ExtEnabled InstanceExtensions::*state_, const InstanceReqVec requirements_)
            : state(state_), requirements(requirements_) {}
        ExtEnabled InstanceExtensions::*state;
        InstanceReqVec requirements;
    };

    typedef vvl::unordered_map<std::string, InstanceInfo> InstanceInfoMap;
    static const InstanceInfoMap &get_info_map() {
        static const InstanceInfoMap info_map = {
            {"VK_VERSION_1_1", InstanceInfo(&InstanceExtensions::vk_feature_version_1_1, {})},
            {"VK_VERSION_1_2", InstanceInfo(&InstanceExtensions::vk_feature_version_1_2, {})},
            {"VK_VERSION_1_3", InstanceInfo(&InstanceExtensions::vk_feature_version_1_3, {})},
            {VK_KHR_SURFACE_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_khr_surface, {})},
            {VK_KHR_DISPLAY_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_khr_display,
                                                         {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_XLIB_KHR
            {VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_xlib_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
            {VK_KHR_XCB_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_xcb_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
            {VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_wayland_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            {VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_android_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_win32_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
            {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_get_physical_device_properties2, {})},
            {VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_khr_device_group_creation, {})},
            {VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_external_memory_capabilities,
                          {{{&InstanceExtensions::vk_khr_get_physical_device_properties2,
                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_external_semaphore_capabilities,
                          {{{&InstanceExtensions::vk_khr_get_physical_device_properties2,
                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_external_fence_capabilities,
                          {{{&InstanceExtensions::vk_khr_get_physical_device_properties2,
                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_get_surface_capabilities2,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_khr_get_display_properties2,
                          {{{&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME,
             InstanceInfo(
                 &InstanceExtensions::vk_khr_surface_protected_capabilities,
                 {{{&InstanceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                   {&InstanceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME}}})},
            {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_khr_portability_enumeration, {})},
            {VK_EXT_DEBUG_REPORT_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_debug_report, {})},
#ifdef VK_USE_PLATFORM_GGP
            {VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_ggp_stream_descriptor_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
            {VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_nv_external_memory_capabilities, {})},
            {VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_validation_flags, {})},
#ifdef VK_USE_PLATFORM_VI_NN
            {VK_NN_VI_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_nn_vi_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
            {VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_ext_direct_mode_display,
                          {{{&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
            {VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_ext_acquire_xlib_display,
                          {{{&InstanceExtensions::vk_ext_direct_mode_display, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME}}})},
#endif
            {VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_ext_display_surface_counter,
                          {{{&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_ext_swapchain_colorspace,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_IOS_MVK
            {VK_MVK_IOS_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_mvk_ios_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
            {VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_mvk_macos_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
            {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_debug_utils, {})},
#ifdef VK_USE_PLATFORM_FUCHSIA
            {VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_fuchsia_imagepipe_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
            {VK_EXT_METAL_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_ext_metal_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
            {VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, InstanceInfo(&InstanceExtensions::vk_ext_validation_features, {})},
            {VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_ext_headless_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
             InstanceInfo(
                 &InstanceExtensions::vk_ext_surface_maintenance1,
                 {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME},
                   {&InstanceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME}}})},
            {VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_ext_acquire_drm_display,
                          {{{&InstanceExtensions::vk_ext_direct_mode_display, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
            {VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_ext_directfb_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            {VK_QNX_SCREEN_SURFACE_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_qnx_screen_surface,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif
            {VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_google_surfaceless_query,
                          {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME,
             InstanceInfo(&InstanceExtensions::vk_lunarg_direct_driver_loading, {})},

        };
        return info_map;
    }

    static const InstanceInfo &get_info(const char *name) {
        static const InstanceInfo empty_info{nullptr, InstanceReqVec()};
        const auto &ext_map = InstanceExtensions::get_info_map();
        const auto info = ext_map.find(name);
        if (info != ext_map.cend()) {
            return info->second;
        }
        return empty_info;
    }

    APIVersion InitFromInstanceCreateInfo(APIVersion requested_api_version, const VkInstanceCreateInfo *pCreateInfo) {
        constexpr std::array<const char *, 5> V_1_1_promoted_instance_apis = {
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,     VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME,
        };
        constexpr std::array<const char *, 0> V_1_2_promoted_instance_apis = {};
        constexpr std::array<const char *, 0> V_1_3_promoted_instance_apis = {};

        // Initialize struct data, robust to invalid pCreateInfo
        auto api_version = NormalizeApiVersion(requested_api_version);
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
        if (api_version >= VK_API_VERSION_1_3) {
            auto info = get_info("VK_VERSION_1_3");
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            for (auto promoted_ext : V_1_3_promoted_instance_apis) {
                info = get_info(promoted_ext);
                assert(info.state);
                if (info.state) this->*(info.state) = kEnabledByApiLevel;
            }
        }
        // CreateInfo takes precedence over promoted
        if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
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
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_DISPLAY_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_XLIB_KHR
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
    VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
    VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
    VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_GGP
    VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME,
#endif
    VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
    VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_VI_NN
    VK_NN_VI_SURFACE_EXTENSION_NAME,
#endif
    VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME,
#endif
    VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME,
    VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_IOS_MVK
    VK_MVK_IOS_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#endif
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_FUCHSIA
    VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
    VK_EXT_METAL_SURFACE_EXTENSION_NAME,
#endif
    VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
    VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME,
    VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
    VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    VK_QNX_SCREEN_SURFACE_EXTENSION_NAME,
#endif
    VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME,
    VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME,
};

struct DeviceExtensions : public InstanceExtensions {
    ExtEnabled vk_feature_version_1_1{kNotEnabled};
    ExtEnabled vk_feature_version_1_2{kNotEnabled};
    ExtEnabled vk_feature_version_1_3{kNotEnabled};
    ExtEnabled vk_khr_swapchain{kNotEnabled};
    ExtEnabled vk_khr_display_swapchain{kNotEnabled};
    ExtEnabled vk_khr_sampler_mirror_clamp_to_edge{kNotEnabled};
    ExtEnabled vk_khr_video_queue{kNotEnabled};
    ExtEnabled vk_khr_video_decode_queue{kNotEnabled};
    ExtEnabled vk_khr_video_decode_h264{kNotEnabled};
    ExtEnabled vk_khr_dynamic_rendering{kNotEnabled};
    ExtEnabled vk_khr_multiview{kNotEnabled};
    ExtEnabled vk_khr_device_group{kNotEnabled};
    ExtEnabled vk_khr_shader_draw_parameters{kNotEnabled};
    ExtEnabled vk_khr_maintenance1{kNotEnabled};
    ExtEnabled vk_khr_external_memory{kNotEnabled};
    ExtEnabled vk_khr_external_memory_win32{kNotEnabled};
    ExtEnabled vk_khr_external_memory_fd{kNotEnabled};
    ExtEnabled vk_khr_win32_keyed_mutex{kNotEnabled};
    ExtEnabled vk_khr_external_semaphore{kNotEnabled};
    ExtEnabled vk_khr_external_semaphore_win32{kNotEnabled};
    ExtEnabled vk_khr_external_semaphore_fd{kNotEnabled};
    ExtEnabled vk_khr_push_descriptor{kNotEnabled};
    ExtEnabled vk_khr_shader_float16_int8{kNotEnabled};
    ExtEnabled vk_khr_16bit_storage{kNotEnabled};
    ExtEnabled vk_khr_incremental_present{kNotEnabled};
    ExtEnabled vk_khr_descriptor_update_template{kNotEnabled};
    ExtEnabled vk_khr_imageless_framebuffer{kNotEnabled};
    ExtEnabled vk_khr_create_renderpass2{kNotEnabled};
    ExtEnabled vk_khr_shared_presentable_image{kNotEnabled};
    ExtEnabled vk_khr_external_fence{kNotEnabled};
    ExtEnabled vk_khr_external_fence_win32{kNotEnabled};
    ExtEnabled vk_khr_external_fence_fd{kNotEnabled};
    ExtEnabled vk_khr_performance_query{kNotEnabled};
    ExtEnabled vk_khr_maintenance2{kNotEnabled};
    ExtEnabled vk_khr_variable_pointers{kNotEnabled};
    ExtEnabled vk_khr_dedicated_allocation{kNotEnabled};
    ExtEnabled vk_khr_storage_buffer_storage_class{kNotEnabled};
    ExtEnabled vk_khr_relaxed_block_layout{kNotEnabled};
    ExtEnabled vk_khr_get_memory_requirements2{kNotEnabled};
    ExtEnabled vk_khr_image_format_list{kNotEnabled};
    ExtEnabled vk_khr_sampler_ycbcr_conversion{kNotEnabled};
    ExtEnabled vk_khr_bind_memory2{kNotEnabled};
    ExtEnabled vk_khr_portability_subset{kNotEnabled};
    ExtEnabled vk_khr_maintenance3{kNotEnabled};
    ExtEnabled vk_khr_draw_indirect_count{kNotEnabled};
    ExtEnabled vk_khr_shader_subgroup_extended_types{kNotEnabled};
    ExtEnabled vk_khr_8bit_storage{kNotEnabled};
    ExtEnabled vk_khr_shader_atomic_int64{kNotEnabled};
    ExtEnabled vk_khr_shader_clock{kNotEnabled};
    ExtEnabled vk_khr_video_decode_h265{kNotEnabled};
    ExtEnabled vk_khr_global_priority{kNotEnabled};
    ExtEnabled vk_khr_driver_properties{kNotEnabled};
    ExtEnabled vk_khr_shader_float_controls{kNotEnabled};
    ExtEnabled vk_khr_depth_stencil_resolve{kNotEnabled};
    ExtEnabled vk_khr_swapchain_mutable_format{kNotEnabled};
    ExtEnabled vk_khr_timeline_semaphore{kNotEnabled};
    ExtEnabled vk_khr_vulkan_memory_model{kNotEnabled};
    ExtEnabled vk_khr_shader_terminate_invocation{kNotEnabled};
    ExtEnabled vk_khr_fragment_shading_rate{kNotEnabled};
    ExtEnabled vk_khr_spirv_1_4{kNotEnabled};
    ExtEnabled vk_khr_separate_depth_stencil_layouts{kNotEnabled};
    ExtEnabled vk_khr_present_wait{kNotEnabled};
    ExtEnabled vk_khr_uniform_buffer_standard_layout{kNotEnabled};
    ExtEnabled vk_khr_buffer_device_address{kNotEnabled};
    ExtEnabled vk_khr_deferred_host_operations{kNotEnabled};
    ExtEnabled vk_khr_pipeline_executable_properties{kNotEnabled};
    ExtEnabled vk_khr_map_memory2{kNotEnabled};
    ExtEnabled vk_khr_shader_integer_dot_product{kNotEnabled};
    ExtEnabled vk_khr_pipeline_library{kNotEnabled};
    ExtEnabled vk_khr_shader_non_semantic_info{kNotEnabled};
    ExtEnabled vk_khr_present_id{kNotEnabled};
    ExtEnabled vk_khr_video_encode_queue{kNotEnabled};
    ExtEnabled vk_khr_synchronization2{kNotEnabled};
    ExtEnabled vk_khr_fragment_shader_barycentric{kNotEnabled};
    ExtEnabled vk_khr_shader_subgroup_uniform_control_flow{kNotEnabled};
    ExtEnabled vk_khr_zero_initialize_workgroup_memory{kNotEnabled};
    ExtEnabled vk_khr_workgroup_memory_explicit_layout{kNotEnabled};
    ExtEnabled vk_khr_copy_commands2{kNotEnabled};
    ExtEnabled vk_khr_format_feature_flags2{kNotEnabled};
    ExtEnabled vk_khr_ray_tracing_maintenance1{kNotEnabled};
    ExtEnabled vk_khr_maintenance4{kNotEnabled};
    ExtEnabled vk_khr_maintenance5{kNotEnabled};
    ExtEnabled vk_khr_ray_tracing_position_fetch{kNotEnabled};
    ExtEnabled vk_khr_cooperative_matrix{kNotEnabled};
    ExtEnabled vk_nv_glsl_shader{kNotEnabled};
    ExtEnabled vk_ext_depth_range_unrestricted{kNotEnabled};
    ExtEnabled vk_img_filter_cubic{kNotEnabled};
    ExtEnabled vk_amd_rasterization_order{kNotEnabled};
    ExtEnabled vk_amd_shader_trinary_minmax{kNotEnabled};
    ExtEnabled vk_amd_shader_explicit_vertex_parameter{kNotEnabled};
    ExtEnabled vk_ext_debug_marker{kNotEnabled};
    ExtEnabled vk_amd_gcn_shader{kNotEnabled};
    ExtEnabled vk_nv_dedicated_allocation{kNotEnabled};
    ExtEnabled vk_ext_transform_feedback{kNotEnabled};
    ExtEnabled vk_nvx_binary_import{kNotEnabled};
    ExtEnabled vk_nvx_image_view_handle{kNotEnabled};
    ExtEnabled vk_amd_draw_indirect_count{kNotEnabled};
    ExtEnabled vk_amd_negative_viewport_height{kNotEnabled};
    ExtEnabled vk_amd_gpu_shader_half_float{kNotEnabled};
    ExtEnabled vk_amd_shader_ballot{kNotEnabled};
    ExtEnabled vk_ext_video_encode_h264{kNotEnabled};
    ExtEnabled vk_ext_video_encode_h265{kNotEnabled};
    ExtEnabled vk_amd_texture_gather_bias_lod{kNotEnabled};
    ExtEnabled vk_amd_shader_info{kNotEnabled};
    ExtEnabled vk_amd_shader_image_load_store_lod{kNotEnabled};
    ExtEnabled vk_nv_corner_sampled_image{kNotEnabled};
    ExtEnabled vk_img_format_pvrtc{kNotEnabled};
    ExtEnabled vk_nv_external_memory{kNotEnabled};
    ExtEnabled vk_nv_external_memory_win32{kNotEnabled};
    ExtEnabled vk_nv_win32_keyed_mutex{kNotEnabled};
    ExtEnabled vk_ext_shader_subgroup_ballot{kNotEnabled};
    ExtEnabled vk_ext_shader_subgroup_vote{kNotEnabled};
    ExtEnabled vk_ext_texture_compression_astc_hdr{kNotEnabled};
    ExtEnabled vk_ext_astc_decode_mode{kNotEnabled};
    ExtEnabled vk_ext_pipeline_robustness{kNotEnabled};
    ExtEnabled vk_ext_conditional_rendering{kNotEnabled};
    ExtEnabled vk_nv_clip_space_w_scaling{kNotEnabled};
    ExtEnabled vk_ext_display_control{kNotEnabled};
    ExtEnabled vk_google_display_timing{kNotEnabled};
    ExtEnabled vk_nv_sample_mask_override_coverage{kNotEnabled};
    ExtEnabled vk_nv_geometry_shader_passthrough{kNotEnabled};
    ExtEnabled vk_nv_viewport_array2{kNotEnabled};
    ExtEnabled vk_nvx_multiview_per_view_attributes{kNotEnabled};
    ExtEnabled vk_nv_viewport_swizzle{kNotEnabled};
    ExtEnabled vk_ext_discard_rectangles{kNotEnabled};
    ExtEnabled vk_ext_conservative_rasterization{kNotEnabled};
    ExtEnabled vk_ext_depth_clip_enable{kNotEnabled};
    ExtEnabled vk_ext_hdr_metadata{kNotEnabled};
    ExtEnabled vk_ext_external_memory_dma_buf{kNotEnabled};
    ExtEnabled vk_ext_queue_family_foreign{kNotEnabled};
    ExtEnabled vk_android_external_memory_android_hardware_buffer{kNotEnabled};
    ExtEnabled vk_ext_sampler_filter_minmax{kNotEnabled};
    ExtEnabled vk_amd_gpu_shader_int16{kNotEnabled};
    ExtEnabled vk_amdx_shader_enqueue{kNotEnabled};
    ExtEnabled vk_amd_mixed_attachment_samples{kNotEnabled};
    ExtEnabled vk_amd_shader_fragment_mask{kNotEnabled};
    ExtEnabled vk_ext_inline_uniform_block{kNotEnabled};
    ExtEnabled vk_ext_shader_stencil_export{kNotEnabled};
    ExtEnabled vk_ext_sample_locations{kNotEnabled};
    ExtEnabled vk_ext_blend_operation_advanced{kNotEnabled};
    ExtEnabled vk_nv_fragment_coverage_to_color{kNotEnabled};
    ExtEnabled vk_nv_framebuffer_mixed_samples{kNotEnabled};
    ExtEnabled vk_nv_fill_rectangle{kNotEnabled};
    ExtEnabled vk_nv_shader_sm_builtins{kNotEnabled};
    ExtEnabled vk_ext_post_depth_coverage{kNotEnabled};
    ExtEnabled vk_ext_image_drm_format_modifier{kNotEnabled};
    ExtEnabled vk_ext_validation_cache{kNotEnabled};
    ExtEnabled vk_ext_descriptor_indexing{kNotEnabled};
    ExtEnabled vk_ext_shader_viewport_index_layer{kNotEnabled};
    ExtEnabled vk_nv_shading_rate_image{kNotEnabled};
    ExtEnabled vk_nv_ray_tracing{kNotEnabled};
    ExtEnabled vk_nv_representative_fragment_test{kNotEnabled};
    ExtEnabled vk_ext_filter_cubic{kNotEnabled};
    ExtEnabled vk_qcom_render_pass_shader_resolve{kNotEnabled};
    ExtEnabled vk_ext_global_priority{kNotEnabled};
    ExtEnabled vk_ext_external_memory_host{kNotEnabled};
    ExtEnabled vk_amd_buffer_marker{kNotEnabled};
    ExtEnabled vk_amd_pipeline_compiler_control{kNotEnabled};
    ExtEnabled vk_ext_calibrated_timestamps{kNotEnabled};
    ExtEnabled vk_amd_shader_core_properties{kNotEnabled};
    ExtEnabled vk_amd_memory_overallocation_behavior{kNotEnabled};
    ExtEnabled vk_ext_vertex_attribute_divisor{kNotEnabled};
    ExtEnabled vk_ggp_frame_token{kNotEnabled};
    ExtEnabled vk_ext_pipeline_creation_feedback{kNotEnabled};
    ExtEnabled vk_nv_shader_subgroup_partitioned{kNotEnabled};
    ExtEnabled vk_nv_compute_shader_derivatives{kNotEnabled};
    ExtEnabled vk_nv_mesh_shader{kNotEnabled};
    ExtEnabled vk_nv_fragment_shader_barycentric{kNotEnabled};
    ExtEnabled vk_nv_shader_image_footprint{kNotEnabled};
    ExtEnabled vk_nv_scissor_exclusive{kNotEnabled};
    ExtEnabled vk_nv_device_diagnostic_checkpoints{kNotEnabled};
    ExtEnabled vk_intel_shader_integer_functions2{kNotEnabled};
    ExtEnabled vk_intel_performance_query{kNotEnabled};
    ExtEnabled vk_ext_pci_bus_info{kNotEnabled};
    ExtEnabled vk_amd_display_native_hdr{kNotEnabled};
    ExtEnabled vk_ext_fragment_density_map{kNotEnabled};
    ExtEnabled vk_ext_scalar_block_layout{kNotEnabled};
    ExtEnabled vk_google_hlsl_functionality1{kNotEnabled};
    ExtEnabled vk_google_decorate_string{kNotEnabled};
    ExtEnabled vk_ext_subgroup_size_control{kNotEnabled};
    ExtEnabled vk_amd_shader_core_properties2{kNotEnabled};
    ExtEnabled vk_amd_device_coherent_memory{kNotEnabled};
    ExtEnabled vk_ext_shader_image_atomic_int64{kNotEnabled};
    ExtEnabled vk_ext_memory_budget{kNotEnabled};
    ExtEnabled vk_ext_memory_priority{kNotEnabled};
    ExtEnabled vk_nv_dedicated_allocation_image_aliasing{kNotEnabled};
    ExtEnabled vk_ext_buffer_device_address{kNotEnabled};
    ExtEnabled vk_ext_tooling_info{kNotEnabled};
    ExtEnabled vk_ext_separate_stencil_usage{kNotEnabled};
    ExtEnabled vk_nv_cooperative_matrix{kNotEnabled};
    ExtEnabled vk_nv_coverage_reduction_mode{kNotEnabled};
    ExtEnabled vk_ext_fragment_shader_interlock{kNotEnabled};
    ExtEnabled vk_ext_ycbcr_image_arrays{kNotEnabled};
    ExtEnabled vk_ext_provoking_vertex{kNotEnabled};
    ExtEnabled vk_ext_full_screen_exclusive{kNotEnabled};
    ExtEnabled vk_ext_line_rasterization{kNotEnabled};
    ExtEnabled vk_ext_shader_atomic_float{kNotEnabled};
    ExtEnabled vk_ext_host_query_reset{kNotEnabled};
    ExtEnabled vk_ext_index_type_uint8{kNotEnabled};
    ExtEnabled vk_ext_extended_dynamic_state{kNotEnabled};
    ExtEnabled vk_ext_host_image_copy{kNotEnabled};
    ExtEnabled vk_ext_shader_atomic_float2{kNotEnabled};
    ExtEnabled vk_ext_swapchain_maintenance1{kNotEnabled};
    ExtEnabled vk_ext_shader_demote_to_helper_invocation{kNotEnabled};
    ExtEnabled vk_nv_device_generated_commands{kNotEnabled};
    ExtEnabled vk_nv_inherited_viewport_scissor{kNotEnabled};
    ExtEnabled vk_ext_texel_buffer_alignment{kNotEnabled};
    ExtEnabled vk_qcom_render_pass_transform{kNotEnabled};
    ExtEnabled vk_ext_depth_bias_control{kNotEnabled};
    ExtEnabled vk_ext_device_memory_report{kNotEnabled};
    ExtEnabled vk_ext_robustness2{kNotEnabled};
    ExtEnabled vk_ext_custom_border_color{kNotEnabled};
    ExtEnabled vk_google_user_type{kNotEnabled};
    ExtEnabled vk_nv_present_barrier{kNotEnabled};
    ExtEnabled vk_ext_private_data{kNotEnabled};
    ExtEnabled vk_ext_pipeline_creation_cache_control{kNotEnabled};
    ExtEnabled vk_nv_device_diagnostics_config{kNotEnabled};
    ExtEnabled vk_qcom_render_pass_store_ops{kNotEnabled};
    ExtEnabled vk_nv_low_latency{kNotEnabled};
    ExtEnabled vk_ext_metal_objects{kNotEnabled};
    ExtEnabled vk_ext_descriptor_buffer{kNotEnabled};
    ExtEnabled vk_ext_graphics_pipeline_library{kNotEnabled};
    ExtEnabled vk_amd_shader_early_and_late_fragment_tests{kNotEnabled};
    ExtEnabled vk_nv_fragment_shading_rate_enums{kNotEnabled};
    ExtEnabled vk_nv_ray_tracing_motion_blur{kNotEnabled};
    ExtEnabled vk_ext_ycbcr_2plane_444_formats{kNotEnabled};
    ExtEnabled vk_ext_fragment_density_map2{kNotEnabled};
    ExtEnabled vk_qcom_rotated_copy_commands{kNotEnabled};
    ExtEnabled vk_ext_image_robustness{kNotEnabled};
    ExtEnabled vk_ext_image_compression_control{kNotEnabled};
    ExtEnabled vk_ext_attachment_feedback_loop_layout{kNotEnabled};
    ExtEnabled vk_ext_4444_formats{kNotEnabled};
    ExtEnabled vk_ext_device_fault{kNotEnabled};
    ExtEnabled vk_arm_rasterization_order_attachment_access{kNotEnabled};
    ExtEnabled vk_ext_rgba10x6_formats{kNotEnabled};
    ExtEnabled vk_nv_acquire_winrt_display{kNotEnabled};
    ExtEnabled vk_valve_mutable_descriptor_type{kNotEnabled};
    ExtEnabled vk_ext_vertex_input_dynamic_state{kNotEnabled};
    ExtEnabled vk_ext_physical_device_drm{kNotEnabled};
    ExtEnabled vk_ext_device_address_binding_report{kNotEnabled};
    ExtEnabled vk_ext_depth_clip_control{kNotEnabled};
    ExtEnabled vk_ext_primitive_topology_list_restart{kNotEnabled};
    ExtEnabled vk_fuchsia_external_memory{kNotEnabled};
    ExtEnabled vk_fuchsia_external_semaphore{kNotEnabled};
    ExtEnabled vk_fuchsia_buffer_collection{kNotEnabled};
    ExtEnabled vk_huawei_subpass_shading{kNotEnabled};
    ExtEnabled vk_huawei_invocation_mask{kNotEnabled};
    ExtEnabled vk_nv_external_memory_rdma{kNotEnabled};
    ExtEnabled vk_ext_pipeline_properties{kNotEnabled};
    ExtEnabled vk_ext_frame_boundary{kNotEnabled};
    ExtEnabled vk_ext_multisampled_render_to_single_sampled{kNotEnabled};
    ExtEnabled vk_ext_extended_dynamic_state2{kNotEnabled};
    ExtEnabled vk_ext_color_write_enable{kNotEnabled};
    ExtEnabled vk_ext_primitives_generated_query{kNotEnabled};
    ExtEnabled vk_ext_global_priority_query{kNotEnabled};
    ExtEnabled vk_ext_image_view_min_lod{kNotEnabled};
    ExtEnabled vk_ext_multi_draw{kNotEnabled};
    ExtEnabled vk_ext_image_2d_view_of_3d{kNotEnabled};
    ExtEnabled vk_ext_shader_tile_image{kNotEnabled};
    ExtEnabled vk_ext_opacity_micromap{kNotEnabled};
    ExtEnabled vk_nv_displacement_micromap{kNotEnabled};
    ExtEnabled vk_ext_load_store_op_none{kNotEnabled};
    ExtEnabled vk_huawei_cluster_culling_shader{kNotEnabled};
    ExtEnabled vk_ext_border_color_swizzle{kNotEnabled};
    ExtEnabled vk_ext_pageable_device_local_memory{kNotEnabled};
    ExtEnabled vk_arm_shader_core_properties{kNotEnabled};
    ExtEnabled vk_ext_image_sliced_view_of_3d{kNotEnabled};
    ExtEnabled vk_valve_descriptor_set_host_mapping{kNotEnabled};
    ExtEnabled vk_ext_depth_clamp_zero_one{kNotEnabled};
    ExtEnabled vk_ext_non_seamless_cube_map{kNotEnabled};
    ExtEnabled vk_qcom_fragment_density_map_offset{kNotEnabled};
    ExtEnabled vk_nv_copy_memory_indirect{kNotEnabled};
    ExtEnabled vk_nv_memory_decompression{kNotEnabled};
    ExtEnabled vk_nv_device_generated_commands_compute{kNotEnabled};
    ExtEnabled vk_nv_linear_color_attachment{kNotEnabled};
    ExtEnabled vk_ext_image_compression_control_swapchain{kNotEnabled};
    ExtEnabled vk_qcom_image_processing{kNotEnabled};
    ExtEnabled vk_ext_nested_command_buffer{kNotEnabled};
    ExtEnabled vk_ext_external_memory_acquire_unmodified{kNotEnabled};
    ExtEnabled vk_ext_extended_dynamic_state3{kNotEnabled};
    ExtEnabled vk_ext_subpass_merge_feedback{kNotEnabled};
    ExtEnabled vk_ext_shader_module_identifier{kNotEnabled};
    ExtEnabled vk_ext_rasterization_order_attachment_access{kNotEnabled};
    ExtEnabled vk_nv_optical_flow{kNotEnabled};
    ExtEnabled vk_ext_legacy_dithering{kNotEnabled};
    ExtEnabled vk_ext_pipeline_protected_access{kNotEnabled};
    ExtEnabled vk_android_external_format_resolve{kNotEnabled};
    ExtEnabled vk_ext_shader_object{kNotEnabled};
    ExtEnabled vk_qcom_tile_properties{kNotEnabled};
    ExtEnabled vk_sec_amigo_profiling{kNotEnabled};
    ExtEnabled vk_qcom_multiview_per_view_viewports{kNotEnabled};
    ExtEnabled vk_nv_ray_tracing_invocation_reorder{kNotEnabled};
    ExtEnabled vk_nv_extended_sparse_address_space{kNotEnabled};
    ExtEnabled vk_ext_mutable_descriptor_type{kNotEnabled};
    ExtEnabled vk_arm_shader_core_builtins{kNotEnabled};
    ExtEnabled vk_ext_pipeline_library_group_handles{kNotEnabled};
    ExtEnabled vk_ext_dynamic_rendering_unused_attachments{kNotEnabled};
    ExtEnabled vk_nv_low_latency2{kNotEnabled};
    ExtEnabled vk_qcom_multiview_per_view_render_areas{kNotEnabled};
    ExtEnabled vk_qcom_image_processing2{kNotEnabled};
    ExtEnabled vk_qcom_filter_cubic_weights{kNotEnabled};
    ExtEnabled vk_qcom_ycbcr_degamma{kNotEnabled};
    ExtEnabled vk_qcom_filter_cubic_clamp{kNotEnabled};
    ExtEnabled vk_ext_attachment_feedback_loop_dynamic_state{kNotEnabled};
    ExtEnabled vk_qnx_external_memory_screen_buffer{kNotEnabled};
    ExtEnabled vk_msft_layered_driver{kNotEnabled};
    ExtEnabled vk_nv_descriptor_pool_overallocation{kNotEnabled};
    ExtEnabled vk_khr_acceleration_structure{kNotEnabled};
    ExtEnabled vk_khr_ray_tracing_pipeline{kNotEnabled};
    ExtEnabled vk_khr_ray_query{kNotEnabled};
    ExtEnabled vk_ext_mesh_shader{kNotEnabled};

    struct DeviceReq {
        const ExtEnabled DeviceExtensions::*enabled;
        const char *name;
    };
    typedef std::vector<DeviceReq> DeviceReqVec;
    struct DeviceInfo {
        DeviceInfo(ExtEnabled DeviceExtensions::*state_, const DeviceReqVec requirements_)
            : state(state_), requirements(requirements_) {}
        ExtEnabled DeviceExtensions::*state;
        DeviceReqVec requirements;
    };

    typedef vvl::unordered_map<std::string, DeviceInfo> DeviceInfoMap;
    static const DeviceInfoMap &get_info_map() {
        static const DeviceInfoMap info_map = {
            {"VK_VERSION_1_1", DeviceInfo(&DeviceExtensions::vk_feature_version_1_1, {})},
            {"VK_VERSION_1_2", DeviceInfo(&DeviceExtensions::vk_feature_version_1_2, {})},
            {"VK_VERSION_1_3", DeviceInfo(&DeviceExtensions::vk_feature_version_1_3, {})},
            {VK_KHR_SWAPCHAIN_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_swapchain,
                                                         {{{&DeviceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_display_swapchain,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_sampler_mirror_clamp_to_edge, {})},
            {VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_video_queue,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                          {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_video_decode_queue,
                        {{{&DeviceExtensions::vk_khr_video_queue, VK_KHR_VIDEO_QUEUE_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_video_decode_h264,
                        {{{&DeviceExtensions::vk_khr_video_decode_queue, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME}}})},
            {VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_dynamic_rendering,
                        {{{&DeviceExtensions::vk_khr_depth_stencil_resolve, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_MULTIVIEW_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_multiview, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_device_group,
                        {{{&DeviceExtensions::vk_khr_device_group_creation, VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME}}})},
            {VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_shader_draw_parameters, {})},
            {VK_KHR_MAINTENANCE_1_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_maintenance1, {})},
            {VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_external_memory, {{{&DeviceExtensions::vk_khr_external_memory_capabilities,
                                                                      VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_external_memory_win32,
                        {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
#endif
            {VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_external_memory_fd,
                        {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {VK_KHR_WIN32_KEYED_MUTEX_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_win32_keyed_mutex,
                        {{{&DeviceExtensions::vk_khr_external_memory_win32, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME}}})},
#endif
            {VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_external_semaphore, {{{&DeviceExtensions::vk_khr_external_semaphore_capabilities,
                                                                         VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_external_semaphore_win32,
                        {{{&DeviceExtensions::vk_khr_external_semaphore, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME}}})},
#endif
            {VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_external_semaphore_fd,
                        {{{&DeviceExtensions::vk_khr_external_semaphore, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME}}})},
            {VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_push_descriptor, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_shader_float16_int8,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_16bit_storage, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                                                                   {&DeviceExtensions::vk_khr_storage_buffer_storage_class,
                                                                    VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME}}})},
            {VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_incremental_present,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_descriptor_update_template, {})},
            {VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_imageless_framebuffer,
                        {{{&DeviceExtensions::vk_khr_maintenance2, VK_KHR_MAINTENANCE_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_image_format_list, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_create_renderpass2,
                        {{{&DeviceExtensions::vk_khr_multiview, VK_KHR_MULTIVIEW_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_maintenance2, VK_KHR_MAINTENANCE_2_EXTENSION_NAME}}})},
            {VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_shared_presentable_image,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_external_fence, {{{&DeviceExtensions::vk_khr_external_fence_capabilities,
                                                                     VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_external_fence_win32,
                        {{{&DeviceExtensions::vk_khr_external_fence, VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME}}})},
#endif
            {VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_external_fence_fd,
                        {{{&DeviceExtensions::vk_khr_external_fence, VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME}}})},
            {VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_performance_query, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_MAINTENANCE_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_maintenance2, {})},
            {VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_variable_pointers, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                                                                       {&DeviceExtensions::vk_khr_storage_buffer_storage_class,
                                                                        VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME}}})},
            {VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_dedicated_allocation,
                        {{{&DeviceExtensions::vk_khr_get_memory_requirements2, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME}}})},
            {VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_storage_buffer_storage_class, {})},
            {VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_relaxed_block_layout, {})},
            {VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_get_memory_requirements2, {})},
            {VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_image_format_list, {})},
            {VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_sampler_ycbcr_conversion,
                        {{{&DeviceExtensions::vk_khr_maintenance1, VK_KHR_MAINTENANCE_1_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_bind_memory2, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_memory_requirements2, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_BIND_MEMORY_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_bind_memory2, {})},
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_portability_subset,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#endif
            {VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_maintenance3, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_draw_indirect_count, {})},
            {VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_shader_subgroup_extended_types,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_8bit_storage, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                                                                  {&DeviceExtensions::vk_khr_storage_buffer_storage_class,
                                                                   VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME}}})},
            {VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_shader_atomic_int64,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_SHADER_CLOCK_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_shader_clock, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_video_decode_h265,
                        {{{&DeviceExtensions::vk_khr_video_decode_queue, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME}}})},
            {VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_global_priority, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_driver_properties, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_shader_float_controls,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_depth_stencil_resolve,
                        {{{&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME}}})},
            {VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_swapchain_mutable_format,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_maintenance2, VK_KHR_MAINTENANCE_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_image_format_list, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME}}})},
            {VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_timeline_semaphore,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_vulkan_memory_model,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_shader_terminate_invocation,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_fragment_shading_rate,
                        {{{&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_SPIRV_1_4_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_spirv_1_4,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                          {&DeviceExtensions::vk_khr_shader_float_controls, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME}}})},
            {VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_separate_depth_stencil_layouts,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME}}})},
            {VK_KHR_PRESENT_WAIT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_present_wait,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_present_id, VK_KHR_PRESENT_ID_EXTENSION_NAME}}})},
            {VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_uniform_buffer_standard_layout,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_buffer_device_address,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_device_group, VK_KHR_DEVICE_GROUP_EXTENSION_NAME}}})},
            {VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_deferred_host_operations, {})},
            {VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_pipeline_executable_properties,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_MAP_MEMORY_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_map_memory2, {})},
            {VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_shader_integer_dot_product,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_pipeline_library, {})},
            {VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_shader_non_semantic_info, {})},
            {VK_KHR_PRESENT_ID_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_present_id,
                                                          {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                                                            {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_video_encode_queue,
                        {{{&DeviceExtensions::vk_khr_video_queue, VK_KHR_VIDEO_QUEUE_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
#endif
            {VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_synchronization2, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_fragment_shader_barycentric,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_shader_subgroup_uniform_control_flow,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_zero_initialize_workgroup_memory,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_workgroup_memory_explicit_layout,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_copy_commands2, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_format_feature_flags2,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_ray_tracing_maintenance1,
                        {{{&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_maintenance4, {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_maintenance5,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                          {&DeviceExtensions::vk_khr_dynamic_rendering, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME}}})},
            {VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_ray_tracing_position_fetch,
                        {{{&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_khr_cooperative_matrix,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_GLSL_SHADER_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_glsl_shader, {})},
            {VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_depth_range_unrestricted, {})},
            {VK_IMG_FILTER_CUBIC_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_img_filter_cubic, {})},
            {VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_rasterization_order, {})},
            {VK_AMD_SHADER_TRINARY_MINMAX_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_shader_trinary_minmax, {})},
            {VK_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amd_shader_explicit_vertex_parameter, {})},
            {VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_debug_marker,
                        {{{&DeviceExtensions::vk_ext_debug_report, VK_EXT_DEBUG_REPORT_EXTENSION_NAME}}})},
            {VK_AMD_GCN_SHADER_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_gcn_shader, {})},
            {VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_dedicated_allocation, {})},
            {VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_transform_feedback,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NVX_BINARY_IMPORT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nvx_binary_import, {})},
            {VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nvx_image_view_handle, {})},
            {VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_draw_indirect_count, {})},
            {VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_negative_viewport_height, {})},
            {VK_AMD_GPU_SHADER_HALF_FLOAT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_gpu_shader_half_float, {})},
            {VK_AMD_SHADER_BALLOT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_shader_ballot, {})},
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_video_encode_h264,
                        {{{&DeviceExtensions::vk_khr_video_encode_queue, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME}}})},
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {VK_EXT_VIDEO_ENCODE_H265_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_video_encode_h265,
                        {{{&DeviceExtensions::vk_khr_video_encode_queue, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME}}})},
#endif
            {VK_AMD_TEXTURE_GATHER_BIAS_LOD_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amd_texture_gather_bias_lod,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_AMD_SHADER_INFO_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_shader_info, {})},
            {VK_AMD_SHADER_IMAGE_LOAD_STORE_LOD_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amd_shader_image_load_store_lod, {})},
            {VK_NV_CORNER_SAMPLED_IMAGE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_corner_sampled_image,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_IMG_FORMAT_PVRTC_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_img_format_pvrtc, {})},
            {VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_external_memory, {{{&DeviceExtensions::vk_nv_external_memory_capabilities,
                                                                     VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_external_memory_win32,
                        {{{&DeviceExtensions::vk_nv_external_memory, VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {VK_NV_WIN32_KEYED_MUTEX_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_win32_keyed_mutex,
                        {{{&DeviceExtensions::vk_nv_external_memory_win32, VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME}}})},
#endif
            {VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_shader_subgroup_ballot, {})},
            {VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_shader_subgroup_vote, {})},
            {VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_texture_compression_astc_hdr,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_astc_decode_mode, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_pipeline_robustness,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_conditional_rendering,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_clip_space_w_scaling, {})},
            {VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_display_control,
                        {{{&DeviceExtensions::vk_ext_display_surface_counter, VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_google_display_timing,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_NV_SAMPLE_MASK_OVERRIDE_COVERAGE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_sample_mask_override_coverage, {})},
            {VK_NV_GEOMETRY_SHADER_PASSTHROUGH_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_geometry_shader_passthrough, {})},
            {VK_NV_VIEWPORT_ARRAY_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_viewport_array2, {})},
            {VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nvx_multiview_per_view_attributes,
                        {{{&DeviceExtensions::vk_khr_multiview, VK_KHR_MULTIVIEW_EXTENSION_NAME}}})},
            {VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_viewport_swizzle, {})},
            {VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_discard_rectangles,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_conservative_rasterization,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_depth_clip_enable, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_HDR_METADATA_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_hdr_metadata,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_external_memory_dma_buf,
                        {{{&DeviceExtensions::vk_khr_external_memory_fd, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME}}})},
            {VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_queue_family_foreign,
                        {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            {VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_android_external_memory_android_hardware_buffer,
                        {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME},
                          {&DeviceExtensions::vk_ext_queue_family_foreign, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_dedicated_allocation, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME}}})},
#endif
            {VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_sampler_filter_minmax,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_AMD_GPU_SHADER_INT16_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_gpu_shader_int16, {})},
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {VK_AMDX_SHADER_ENQUEUE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amdx_shader_enqueue,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_pipeline_library, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_spirv_1_4, VK_KHR_SPIRV_1_4_EXTENSION_NAME}}})},
#endif
            {VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_mixed_attachment_samples, {})},
            {VK_AMD_SHADER_FRAGMENT_MASK_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_shader_fragment_mask, {})},
            {VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_inline_uniform_block,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_maintenance1, VK_KHR_MAINTENANCE_1_EXTENSION_NAME}}})},
            {VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_shader_stencil_export, {})},
            {VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_sample_locations, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_blend_operation_advanced,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_fragment_coverage_to_color, {})},
            {VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_framebuffer_mixed_samples, {})},
            {VK_NV_FILL_RECTANGLE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_fill_rectangle, {})},
            {VK_NV_SHADER_SM_BUILTINS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_shader_sm_builtins,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_post_depth_coverage, {})},
            {VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_image_drm_format_modifier,
                        {{{&DeviceExtensions::vk_khr_bind_memory2, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_image_format_list, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME}}})},
            {VK_EXT_VALIDATION_CACHE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_validation_cache, {})},
            {VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_descriptor_indexing,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_maintenance3, VK_KHR_MAINTENANCE_3_EXTENSION_NAME}}})},
            {VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_shader_viewport_index_layer, {})},
            {VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_shading_rate_image, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_RAY_TRACING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_ray_tracing,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_memory_requirements2, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME}}})},
            {VK_NV_REPRESENTATIVE_FRAGMENT_TEST_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_representative_fragment_test,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_FILTER_CUBIC_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_filter_cubic, {})},
            {VK_QCOM_RENDER_PASS_SHADER_RESOLVE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_render_pass_shader_resolve, {})},
            {VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_global_priority, {})},
            {VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_external_memory_host,
                        {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
            {VK_AMD_BUFFER_MARKER_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_buffer_marker, {})},
            {VK_AMD_PIPELINE_COMPILER_CONTROL_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_amd_pipeline_compiler_control, {})},
            {VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_calibrated_timestamps,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_AMD_SHADER_CORE_PROPERTIES_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amd_shader_core_properties,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_AMD_MEMORY_OVERALLOCATION_BEHAVIOR_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amd_memory_overallocation_behavior, {})},
            {VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_vertex_attribute_divisor,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_GGP
            {VK_GGP_FRAME_TOKEN_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ggp_frame_token,
                                                           {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                                                             {&DeviceExtensions::vk_ggp_stream_descriptor_surface,
                                                              VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME}}})},
#endif
            {VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_pipeline_creation_feedback, {})},
            {VK_NV_SHADER_SUBGROUP_PARTITIONED_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_shader_subgroup_partitioned,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {VK_NV_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_compute_shader_derivatives,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_MESH_SHADER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_mesh_shader, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_fragment_shader_barycentric,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_SHADER_IMAGE_FOOTPRINT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_shader_image_footprint,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_scissor_exclusive, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_device_diagnostic_checkpoints,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_INTEL_SHADER_INTEGER_FUNCTIONS_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_intel_shader_integer_functions2,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_intel_performance_query, {})},
            {VK_EXT_PCI_BUS_INFO_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_pci_bus_info, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_AMD_DISPLAY_NATIVE_HDR_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amd_display_native_hdr,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_fragment_density_map,
                                                                    {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_scalar_block_layout,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_GOOGLE_HLSL_FUNCTIONALITY_1_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_google_hlsl_functionality1, {})},
            {VK_GOOGLE_DECORATE_STRING_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_google_decorate_string, {})},
            {VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_subgroup_size_control,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {VK_AMD_SHADER_CORE_PROPERTIES_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amd_shader_core_properties2,
                        {{{&DeviceExtensions::vk_amd_shader_core_properties, VK_AMD_SHADER_CORE_PROPERTIES_EXTENSION_NAME}}})},
            {VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amd_device_coherent_memory,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_shader_image_atomic_int64,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_memory_budget, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_memory_priority, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_dedicated_allocation_image_aliasing,
                        {{{&DeviceExtensions::vk_khr_dedicated_allocation, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_buffer_device_address,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_TOOLING_INFO_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_tooling_info, {})},
            {VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_separate_stencil_usage, {})},
            {VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_cooperative_matrix, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_coverage_reduction_mode,
                        {{{&DeviceExtensions::vk_nv_framebuffer_mixed_samples, VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_fragment_shader_interlock,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_ycbcr_image_arrays,
                        {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME}}})},
            {VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_provoking_vertex, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_full_screen_exclusive,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
#endif
            {VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_line_rasterization,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_shader_atomic_float,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_host_query_reset, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_index_type_uint8, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_extended_dynamic_state,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_host_image_copy,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_copy_commands2, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_format_feature_flags2, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME}}})},
            {VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_shader_atomic_float2,
                        {{{&DeviceExtensions::vk_ext_shader_atomic_float, VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME}}})},
            {VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_swapchain_maintenance1,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                          {&DeviceExtensions::vk_ext_surface_maintenance1, VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_shader_demote_to_helper_invocation,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_device_generated_commands,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                          {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}}})},
            {VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_inherited_viewport_scissor,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_texel_buffer_alignment,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_render_pass_transform,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_depth_bias_control,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_DEVICE_MEMORY_REPORT_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_device_memory_report,
                                                                    {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_robustness2, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_custom_border_color,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_GOOGLE_USER_TYPE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_google_user_type, {})},
            {VK_NV_PRESENT_BARRIER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_present_barrier,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {VK_EXT_PRIVATE_DATA_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_private_data, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_pipeline_creation_cache_control,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_device_diagnostics_config,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_QCOM_RENDER_PASS_STORE_OPS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_qcom_render_pass_store_ops, {})},
            {VK_NV_LOW_LATENCY_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_low_latency, {})},
#ifdef VK_USE_PLATFORM_METAL_EXT
            {VK_EXT_METAL_OBJECTS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_metal_objects, {})},
#endif
            {VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_descriptor_buffer,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_ext_descriptor_indexing, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME}}})},
            {VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_graphics_pipeline_library,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_pipeline_library, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME}}})},
            {VK_AMD_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_amd_shader_early_and_late_fragment_tests,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_FRAGMENT_SHADING_RATE_ENUMS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_fragment_shading_rate_enums,
                        {{{&DeviceExtensions::vk_khr_fragment_shading_rate, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME}}})},
            {VK_NV_RAY_TRACING_MOTION_BLUR_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_ray_tracing_motion_blur,
                        {{{&DeviceExtensions::vk_khr_ray_tracing_pipeline, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME}}})},
            {VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_ycbcr_2plane_444_formats,
                        {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME}}})},
            {VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_fragment_density_map2,
                        {{{&DeviceExtensions::vk_ext_fragment_density_map, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME}}})},
            {VK_QCOM_ROTATED_COPY_COMMANDS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_rotated_copy_commands,
                        {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_copy_commands2, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME}}})},
            {VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_image_robustness, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_image_compression_control,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_attachment_feedback_loop_layout,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_4444_FORMATS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_4444_formats, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_DEVICE_FAULT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_device_fault, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_arm_rasterization_order_attachment_access,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_RGBA10X6_FORMATS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_rgba10x6_formats,
                        {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {VK_NV_ACQUIRE_WINRT_DISPLAY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_acquire_winrt_display,
                        {{{&DeviceExtensions::vk_ext_direct_mode_display, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME}}})},
#endif
            {VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_valve_mutable_descriptor_type,
                        {{{&DeviceExtensions::vk_khr_maintenance3, VK_KHR_MAINTENANCE_3_EXTENSION_NAME}}})},
            {VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_vertex_input_dynamic_state,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_physical_device_drm,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_device_address_binding_report,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_ext_debug_utils, VK_EXT_DEBUG_UTILS_EXTENSION_NAME}}})},
            {VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_depth_clip_control,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_primitive_topology_list_restart,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_FUCHSIA
            {VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME,
             DeviceInfo(
                 &DeviceExtensions::vk_fuchsia_external_memory,
                 {{{&DeviceExtensions::vk_khr_external_memory_capabilities, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME},
                   {&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
            {VK_FUCHSIA_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_fuchsia_external_semaphore,
                        {{{&DeviceExtensions::vk_khr_external_semaphore_capabilities,
                           VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_external_semaphore, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME}}})},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
            {VK_FUCHSIA_BUFFER_COLLECTION_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_fuchsia_buffer_collection,
                        {{{&DeviceExtensions::vk_fuchsia_external_memory, VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME}}})},
#endif
            {VK_HUAWEI_SUBPASS_SHADING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_huawei_subpass_shading,
                        {{{&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {VK_HUAWEI_INVOCATION_MASK_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_huawei_invocation_mask,
                        {{{&DeviceExtensions::vk_khr_ray_tracing_pipeline, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {VK_NV_EXTERNAL_MEMORY_RDMA_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_external_memory_rdma,
                        {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
            {VK_EXT_PIPELINE_PROPERTIES_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_pipeline_properties,
                                                                   {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_FRAME_BOUNDARY_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_frame_boundary, {})},
            {VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_multisampled_render_to_single_sampled,
                        {{{&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_depth_stencil_resolve, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME}}})},
            {VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_extended_dynamic_state2,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_color_write_enable,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_primitives_generated_query,
                        {{{&DeviceExtensions::vk_ext_transform_feedback, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME}}})},
            {VK_EXT_GLOBAL_PRIORITY_QUERY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_global_priority_query,
                        {{{&DeviceExtensions::vk_ext_global_priority, VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_IMAGE_VIEW_MIN_LOD_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_image_view_min_lod,
                                                                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_MULTI_DRAW_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_multi_draw, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_image_2d_view_of_3d,
                        {{{&DeviceExtensions::vk_khr_maintenance1, VK_KHR_MAINTENANCE_1_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_shader_tile_image,
                        {{{&DeviceExtensions::vk_feature_version_1_3, "VK_VERSION_1_3"}}})},
            {VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_opacity_micromap,
                        {{{&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {VK_NV_DISPLACEMENT_MICROMAP_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_displacement_micromap,
                        {{{&DeviceExtensions::vk_ext_opacity_micromap, VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME}}})},
#endif
            {VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_load_store_op_none, {})},
            {VK_HUAWEI_CLUSTER_CULLING_SHADER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_huawei_cluster_culling_shader,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_BORDER_COLOR_SWIZZLE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_border_color_swizzle,
                        {{{&DeviceExtensions::vk_ext_custom_border_color, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME}}})},
            {VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_pageable_device_local_memory,
                        {{{&DeviceExtensions::vk_ext_memory_priority, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME}}})},
            {VK_ARM_SHADER_CORE_PROPERTIES_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_arm_shader_core_properties,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_image_sliced_view_of_3d,
                        {{{&DeviceExtensions::vk_khr_maintenance1, VK_KHR_MAINTENANCE_1_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_VALVE_DESCRIPTOR_SET_HOST_MAPPING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_valve_descriptor_set_host_mapping,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_DEPTH_CLAMP_ZERO_ONE_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_depth_clamp_zero_one,
                                                                    {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_NON_SEAMLESS_CUBE_MAP_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_non_seamless_cube_map,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_fragment_density_map_offset,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_ext_fragment_density_map, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME}}})},
            {VK_NV_COPY_MEMORY_INDIRECT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_copy_memory_indirect,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}}})},
            {VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_memory_decompression,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}}})},
            {VK_NV_DEVICE_GENERATED_COMMANDS_COMPUTE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_device_generated_commands_compute,
                        {{{&DeviceExtensions::vk_nv_device_generated_commands, VK_NV_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME}}})},
            {VK_NV_LINEAR_COLOR_ATTACHMENT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_linear_color_attachment,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_EXTENSION_NAME,
             DeviceInfo(
                 &DeviceExtensions::vk_ext_image_compression_control_swapchain,
                 {{{&DeviceExtensions::vk_ext_image_compression_control, VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME}}})},
            {VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_image_processing,
                        {{{&DeviceExtensions::vk_khr_format_feature_flags2, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME}}})},
            {VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_ext_nested_command_buffer,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_external_memory_acquire_unmodified,
                        {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
            {VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_extended_dynamic_state3,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_SUBPASS_MERGE_FEEDBACK_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_subpass_merge_feedback,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_shader_module_identifier,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_ext_pipeline_creation_cache_control,
                           VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME}}})},
            {VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_rasterization_order_attachment_access,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_OPTICAL_FLOW_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_optical_flow,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_format_feature_flags2, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {VK_EXT_LEGACY_DITHERING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_legacy_dithering, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_PIPELINE_PROTECTED_ACCESS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_pipeline_protected_access,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            {VK_ANDROID_EXTERNAL_FORMAT_RESOLVE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_android_external_format_resolve,
                        {{{&DeviceExtensions::vk_android_external_memory_android_hardware_buffer,
                           VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME}}})},
#endif
            {VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_shader_object,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_dynamic_rendering, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME}}})},
            {VK_QCOM_TILE_PROPERTIES_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_tile_properties, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_SEC_AMIGO_PROFILING_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_sec_amigo_profiling, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_QCOM_MULTIVIEW_PER_VIEW_VIEWPORTS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_multiview_per_view_viewports,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_ray_tracing_invocation_reorder,
                        {{{&DeviceExtensions::vk_khr_ray_tracing_pipeline, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME}}})},
            {VK_NV_EXTENDED_SPARSE_ADDRESS_SPACE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_extended_sparse_address_space, {})},
            {VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_mutable_descriptor_type,
                        {{{&DeviceExtensions::vk_khr_maintenance3, VK_KHR_MAINTENANCE_3_EXTENSION_NAME}}})},
            {VK_ARM_SHADER_CORE_BUILTINS_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_arm_shader_core_builtins,
                                                                    {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_EXT_PIPELINE_LIBRARY_GROUP_HANDLES_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_pipeline_library_group_handles,
                        {{{&DeviceExtensions::vk_khr_ray_tracing_pipeline, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_pipeline_library, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME}}})},
            {VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_dynamic_rendering_unused_attachments,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_dynamic_rendering, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME}}})},
            {VK_NV_LOW_LATENCY_2_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_nv_low_latency2, {})},
            {VK_QCOM_MULTIVIEW_PER_VIEW_RENDER_AREAS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_multiview_per_view_render_areas, {})},
            {VK_QCOM_IMAGE_PROCESSING_2_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_image_processing2,
                        {{{&DeviceExtensions::vk_qcom_image_processing, VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME}}})},
            {VK_QCOM_FILTER_CUBIC_WEIGHTS_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_filter_cubic_weights,
                        {{{&DeviceExtensions::vk_ext_filter_cubic, VK_EXT_FILTER_CUBIC_EXTENSION_NAME}}})},
            {VK_QCOM_YCBCR_DEGAMMA_EXTENSION_NAME, DeviceInfo(&DeviceExtensions::vk_qcom_ycbcr_degamma, {})},
            {VK_QCOM_FILTER_CUBIC_CLAMP_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qcom_filter_cubic_clamp,
                        {{{&DeviceExtensions::vk_ext_filter_cubic, VK_EXT_FILTER_CUBIC_EXTENSION_NAME},
                          {&DeviceExtensions::vk_feature_version_1_2, "VK_VERSION_1_2"},
                          {&DeviceExtensions::vk_ext_sampler_filter_minmax, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME}}})},
            {VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_attachment_feedback_loop_dynamic_state,
                        {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                          {&DeviceExtensions::vk_ext_attachment_feedback_loop_layout,
                           VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            {VK_QNX_EXTERNAL_MEMORY_SCREEN_BUFFER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_qnx_external_memory_screen_buffer,
                        {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_dedicated_allocation, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME},
                          {&DeviceExtensions::vk_ext_queue_family_foreign, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME}}})},
#endif
            {VK_MSFT_LAYERED_DRIVER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_msft_layered_driver, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {VK_NV_DESCRIPTOR_POOL_OVERALLOCATION_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_nv_descriptor_pool_overallocation,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_acceleration_structure,
                        {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                          {&DeviceExtensions::vk_ext_descriptor_indexing, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_deferred_host_operations, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME}}})},
            {VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_ray_tracing_pipeline,
                        {{{&DeviceExtensions::vk_khr_spirv_1_4, VK_KHR_SPIRV_1_4_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {VK_KHR_RAY_QUERY_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_khr_ray_query,
                        {{{&DeviceExtensions::vk_khr_spirv_1_4, VK_KHR_SPIRV_1_4_EXTENSION_NAME},
                          {&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {VK_EXT_MESH_SHADER_EXTENSION_NAME,
             DeviceInfo(&DeviceExtensions::vk_ext_mesh_shader,
                        {{{&DeviceExtensions::vk_khr_spirv_1_4, VK_KHR_SPIRV_1_4_EXTENSION_NAME}}})},

        };

        return info_map;
    }

    static const DeviceInfo &get_info(const char *name) {
        static const DeviceInfo empty_info{nullptr, DeviceReqVec()};
        const auto &ext_map = DeviceExtensions::get_info_map();
        const auto info = ext_map.find(name);
        if (info != ext_map.cend()) {
            return info->second;
        }
        return empty_info;
    }

    DeviceExtensions() = default;
    DeviceExtensions(const InstanceExtensions &instance_ext) : InstanceExtensions(instance_ext) {}

    APIVersion InitFromDeviceCreateInfo(const InstanceExtensions *instance_extensions, APIVersion requested_api_version,
                                        const VkDeviceCreateInfo *pCreateInfo = nullptr) {
        // Initialize: this to defaults,  base class fields to input.
        assert(instance_extensions);
        *this = DeviceExtensions(*instance_extensions);
        constexpr std::array<const char *, 18> V_1_1_promoted_device_apis = {
            VK_KHR_MULTIVIEW_EXTENSION_NAME,
            VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_1_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
            VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
            VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
            VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_2_EXTENSION_NAME,
            VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME,
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
            VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME,
            VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
            VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
            VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
        };
        constexpr std::array<const char *, 24> V_1_2_promoted_device_apis = {
            VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
            VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
            VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME,
            VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
            VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
            VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
            VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME,
            VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
            VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME,
            VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
            VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
            VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
            VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
            VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
            VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME,
            VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
            VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME,
            VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
        };
        constexpr std::array<const char *, 23> V_1_3_promoted_device_apis = {
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME,
            VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME,
            VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
            VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME,
            VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
            VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
            VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME,
            VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME,
            VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME,
            VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME,
            VK_EXT_TOOLING_INFO_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
            VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
            VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME,
            VK_EXT_PRIVATE_DATA_EXTENSION_NAME,
            VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME,
            VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME,
            VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME,
            VK_EXT_4444_FORMATS_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
        };

        // Initialize struct data, robust to invalid pCreateInfo
        auto api_version = NormalizeApiVersion(requested_api_version);
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
        if (api_version >= VK_API_VERSION_1_3) {
            auto info = get_info("VK_VERSION_1_3");
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            for (auto promoted_ext : V_1_3_promoted_device_apis) {
                info = get_info(promoted_ext);
                assert(info.state);
                if (info.state) this->*(info.state) = kEnabledByApiLevel;
            }
        }
        // CreateInfo takes precedence over promoted
        if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
            for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
                auto info = get_info(pCreateInfo->ppEnabledExtensionNames[i]);
                if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            }
        }
        // Workaround for functions being introduced by multiple extensions, until the layer is fixed to handle this correctly
        // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5579 and
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5600
        {
            constexpr std::array shader_object_interactions = {
                VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
                VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
                VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
                VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
            };
            auto info = get_info(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
            if (info.state) {
                if (this->*(info.state) != kNotEnabled) {
                    for (auto interaction_ext : shader_object_interactions) {
                        info = get_info(interaction_ext);
                        assert(info.state);
                        if (this->*(info.state) != kEnabledByCreateinfo) {
                            this->*(info.state) = kEnabledByInteraction;
                        }
                    }
                }
            }
        }
        return api_version;
    }
};

static const std::set<std::string> kDeviceExtensionNames = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
    VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_MULTIVIEW_EXTENSION_NAME,
    VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
    VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
    VK_KHR_MAINTENANCE_1_EXTENSION_NAME,
    VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
#endif
    VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_KHR_WIN32_KEYED_MUTEX_EXTENSION_NAME,
#endif
    VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
#endif
    VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
    VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
    VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
    VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
    VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME,
    VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
    VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
    VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME,
    VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,
#endif
    VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME,
    VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME,
    VK_KHR_MAINTENANCE_2_EXTENSION_NAME,
    VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME,
    VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
    VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME,
    VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME,
    VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
    VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
    VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
    VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
    VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
    VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
    VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME,
    VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
    VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME,
    VK_KHR_SHADER_CLOCK_EXTENSION_NAME,
    VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME,
    VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME,
    VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
    VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
    VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
    VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME,
    VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME,
    VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
    VK_KHR_PRESENT_WAIT_EXTENSION_NAME,
    VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
    VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME,
    VK_KHR_MAP_MEMORY_2_EXTENSION_NAME,
    VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME,
    VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
    VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
    VK_KHR_PRESENT_ID_EXTENSION_NAME,
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME,
#endif
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME,
    VK_KHR_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_EXTENSION_NAME,
    VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME,
    VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME,
    VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
    VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME,
    VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
    VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME,
    VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME,
    VK_NV_GLSL_SHADER_EXTENSION_NAME,
    VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME,
    VK_IMG_FILTER_CUBIC_EXTENSION_NAME,
    VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME,
    VK_AMD_SHADER_TRINARY_MINMAX_EXTENSION_NAME,
    VK_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER_EXTENSION_NAME,
    VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
    VK_AMD_GCN_SHADER_EXTENSION_NAME,
    VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME,
    VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME,
    VK_NVX_BINARY_IMPORT_EXTENSION_NAME,
    VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME,
    VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
    VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME,
    VK_AMD_GPU_SHADER_HALF_FLOAT_EXTENSION_NAME,
    VK_AMD_SHADER_BALLOT_EXTENSION_NAME,
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_EXT_VIDEO_ENCODE_H265_EXTENSION_NAME,
#endif
    VK_AMD_TEXTURE_GATHER_BIAS_LOD_EXTENSION_NAME,
    VK_AMD_SHADER_INFO_EXTENSION_NAME,
    VK_AMD_SHADER_IMAGE_LOAD_STORE_LOD_EXTENSION_NAME,
    VK_NV_CORNER_SAMPLED_IMAGE_EXTENSION_NAME,
    VK_IMG_FORMAT_PVRTC_EXTENSION_NAME,
    VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_NV_WIN32_KEYED_MUTEX_EXTENSION_NAME,
#endif
    VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME,
    VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME,
    VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME,
    VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME,
    VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME,
    VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME,
    VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME,
    VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME,
    VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME,
    VK_NV_SAMPLE_MASK_OVERRIDE_COVERAGE_EXTENSION_NAME,
    VK_NV_GEOMETRY_SHADER_PASSTHROUGH_EXTENSION_NAME,
    VK_NV_VIEWPORT_ARRAY_2_EXTENSION_NAME,
    VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME,
    VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME,
    VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME,
    VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,
    VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME,
    VK_EXT_HDR_METADATA_EXTENSION_NAME,
    VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
    VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME,
#endif
    VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME,
    VK_AMD_GPU_SHADER_INT16_EXTENSION_NAME,
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_AMDX_SHADER_ENQUEUE_EXTENSION_NAME,
#endif
    VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME,
    VK_AMD_SHADER_FRAGMENT_MASK_EXTENSION_NAME,
    VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME,
    VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME,
    VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME,
    VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME,
    VK_NV_FRAGMENT_COVERAGE_TO_COLOR_EXTENSION_NAME,
    VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME,
    VK_NV_FILL_RECTANGLE_EXTENSION_NAME,
    VK_NV_SHADER_SM_BUILTINS_EXTENSION_NAME,
    VK_EXT_POST_DEPTH_COVERAGE_EXTENSION_NAME,
    VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
    VK_EXT_VALIDATION_CACHE_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME,
    VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME,
    VK_NV_RAY_TRACING_EXTENSION_NAME,
    VK_NV_REPRESENTATIVE_FRAGMENT_TEST_EXTENSION_NAME,
    VK_EXT_FILTER_CUBIC_EXTENSION_NAME,
    VK_QCOM_RENDER_PASS_SHADER_RESOLVE_EXTENSION_NAME,
    VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME,
    VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME,
    VK_AMD_BUFFER_MARKER_EXTENSION_NAME,
    VK_AMD_PIPELINE_COMPILER_CONTROL_EXTENSION_NAME,
    VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME,
    VK_AMD_SHADER_CORE_PROPERTIES_EXTENSION_NAME,
    VK_AMD_MEMORY_OVERALLOCATION_BEHAVIOR_EXTENSION_NAME,
    VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_GGP
    VK_GGP_FRAME_TOKEN_EXTENSION_NAME,
#endif
    VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME,
    VK_NV_SHADER_SUBGROUP_PARTITIONED_EXTENSION_NAME,
    VK_NV_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME,
    VK_NV_MESH_SHADER_EXTENSION_NAME,
    VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME,
    VK_NV_SHADER_IMAGE_FOOTPRINT_EXTENSION_NAME,
    VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME,
    VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME,
    VK_INTEL_SHADER_INTEGER_FUNCTIONS_2_EXTENSION_NAME,
    VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME,
    VK_EXT_PCI_BUS_INFO_EXTENSION_NAME,
    VK_AMD_DISPLAY_NATIVE_HDR_EXTENSION_NAME,
    VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME,
    VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
    VK_GOOGLE_HLSL_FUNCTIONALITY_1_EXTENSION_NAME,
    VK_GOOGLE_DECORATE_STRING_EXTENSION_NAME,
    VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME,
    VK_AMD_SHADER_CORE_PROPERTIES_2_EXTENSION_NAME,
    VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME,
    VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME,
    VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
    VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME,
    VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME,
    VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_EXT_TOOLING_INFO_EXTENSION_NAME,
    VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME,
    VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME,
    VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME,
    VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME,
    VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME,
    VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME,
#endif
    VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME,
    VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME,
    VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
    VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME,
    VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
    VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME,
    VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME,
    VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME,
    VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
    VK_NV_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME,
    VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME,
    VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME,
    VK_QCOM_RENDER_PASS_TRANSFORM_EXTENSION_NAME,
    VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME,
    VK_EXT_DEVICE_MEMORY_REPORT_EXTENSION_NAME,
    VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
    VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME,
    VK_GOOGLE_USER_TYPE_EXTENSION_NAME,
    VK_NV_PRESENT_BARRIER_EXTENSION_NAME,
    VK_EXT_PRIVATE_DATA_EXTENSION_NAME,
    VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME,
    VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME,
    VK_QCOM_RENDER_PASS_STORE_OPS_EXTENSION_NAME,
    VK_NV_LOW_LATENCY_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_METAL_EXT
    VK_EXT_METAL_OBJECTS_EXTENSION_NAME,
#endif
    VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
    VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME,
    VK_AMD_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_EXTENSION_NAME,
    VK_NV_FRAGMENT_SHADING_RATE_ENUMS_EXTENSION_NAME,
    VK_NV_RAY_TRACING_MOTION_BLUR_EXTENSION_NAME,
    VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME,
    VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME,
    VK_QCOM_ROTATED_COPY_COMMANDS_EXTENSION_NAME,
    VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME,
    VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME,
    VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME,
    VK_EXT_4444_FORMATS_EXTENSION_NAME,
    VK_EXT_DEVICE_FAULT_EXTENSION_NAME,
    VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME,
    VK_EXT_RGBA10X6_FORMATS_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_NV_ACQUIRE_WINRT_DISPLAY_EXTENSION_NAME,
#endif
    VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
    VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
    VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME,
    VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME,
    VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME,
    VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_FUCHSIA
    VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    VK_FUCHSIA_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    VK_FUCHSIA_BUFFER_COLLECTION_EXTENSION_NAME,
#endif
    VK_HUAWEI_SUBPASS_SHADING_EXTENSION_NAME,
    VK_HUAWEI_INVOCATION_MASK_EXTENSION_NAME,
    VK_NV_EXTERNAL_MEMORY_RDMA_EXTENSION_NAME,
    VK_EXT_PIPELINE_PROPERTIES_EXTENSION_NAME,
    VK_EXT_FRAME_BOUNDARY_EXTENSION_NAME,
    VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME,
    VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
    VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME,
    VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME,
    VK_EXT_GLOBAL_PRIORITY_QUERY_EXTENSION_NAME,
    VK_EXT_IMAGE_VIEW_MIN_LOD_EXTENSION_NAME,
    VK_EXT_MULTI_DRAW_EXTENSION_NAME,
    VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME,
    VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME,
    VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME,
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_NV_DISPLACEMENT_MICROMAP_EXTENSION_NAME,
#endif
    VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME,
    VK_HUAWEI_CLUSTER_CULLING_SHADER_EXTENSION_NAME,
    VK_EXT_BORDER_COLOR_SWIZZLE_EXTENSION_NAME,
    VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME,
    VK_ARM_SHADER_CORE_PROPERTIES_EXTENSION_NAME,
    VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME,
    VK_VALVE_DESCRIPTOR_SET_HOST_MAPPING_EXTENSION_NAME,
    VK_EXT_DEPTH_CLAMP_ZERO_ONE_EXTENSION_NAME,
    VK_EXT_NON_SEAMLESS_CUBE_MAP_EXTENSION_NAME,
    VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME,
    VK_NV_COPY_MEMORY_INDIRECT_EXTENSION_NAME,
    VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME,
    VK_NV_DEVICE_GENERATED_COMMANDS_COMPUTE_EXTENSION_NAME,
    VK_NV_LINEAR_COLOR_ATTACHMENT_EXTENSION_NAME,
    VK_EXT_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_EXTENSION_NAME,
    VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME,
    VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
    VK_EXT_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXTENSION_NAME,
    VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
    VK_EXT_SUBPASS_MERGE_FEEDBACK_EXTENSION_NAME,
    VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME,
    VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME,
    VK_NV_OPTICAL_FLOW_EXTENSION_NAME,
    VK_EXT_LEGACY_DITHERING_EXTENSION_NAME,
    VK_EXT_PIPELINE_PROTECTED_ACCESS_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    VK_ANDROID_EXTERNAL_FORMAT_RESOLVE_EXTENSION_NAME,
#endif
    VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
    VK_QCOM_TILE_PROPERTIES_EXTENSION_NAME,
    VK_SEC_AMIGO_PROFILING_EXTENSION_NAME,
    VK_QCOM_MULTIVIEW_PER_VIEW_VIEWPORTS_EXTENSION_NAME,
    VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME,
    VK_NV_EXTENDED_SPARSE_ADDRESS_SPACE_EXTENSION_NAME,
    VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
    VK_ARM_SHADER_CORE_BUILTINS_EXTENSION_NAME,
    VK_EXT_PIPELINE_LIBRARY_GROUP_HANDLES_EXTENSION_NAME,
    VK_EXT_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_EXTENSION_NAME,
    VK_NV_LOW_LATENCY_2_EXTENSION_NAME,
    VK_QCOM_MULTIVIEW_PER_VIEW_RENDER_AREAS_EXTENSION_NAME,
    VK_QCOM_IMAGE_PROCESSING_2_EXTENSION_NAME,
    VK_QCOM_FILTER_CUBIC_WEIGHTS_EXTENSION_NAME,
    VK_QCOM_YCBCR_DEGAMMA_EXTENSION_NAME,
    VK_QCOM_FILTER_CUBIC_CLAMP_EXTENSION_NAME,
    VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    VK_QNX_EXTERNAL_MEMORY_SCREEN_BUFFER_EXTENSION_NAME,
#endif
    VK_MSFT_LAYERED_DRIVER_EXTENSION_NAME,
    VK_NV_DESCRIPTOR_POOL_OVERALLOCATION_EXTENSION_NAME,
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_RAY_QUERY_EXTENSION_NAME,
    VK_EXT_MESH_SHADER_EXTENSION_NAME,
};
// NOLINTEND
