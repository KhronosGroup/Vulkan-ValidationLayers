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

VULKAN_INCLUDE := $(LOCAL_PATH)/$(THIRD_PARTY)/Vulkan-Headers/include

include $(CLEAR_VARS)
LOCAL_MODULE := layer_utils
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vk_layer_config.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vk_layer_extension_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vk_layer_logging.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/vk_layer_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/vk_format_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/xxhash.cpp
LOCAL_C_INCLUDES += $(VULKAN_INCLUDE) \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers/generated \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers \
                    $(LOCAL_PATH)/$(THIRD_PARTY)/robin-hood-hashing/src/include
LOCAL_CPPFLAGS += -std=c++17 -Wall -Werror -Wno-unused-function -Wno-unused-const-variable -fvisibility=hidden
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DVK_PROTOTYPES -DUSE_ROBIN_HOOD_HASHING -DXXH_NO_LONG_LONG
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := VkLayer_khronos_validation
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/state_tracker.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/android_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/device_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/device_memory_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/device_memory_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/external_object_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/base_node.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/buffer_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/cmd_buffer_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cmd_buffer_dynamic_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cmd_buffer_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/copy_blit_resolve_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/image_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/pipeline_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/pipeline_layout_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/pipeline_sub_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/image_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/image_layout_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/pipeline_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/queue_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/render_pass_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/render_pass_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/video_session_state.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/video_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/drawdispatch_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/descriptor_sets.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/descriptor_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/buffer_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/shader_module.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/shader_instruction.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/shader_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/synchronization_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/spirv_validation_helper.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/spirv_grammar_helper.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/command_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/gpu_validation/gpu_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/gpu_validation/gpu_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/gpu_validation/debug_printf.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/best_practices/best_practices_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/best_practices.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/sync/sync_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/sync/sync_vuid_maps.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_error_location.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/sync_validation_types.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/sync/sync_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/convert_to_renderpass2.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/layer_chassis_dispatch.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/chassis.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/valid_param_values.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/layer_options.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/query_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/queue_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/ray_tracing_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/wsi_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/parameter_validation.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/stateless/parameter_validation_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/object_tracker.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/object_tracker_utils.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/thread_safety.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/generated/vk_safe_struct.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/state_tracker/image_layout_map.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/layers/subresource_adapter.cpp
LOCAL_C_INCLUDES += $(VULKAN_INCLUDE) \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers/generated \
                    $(LOCAL_PATH)/$(THIRD_PARTY)/shaderc/third_party/spirv-tools/external/spirv-headers/include \
                    $(LOCAL_PATH)/$(THIRD_PARTY)/robin-hood-hashing/src/include
