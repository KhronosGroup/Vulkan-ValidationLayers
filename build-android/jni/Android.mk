# Copyright 2015 The Android Open Source Project
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Valve Corporation

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)
SRC_DIR := ../..
THIRD_PARTY := ../third_party

# Treat these as system includes to match the CMake build. We don't care about warnings from headers we don't control.
VULKAN_INCLUDE := $(LOCAL_PATH)/$(THIRD_PARTY)/Vulkan-Headers/include
VVL_EXTERNAL_INCLUDE := $(LOCAL_PATH)/$(SRC_DIR)/layers/external
ROBIN_HOOD_INCLUDE := $(LOCAL_PATH)/$(THIRD_PARTY)/robin-hood-hashing/src/include

include $(CLEAR_VARS)
LOCAL_MODULE := layer_utils
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vk_layer_config.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/utils/vk_layer_extension_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/error_message/logging.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/utils/vk_layer_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/utils/ray_tracing_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/vk_format_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/external/xxhash.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(SRC_DIR)/layers/vulkan \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers
LOCAL_CPPFLAGS += -isystem $(VULKAN_INCLUDE)
LOCAL_CPPFLAGS += -isystem $(VVL_EXTERNAL_INCLUDE)
LOCAL_CPPFLAGS += -isystem $(ROBIN_HOOD_INCLUDE)
LOCAL_CPPFLAGS += -std=c++17 -fvisibility=hidden
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DUSE_ROBIN_HOOD_HASHING -DXXH_NO_LONG_LONG
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := VkLayer_khronos_validation
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/state_tracker.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_android.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_device.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/device_memory_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_device_memory.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_external_object.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/base_node.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/buffer_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/cmd_buffer_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_cmd_buffer_dynamic.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_cmd_buffer.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_copy_blit_resolve.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/image_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/pipeline_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/pipeline_layout_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/pipeline_sub_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_image.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_image_layout.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_pipeline_compute.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_pipeline_graphics.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_pipeline_ray_tracing.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_pipeline.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/queue_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/render_pass_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_render_pass.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/video_session_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_video.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_drawdispatch.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/descriptor_sets.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_descriptor.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_buffer.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/shader_module.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/shader_instruction.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_spirv.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_shader_interface.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_shader_object.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_synchronization.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/spirv_validation_helper.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/spirv_grammar_helper.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/command_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/dynamic_state_helper.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/error_location_helper.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/gpu_validation/gpu_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/gpu_validation/gpu_state_tracker.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/gpu_validation/gpu_error_message.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/gpu_validation/gv_descriptor_sets.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/gpu_validation/debug_printf.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/best_practices/best_practices_utils.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_buffer.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_cmd_buffer.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_copy_blit_resolve.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_descriptor.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_device_memory.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_drawdispatch.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_framebuffer.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_image.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_instance_device.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_pipeline.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_ray_tracing.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_render_pass.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_synchronization.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_video.cpp
LOCAL_SRC_FILES += ${SRC_DIR}/layers/best_practices/bp_wsi.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/best_practices.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/sync/sync_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/sync/sync_vuid_maps.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/error_message/core_error_location.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/sync_validation_types.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/sync/sync_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/utils/convert_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/layer_chassis_dispatch.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/layer_chassis_dispatch_manual.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/chassis.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/valid_enum_values.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/layer_options.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_query.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_queue.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_ray_tracing.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_wsi.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_ycbcr.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/stateless_validation_helper.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_buffer.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_cmd_buffer_dynamic.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_cmd_buffer.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_descriptor.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_device_memory.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_external_object.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_framebuffer.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_image.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_instance_device.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_pipeline.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_ray_tracing.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_render_pass.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_shader_object.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_synchronization.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/sl_wsi.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/object_tracker.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/object_tracker/object_tracker_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/thread_safety.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/thread_tracker/thread_safety_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_core.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_khr.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_ext.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_vendor.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/image_layout_map.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/containers/subresource_adapter.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/external/vma/vma.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/utils/shader_utils.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(SRC_DIR)/layers \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers/vulkan
LOCAL_CPPFLAGS += -isystem $(VULKAN_INCLUDE)
LOCAL_CPPFLAGS += -isystem $(VVL_EXTERNAL_INCLUDE)
LOCAL_CPPFLAGS += -isystem $(ROBIN_HOOD_INCLUDE)
LOCAL_CPPFLAGS += -isystem $(LOCAL_PATH)/$(THIRD_PARTY)/shaderc/third_party/spirv-tools/external/spirv-headers/include
LOCAL_STATIC_LIBRARIES += layer_utils glslang SPIRV-Tools SPIRV-Tools-opt
LOCAL_CPPFLAGS += -std=c++17 -frtti -fvisibility=hidden
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DUSE_ROBIN_HOOD_HASHING -DXXH_NO_LONG_LONG
LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDFLAGS   += -Wl,-Bsymbolic
LOCAL_LDFLAGS   += -Wl,--exclude-libs,ALL
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := VulkanLayerValidationTests
LOCAL_SRC_FILES += $(SRC_DIR)/tests/framework/layer_validation_tests.cpp \
                   $(SRC_DIR)/tests/unit/instanceless.cpp \
                   $(SRC_DIR)/tests/unit/pipeline_advanced_blend.cpp \
                   $(SRC_DIR)/tests/unit/pipeline_layout.cpp \
                   $(SRC_DIR)/tests/unit/pipeline_topology.cpp \
                   $(SRC_DIR)/tests/unit/pipeline.cpp \
                   $(SRC_DIR)/tests/unit/buffer.cpp \
                   $(SRC_DIR)/tests/unit/external_memory_sync.cpp \
                   $(SRC_DIR)/tests/unit/image.cpp \
                   $(SRC_DIR)/tests/unit/memory.cpp \
                   $(SRC_DIR)/tests/unit/object_lifetime.cpp \
                   $(SRC_DIR)/tests/unit/sampler.cpp \
                   $(SRC_DIR)/tests/unit/shader_compute.cpp \
                   $(SRC_DIR)/tests/unit/shader_interface.cpp \
                   $(SRC_DIR)/tests/unit/shader_limits.cpp \
                   $(SRC_DIR)/tests/unit/shader_push_constants.cpp \
                   $(SRC_DIR)/tests/unit/shader_mesh.cpp \
                   $(SRC_DIR)/tests/unit/shader_spirv.cpp \
                   $(SRC_DIR)/tests/unit/shader_storage_image.cpp \
                   $(SRC_DIR)/tests/unit/shader_storage_texel.cpp \
                   $(SRC_DIR)/tests/unit/sparse.cpp \
                   $(SRC_DIR)/tests/unit/sync_object.cpp \
                   $(SRC_DIR)/tests/unit/ycbcr.cpp \
                   $(SRC_DIR)/tests/unit/others.cpp \
                   $(SRC_DIR)/tests/unit/query.cpp \
                   $(SRC_DIR)/tests/unit/atomics.cpp \
                   $(SRC_DIR)/tests/unit/descriptor_buffer.cpp \
                   $(SRC_DIR)/tests/unit/descriptor_indexing.cpp \
                   $(SRC_DIR)/tests/unit/descriptor_indexing_positive.cpp \
                   $(SRC_DIR)/tests/unit/descriptors.cpp \
                   $(SRC_DIR)/tests/unit/device_queue.cpp \
                   $(SRC_DIR)/tests/unit/renderpass.cpp \
                   $(SRC_DIR)/tests/unit/robustness.cpp \
                   $(SRC_DIR)/tests/unit/command.cpp \
                   $(SRC_DIR)/tests/unit/dynamic_state.cpp \
                   $(SRC_DIR)/tests/unit/fragment_shading_rate.cpp \
                   $(SRC_DIR)/tests/unit/multi_device.cpp \
                   $(SRC_DIR)/tests/unit/multiview.cpp \
                   $(SRC_DIR)/tests/unit/transform_feedback.cpp \
                   $(SRC_DIR)/tests/unit/subgroups.cpp \
                   $(SRC_DIR)/tests/unit/subpass.cpp \
                   $(SRC_DIR)/tests/unit/mesh.cpp \
                   $(SRC_DIR)/tests/unit/protected_memory.cpp \
                   $(SRC_DIR)/tests/unit/geometry_tessellation.cpp \
                   $(SRC_DIR)/tests/unit/vertex_input.cpp \
                   $(SRC_DIR)/tests/unit/gpu_av.cpp \
                   $(SRC_DIR)/tests/unit/debug_printf.cpp \
                   $(SRC_DIR)/tests/unit/best_practices.cpp \
                   $(SRC_DIR)/tests/unit/arm_best_practices.cpp \
                   $(SRC_DIR)/tests/unit/wsi.cpp \
                   $(SRC_DIR)/tests/unit/imageless_framebuffer.cpp \
                   $(SRC_DIR)/tests/unit/graphics_library.cpp \
                   $(SRC_DIR)/tests/unit/android_hardware_buffer.cpp \
                   $(SRC_DIR)/tests/unit/ray_tracing.cpp \
                   $(SRC_DIR)/tests/unit/ray_tracing_pipeline.cpp \
                   $(SRC_DIR)/tests/unit/ray_tracing_gpu.cpp \
                   $(SRC_DIR)/tests/unit/buffer_positive.cpp \
                   $(SRC_DIR)/tests/unit/command_positive.cpp \
                   $(SRC_DIR)/tests/unit/descriptor_buffer_positive.cpp \
                   $(SRC_DIR)/tests/unit/descriptors_positive.cpp \
                   $(SRC_DIR)/tests/unit/dynamic_state_positive.cpp \
                   $(SRC_DIR)/tests/unit/external_memory_sync_positive.cpp \
                   $(SRC_DIR)/tests/unit/image_positive.cpp \
                   $(SRC_DIR)/tests/unit/imageless_framebuffer_positive.cpp \
                   $(SRC_DIR)/tests/unit/graphics_library_positive.cpp \
                   $(SRC_DIR)/tests/unit/gpu_av_positive.cpp \
                   $(SRC_DIR)/tests/unit/instance_positive.cpp \
                   $(SRC_DIR)/tests/unit/layer_utils_positive.cpp \
                   $(SRC_DIR)/tests/unit/format_utils_positive.cpp \
                   $(SRC_DIR)/tests/unit/other_positive.cpp \
                   $(SRC_DIR)/tests/unit/pipeline_layout_positive.cpp \
                   $(SRC_DIR)/tests/unit/pipeline_topology_positive.cpp \
                   $(SRC_DIR)/tests/unit/pipeline_positive.cpp \
                   $(SRC_DIR)/tests/unit/protected_memory_positive.cpp \
                   $(SRC_DIR)/tests/unit/query_positive.cpp \
                   $(SRC_DIR)/tests/unit/render_pass_positive.cpp \
                   $(SRC_DIR)/tests/unit/robustness_positive.cpp \
                   $(SRC_DIR)/tests/unit/sampler_positive.cpp \
                   $(SRC_DIR)/tests/unit/shader_compute_positive.cpp \
                   $(SRC_DIR)/tests/unit/shader_image_access_positive.cpp \
                   $(SRC_DIR)/tests/unit/shader_interface_positive.cpp \
                   $(SRC_DIR)/tests/unit/shader_limits_positive.cpp \
                   $(SRC_DIR)/tests/unit/shader_push_constants_positive.cpp \
                   $(SRC_DIR)/tests/unit/shader_spirv_positive.cpp \
                   $(SRC_DIR)/tests/unit/shader_storage_image_positive.cpp \
                   $(SRC_DIR)/tests/unit/shader_storage_texel_positive.cpp \
                   $(SRC_DIR)/tests/unit/sparse_positive.cpp \
                   $(SRC_DIR)/tests/unit/subpass_positive.cpp \
                   $(SRC_DIR)/tests/unit/sync_object_positive.cpp \
                   $(SRC_DIR)/tests/unit/tooling_positive.cpp \
                   $(SRC_DIR)/tests/unit/vertex_input_positive.cpp \
                   $(SRC_DIR)/tests/unit/android_hardware_buffer_positive.cpp \
                   $(SRC_DIR)/tests/unit/atomics_positive.cpp \
                   $(SRC_DIR)/tests/unit/ray_tracing_positive.cpp \
                   $(SRC_DIR)/tests/unit/ray_tracing_pipeline_positive.cpp \
                   $(SRC_DIR)/tests/unit/ycbcr_positive.cpp \
                   $(SRC_DIR)/tests/unit/wsi_positive.cpp \
                   $(SRC_DIR)/tests/unit/sync_val.cpp \
                   $(SRC_DIR)/tests/containers/small_vector.cpp \
                   $(SRC_DIR)/tests/framework/binding.cpp \
                   $(SRC_DIR)/tests/framework/test_framework_android.cpp \
                   $(SRC_DIR)/tests/framework/error_monitor.cpp \
                   $(SRC_DIR)/tests/framework/render.cpp \
                   $(SRC_DIR)/tests/framework/ray_tracing_objects.cpp \
                   $(SRC_DIR)/tests/framework/ray_tracing_nv.cpp \
                   $(SRC_DIR)/layers/utils/convert_utils.cpp \
                   $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_utils.cpp \
                   $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_core.cpp \
                   $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_khr.cpp \
                   $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_ext.cpp \
                   $(SRC_DIR)/layers/vulkan/generated/vk_safe_struct_vendor.cpp \
                   $(SRC_DIR)/layers/vulkan/generated/vk_function_pointers.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(SRC_DIR)/layers/vulkan \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers
LOCAL_CPPFLAGS += -isystem $(VULKAN_INCLUDE)
LOCAL_CPPFLAGS += -isystem $(ROBIN_HOOD_INCLUDE)
LOCAL_STATIC_LIBRARIES := googletest_main layer_utils shaderc
LOCAL_CPPFLAGS += -std=c++17 -fvisibility=hidden
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DUSE_ROBIN_HOOD_HASHING
LOCAL_WHOLE_STATIC_LIBRARIES += android_native_app_glue
LOCAL_LDLIBS := -llog -landroid -ldl

# Force export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
#
# NOTE! This has been fixed for a while now (2017). Is it still reasonable to force export this function via the linker?
LOCAL_LDFLAGS := -u ANativeActivity_onCreate

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,third_party/googletest)
$(call import-module,third_party/shaderc)
