# ~~~
# Copyright (c) 2025 Valve Corporation
# Copyright (c) 2025 LunarG, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

add_library(vvl_stateless STATIC)
target_sources(vvl_stateless PRIVATE
    sl_buffer.cpp
    sl_cmd_buffer_dynamic.cpp
    sl_cmd_buffer.cpp
    sl_descriptor.cpp
    sl_device_generated_commands.cpp
    sl_device_memory.cpp
    sl_external_object.cpp
    sl_framebuffer.cpp
    sl_image.cpp
    sl_instance_device.cpp
    sl_pipeline.cpp
    sl_ray_tracing.cpp
    sl_render_pass.cpp
    sl_shader_object.cpp
    sl_spirv.cpp
    sl_spirv.h
    sl_synchronization.cpp
    sl_tensor.cpp
    sl_utils.cpp
    sl_vuid_maps.cpp
    sl_vuid_maps.h
    sl_wsi.cpp
    stateless_validation.h
    ../${API_TYPE}/generated/stateless_device_methods.h
    ../${API_TYPE}/generated/stateless_instance_methods.h
    ../${API_TYPE}/generated/stateless_validation_helper.cpp
)

target_link_libraries(vvl_stateless PRIVATE
    SPIRV-Headers::SPIRV-Headers
    VkLayer_utils # goes last for mimalloc
)
