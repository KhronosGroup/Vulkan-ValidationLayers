# ~~~
# Copyright (c) 2022 LunarG, Inc.
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
find_package(PythonInterp 3 QUIET)

if(PYTHONINTERP_FOUND)
    if (NOT EXISTS "${VulkanRegistry_DIR}/vk.xml")
        message(FATAL_ERROR "Unable to find vk.xml")
    endif()

    set(spirv_unified_include_dir "${SPIRV_HEADERS_INSTALL_DIR}/include/spirv/unified1/")
    if (NOT IS_DIRECTORY ${spirv_unified_include_dir})
        message(FATAL_ERROR "Unable to find spirv/unified1")
    endif()

    set(generate_source_py "${PROJECT_SOURCE_DIR}/scripts/generate_source.py")
    if (NOT EXISTS ${generate_source_py})
        message(FATAL_ERROR "Unable to find generate_source.py!")
    endif()

    add_custom_target(VulkanVL_generated_source
        COMMAND ${PYTHON_EXECUTABLE} ${generate_source_py} ${VulkanRegistry_DIR} ${spirv_unified_include_dir} --incremental
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/layers/generated
    )

    set_target_properties(VulkanVL_generated_source PROPERTIES FOLDER ${LAYERS_HELPER_FOLDER})
else()
    message("WARNING: VulkanVL_generated_source target requires python 3")
endif()
