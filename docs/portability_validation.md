<!-- markdownlint-disable MD041 -->
<!-- Copyright 2020 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# VK_KHR_portability_subset Validation

## Requirements

Either
- The [VK_LAYER_LUNARG_device_simulation](https://vulkan.lunarg.com/doc/sdk/1.2.154.1/windows/device_simulation_layer.html) layer with portability enabled _or_
- A driver that supports `VK_KHR_portability_subset`.

## Running the tests
The following is an example setup for running the portability tests on Windows with devsim:
```powershell
# Replace ';' with ':' when running on *nix
$env:VK_LAYER_PATH="/path/to/VulkanTools/build/layersvt/Debug;/path/to/Vulkan-ValidationLayers/build/layers/Debug"

# Make sure the devsim layer is loaded.
# Again be sure to replace ';' with ':' when running on *nix.
$env:VK_INSTANCE_LAYERS="VK_LAYER_KHRONOS_validation;VK_LAYER_LUNARG_device_simulation"

# This environment variable must be set to ensure portability is enabled as there is currently no programatic way to enable portability in the devsim layer
$env:VK_DEVSIM_EMULATE_PORTABILITY_SUBSET_EXTENSION="1"

/path/to/Vulkan-ValidationLayers/build/tests/Debug/vk_layer_validation_tests.exe --gtest_filter=VkPortability*
```