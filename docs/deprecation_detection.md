<!-- markdownlint-disable MD041 -->
<!-- Copyright 2025 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Deprecation Detection

The Vulkan Working Group plans to continue to grow the list of what is [deprecated functionality](https://docs.vulkan.org/spec/latest/appendices/deprecation.html#_deprecated_functionality). These are things that will **not** be removed from the API, but have a "newer" way to use that developers should try to use instead.

This all works by first marking things with a `<deprecate>` tags in the [vk.xml](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/xml/vk.xml). From here the Validation Layers can generated the [deprecation.cpp](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/layers/vulkan/generated/deprecation.cpp) file.

# What does it do

When the `Deprecation Detection` setting is enabled, it will report **warnings** when using deprecated functionality of the API in Vulkan.

- It will only report if the device supports the version/extensions that deprecated the functionality.
- It will only report the first usage of the functionality.
    - This is to prevent getting spammed with duplicate error messages
- These act like any other VUID, which can be muted.
    - They will always start with `WARNING-deprecation-*`

⚠️ If you have your own callback, make sure the `VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT` flag is not being ignored. If turning on warnings is a concern, we suggest turning off `Core Validation` and only have `Deprecation Detection` on.

## Enabling

The `Deprecation Detection` is just a [normal layer setting](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/docs/settings.md) that can be turned on.

The main 3 ways to turn on Sync

1. We **highly** suggest people to use [VkConfig](https://www.lunarg.com/introducing-the-new-vulkan-configurator-vkconfig/). There is a preset as well to only turn on `Deprecation Detection`.

2. Use `VK_EXT_layer_settings`

```c++
const VkBool32 verbose_value = true;
const VkLayerSettingEXT layer_setting = {"VK_LAYER_KHRONOS_validation", "deprecation_detection", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &verbose_value};
VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layer_setting};

VkInstanceCreateInfo instance_ci = GetYourCreateInfo();
instance_ci.pNext = &layer_settings_create_info;
```

3. Set as an environment variable (will turn on as an additional setting with core validation)

```bash
# Windows
set VK_LAYER_DEPRECATION_DETECTION=1

# Linux
export VK_LAYER_DEPRECATION_DETECTION=1

# Android
adb setprop debug.vulkan.khronos_validation.deprecation_detection=1
```

## Extra reference

There is a [Vulkan Guide chapter](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/deprecated.adoc) that will go more into depth how to replace older deprecated code.