LOCAL_STATIC_LIBRARIES += layer_utils glslang SPIRV-Tools SPIRV-Tools-opt
LOCAL_CPPFLAGS += -std=c++17 -Wall -Werror -Wno-unused-function -Wno-unused-const-variable -frtti -fvisibility=hidden
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DVK_PROTOTYPES -DUSE_ROBIN_HOOD_HASHING -DXXH_NO_LONG_LONG
LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDFLAGS   += -Wl,-Bsymbolic
LOCAL_LDFLAGS   += -Wl,--exclude-libs,ALL
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := VkLayerValidationTests
LOCAL_SRC_FILES += $(SRC_DIR)/tests/layer_validation_tests.cpp \
                   $(SRC_DIR)/tests/vklayertests_instanceless.cpp \
                   $(SRC_DIR)/tests/vklayertests_pipeline_shader.cpp \
                   $(SRC_DIR)/tests/vklayertests_buffer_image_memory_sampler.cpp \
                   $(SRC_DIR)/tests/vklayertests_others.cpp \
                   $(SRC_DIR)/tests/vklayertests_query.cpp \
                   $(SRC_DIR)/tests/vklayertests_descriptor_renderpass_framebuffer.cpp \
                   $(SRC_DIR)/tests/vklayertests_command.cpp \
                   $(SRC_DIR)/tests/vklayertests_gpu.cpp \
                   $(SRC_DIR)/tests/vklayertests_debug_printf.cpp \
                   $(SRC_DIR)/tests/vklayertests_best_practices.cpp \
                   $(SRC_DIR)/tests/vklayertests_arm_best_practices.cpp \
                   $(SRC_DIR)/tests/vklayertests_wsi.cpp \
                   $(SRC_DIR)/tests/vklayertests_imageless_framebuffer.cpp \
                   $(SRC_DIR)/tests/vklayertests_graphics_library.cpp \
                   $(SRC_DIR)/tests/vklayertests_android_hardware_buffer.cpp \
                   $(SRC_DIR)/tests/vklayertests_ray_tracing.cpp \
                   $(SRC_DIR)/tests/vklayertests_ray_tracing_pipeline.cpp \
                   $(SRC_DIR)/tests/vklayertests_ray_tracing_gpu.cpp \
                   $(SRC_DIR)/tests/positive/command.cpp \
                   $(SRC_DIR)/tests/positive/descriptors.cpp \
                   $(SRC_DIR)/tests/positive/image_buffer.cpp \
                   $(SRC_DIR)/tests/positive/instance.cpp \
                   $(SRC_DIR)/tests/positive/other.cpp \
                   $(SRC_DIR)/tests/positive/pipeline.cpp \
                   $(SRC_DIR)/tests/positive/render_pass.cpp \
                   $(SRC_DIR)/tests/positive/shaderval.cpp \
                   $(SRC_DIR)/tests/positive/sync.cpp \
                   $(SRC_DIR)/tests/positive/tooling.cpp \
                   $(SRC_DIR)/tests/positive/android_hardware_buffer.cpp \
                   $(SRC_DIR)/tests/positive/ray_tracing.cpp \
                   $(SRC_DIR)/tests/positive/ray_tracing_pipeline.cpp \
                   $(SRC_DIR)/tests/vksyncvaltests.cpp \
                   $(SRC_DIR)/tests/vktestbinding.cpp \
                   $(SRC_DIR)/tests/vktestframeworkandroid.cpp \
                   $(SRC_DIR)/tests/vkerrormonitor.cpp \
                   $(SRC_DIR)/tests/vkrenderframework.cpp \
                   $(SRC_DIR)/tests/ray_tracing_objects.cpp \
                   $(SRC_DIR)/layers/convert_to_renderpass2.cpp \
                   $(SRC_DIR)/layers/generated/vk_safe_struct.cpp \
                   $(SRC_DIR)/layers/generated/lvt_function_pointers.cpp
LOCAL_C_INCLUDES += $(VULKAN_INCLUDE) \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers/generated \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers \
                    $(LOCAL_PATH)/$(SRC_DIR)/libs \
                    $(LOCAL_PATH)/$(THIRD_PARTY)/robin-hood-hashing/src/include

LOCAL_STATIC_LIBRARIES := googletest_main layer_utils shaderc
LOCAL_CPPFLAGS += -std=c++17 -DVK_PROTOTYPES -Wall -Werror -Wno-unused-function -Wno-unused-const-variable
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -DUSE_ROBIN_HOOD_HASHING -fvisibility=hidden
LOCAL_LDLIBS := -llog -landroid -ldl
LOCAL_LDFLAGS   += -Wl,-Bsymbolic
LOCAL_LDFLAGS   += -Wl,--exclude-libs,ALL
include $(BUILD_EXECUTABLE)

