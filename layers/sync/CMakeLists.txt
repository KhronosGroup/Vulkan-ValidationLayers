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

add_library(vvl_sync STATIC)
target_sources(vvl_sync PRIVATE
    sync_access_context.cpp
    sync_access_context.h
    sync_access_flags.h
    sync_access_map.cpp
    sync_access_map.h
    sync_access_state.cpp
    sync_access_state.h
    sync_barrier.cpp
    sync_barrier.h
    sync_commandbuffer.cpp
    sync_commandbuffer.h
    sync_common.cpp
    sync_common.h
    sync_error_messages.cpp
    sync_error_messages.h
    sync_image.cpp
    sync_image.h
    sync_op.cpp
    sync_op.h
    sync_renderpass.cpp
    sync_renderpass.h
    sync_reporting.cpp
    sync_reporting.h
    sync_settings.h
    sync_stats.cpp
    sync_stats.h
    sync_submit.cpp
    sync_submit.h
    sync_validation.cpp
    sync_validation.h
)

target_link_libraries(vvl_sync PRIVATE
    SPIRV-Headers::SPIRV-Headers
    SPIRV-Tools-opt # TODO - break up shader_utils.h that needs spirv-tools
    VkLayer_utils # goes last for mimalloc
)
