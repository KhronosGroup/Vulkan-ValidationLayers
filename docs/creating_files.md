# Creating files

## License

Every new source file should include this license at the top:

C++:
```cpp
/* Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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
 */
```

Python:
```python
# Copyright (c) 2023 Valve Corporation
# Copyright (c) 2023 LunarG, Inc.
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
```

## Adding a source file in the validation layer

### Adding a file in `layers/` (and not in `layer/utils`)

To add for instance `layers/core_checks/cc_my_file.h/cpp`, you need to update the following files:

- `layers/CMakeLists.txt`
```
target_sources(vvl PRIVATE
	...
	layers/core_checks/cc_my_file.cpp
	layers/core_checks/cc_my_file.h
	...
)
```

- `BUILD.gn`
```
core_validation_sources = [
  ...
  "layers/core_checks/cc_my_file.cpp",
  "layers/core_checks/cc_my_file.h",
  ...
```

- `build-android/jni/Android.mk`
```
LOCAL_MODULE := VkLayer_khronos_validation
...
LOCAL_SRC_FILES += $(SRC_DIR)/layers/core_checks/cc_my_file.cpp
...
```

### Adding a file in `layers/utils`

To add for instance `layers/utils/my_file.h/cpp`, you need to update the following files:

- `BUILD.gn`
```
source_set("vulkan_layer_utils") {
  include_dirs = [
    "layers",
    "layers/external",
    "layers/vulkan",
    "layers/my_include_dir" <-- You need to make sure this is listed if your new file includes a file located in "layers/my_include_dir"
  ]
  sources = [
    ...
    "layers/utils/my_file.cpp",
    "layers/utils/my_file.h",
    ...
  ]
```

- `build-android/jni/Android.mk`
```
LOCAL_MODULE := layer_utils
...
LOCAL_SRC_FILES += $(SRC_DIR)/layers/utils/my_file.cpp
...
```

- `layers/CMakeLists.txt`
```
target_sources(VkLayer_utils PRIVATE
  ...
  utils/my_file.cpp
  utils/my_file.h
  ...
)

...

if(BUILD_LAYER_SUPPORT_FILES)
    ...
    install(FILES ...
                  ${CMAKE_SOURCE_DIR}/layers/utils/my_file.cpp
                  ${CMAKE_SOURCE_DIR}/layers/utils/my_file.h
                  ...
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vulkan/utils)
    ...        
endif()
```

If your file is used in the test project, add this:

- `tests/layers/CMakeLists.txt`
```
target_sources(VkLayer_device_profile_api PRIVATE
    ...
    ${VVL_SOURCE_DIR}/layers/utils/my_file.cpp
    ...
)
```

## Adding a test file

To add for instance `tests/my_test.cpp`, you need to update the following files:

- `tests/CMakeLists.txt`
```
target_sources(vk_layer_validation_tests PRIVATE
    ...
    my_test.cpp
    ...
)
```

- `build-android/jni/Android.mk`
```
LOCAL_MODULE := VulkanLayerValidationTests
LOCAL_SRC_FILES += $(SRC_DIR)/tests/framework/layer_validation_tests.cpp /
                  ...
                  $(SRC_DIR)/tests/my_test.cpp /
                  ...
```

## Adding a python script

Add it in `scripts/`

## Adding a generated source file

TODO
