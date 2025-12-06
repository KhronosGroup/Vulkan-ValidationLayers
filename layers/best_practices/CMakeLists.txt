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

add_library(vvl_best_practices STATIC)
target_sources(vvl_best_practices PRIVATE
    bp_buffer.cpp
    bp_cmd_buffer.cpp
    bp_cmd_buffer_nv.cpp
    bp_constants.h
    bp_copy_blit_resolve.cpp
    bp_descriptor.cpp
    bp_device_memory.cpp
    bp_drawdispatch.cpp
    bp_framebuffer.cpp
    bp_image.cpp
    bp_instance_device.cpp
    bp_pipeline.cpp
    bp_ray_tracing.cpp
    bp_render_pass.cpp
    bp_state_tracker.cpp
    bp_state.h
    bp_state.cpp
    bp_synchronization.cpp
    bp_utils.cpp
    bp_utils.h
    bp_video.cpp
    bp_wsi.cpp
    best_practices_validation.h
    ../${API_TYPE}/generated/best_practices.cpp
    ../${API_TYPE}/generated/best_practices_device_methods.h
    ../${API_TYPE}/generated/best_practices_instance_methods.h
)

target_link_libraries(vvl_best_practices PRIVATE
    SPIRV-Headers::SPIRV-Headers
    SPIRV-Tools-opt # TODO - break up shader_utils.h that needs spirv-tools
    VkLayer_utils # goes last for mimalloc
)
