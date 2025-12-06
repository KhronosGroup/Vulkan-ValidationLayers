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

add_library(vvl_thread_tracker STATIC)
target_sources(vvl_thread_tracker PRIVATE
    thread_safety_validation.cpp
    thread_safety_validation.h
    ../${API_TYPE}/generated/thread_safety.cpp
    ../${API_TYPE}/generated/thread_safety_device_defs.h
    ../${API_TYPE}/generated/thread_safety_instance_defs.h
)

target_link_libraries(vvl_thread_tracker PRIVATE
    VkLayer_utils # goes last for mimalloc
)