# Note: The following module is similar in name to the executable, but differs so that loader won't enumerate the resulting .so
include $(CLEAR_VARS)
LOCAL_MODULE := VulkanLayerValidationTests
LOCAL_SRC_FILES += $(SRC_DIR)/tests/layer_validation_tests.cpp \
                   $(SRC_DIR)/tests/vklayertests_instanceless.cpp \
                   $(SRC_DIR)/tests/vklayertests_pipeline_shader.cpp \
                   $(SRC_DIR)/tests/vklayertests_buffer_image_memory_sampler.cpp \
                   $(SRC_DIR)/tests/vklayertests_others.cpp \
                   $(SRC_DIR)/tests/vklayertests_query.cpp \
                   $(SRC_DIR)/tests/vklayertests_descriptor_renderpass_framebuffer.cpp \
                   $(SRC_DIR)/tests/vklayertests_command.cpp \
                   $(SRC_DIR)/tests/vklayertests_gpu.cpp \
                   $(SRC_DIR)/tests/vklayertests_debug_printf.cpp \
                   $(SRC_DIR)/tests/vklayertests_best_practices.cpp \
                   $(SRC_DIR)/tests/vklayertests_arm_best_practices.cpp \
                   $(SRC_DIR)/tests/vklayertests_wsi.cpp \
                   $(SRC_DIR)/tests/vklayertests_imageless_framebuffer.cpp \
                   $(SRC_DIR)/tests/vklayertests_graphics_library.cpp \
                   $(SRC_DIR)/tests/vklayertests_android_hardware_buffer.cpp \
                   $(SRC_DIR)/tests/vklayertests_ray_tracing.cpp \
                   $(SRC_DIR)/tests/vklayertests_ray_tracing_pipeline.cpp \
                   $(SRC_DIR)/tests/vklayertests_ray_tracing_gpu.cpp \
                   $(SRC_DIR)/tests/positive/command.cpp \
                   $(SRC_DIR)/tests/positive/descriptors.cpp \
                   $(SRC_DIR)/tests/positive/image_buffer.cpp \
                   $(SRC_DIR)/tests/positive/instance.cpp \
                   $(SRC_DIR)/tests/positive/other.cpp \
                   $(SRC_DIR)/tests/positive/pipeline.cpp \
                   $(SRC_DIR)/tests/positive/render_pass.cpp \
                   $(SRC_DIR)/tests/positive/shaderval.cpp \
                   $(SRC_DIR)/tests/positive/sync.cpp \
                   $(SRC_DIR)/tests/positive/tooling.cpp \
                   $(SRC_DIR)/tests/positive/android_hardware_buffer.cpp \
                   $(SRC_DIR)/tests/positive/ray_tracing.cpp \
                   $(SRC_DIR)/tests/positive/ray_tracing_pipeline.cpp \
                   $(SRC_DIR)/tests/vksyncvaltests.cpp \
                   $(SRC_DIR)/tests/vktestbinding.cpp \
                   $(SRC_DIR)/tests/vktestframeworkandroid.cpp \
                   $(SRC_DIR)/tests/vkerrormonitor.cpp \
                   $(SRC_DIR)/tests/vkrenderframework.cpp \
                   $(SRC_DIR)/tests/ray_tracing_objects.cpp \
                   $(SRC_DIR)/layers/convert_to_renderpass2.cpp \
                   $(SRC_DIR)/layers/generated/vk_safe_struct.cpp \
                   $(SRC_DIR)/layers/generated/lvt_function_pointers.cpp
LOCAL_C_INCLUDES += $(VULKAN_INCLUDE) \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers/generated \
                    $(LOCAL_PATH)/$(SRC_DIR)/layers \
                    $(LOCAL_PATH)/$(SRC_DIR)/libs \
                    $(LOCAL_PATH)/$(THIRD_PARTY)/robin-hood-hashing/src/include

LOCAL_STATIC_LIBRARIES := googletest_main layer_utils shaderc
LOCAL_CPPFLAGS += -std=c++17 -DVK_PROTOTYPES -Wall -Werror -Wno-unused-function -Wno-unused-const-variable
LOCAL_CPPFLAGS += -DVK_ENABLE_BETA_EXTENSIONS -DVK_USE_PLATFORM_ANDROID_KHR -fvisibility=hidden -DVALIDATION_APK -DUSE_ROBIN_HOOD_HASHING
LOCAL_WHOLE_STATIC_LIBRARIES += android_native_app_glue
LOCAL_LDLIBS := -llog -landroid -ldl
LOCAL_LDFLAGS := -u ANativeActivity_onCreate
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,third_party/googletest)
$(call import-module,third_party/shaderc)
