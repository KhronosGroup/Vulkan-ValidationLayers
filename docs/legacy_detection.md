<!-- markdownlint-disable MD041 -->
<!-- Copyright 2025-2026 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Legacy Detection

The Vulkan Working Group plans to continue to grow the list of what is [legacy functionality](https://docs.vulkan.org/spec/latest/appendices/legacy.html#_legacy_functionality). These are things that will **not** be removed from the API, but have a "newer" way to use that developers should try to use instead.

This all works by first marking things with a `<deprecate>` tags in the [vk.xml](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/xml/vk.xml). From here the Validation Layers can generated the [legacy.cpp](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/layers/vulkan/generated/legacy.cpp) file.

> The <deprecate> was before the working group decided to give the name "Legacy" instead to prevent confusion.

# What does it do

When the `Legacy Detection` setting is enabled, it will report **warnings** when using superseded functionality of the API in Vulkan.

- It will only report if the developer has **explicitly enabled** the version/extensions that superseded the functionality.
- It will only report the first usage of the functionality.
    - This is to prevent getting spammed with duplicate error messages
- These act like any other VUID, which can be muted.
    - They will always start with `WARNING-legacy-*`

⚠️ If you have your own callback, make sure the `VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT` flag is not being ignored. If turning on warnings is a concern, we suggest turning off `Core Validation` and only have `Legacy Detection` on.

## Enabling

The `Legacy Detection` is just a [normal layer setting](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/docs/settings.md) that can be turned on.

The main 3 ways to turn on `Legacy Detection`

1. We **highly** suggest people to use [VkConfig](https://www.lunarg.com/introducing-the-new-vulkan-configurator-vkconfig/). There is a preset as well to only turn on `Legacy Detection`.

2. Use `VK_EXT_layer_settings`

```c++
const VkBool32 verbose_value = true;
const VkLayerSettingEXT layer_setting = {"VK_LAYER_KHRONOS_validation", "legacy_detection", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &verbose_value};
VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layer_setting};

VkInstanceCreateInfo instance_ci = GetYourCreateInfo();
instance_ci.pNext = &layer_settings_create_info;
```

3. Set as an environment variable (will turn on as an additional setting with core validation)

```bash
# Windows
set VK_LAYER_LEGACY_DETECTION=1

# Linux
export VK_LAYER_LEGACY_DETECTION=1

# Android
adb shell setprop debug.vulkan.khronos_validation.legacy_detection=1
```

## Why is it reporting extensions my device doesn't even support?

By default, if the user is using `VK_EXT_foo` and there is a `VK_KHR_foo` in the current headers VVL was built with, it will warn the user. Some people will not want this warning unless their local device actually supports `VK_KHR_foo`.

For this case, there is an additional `VK_LAYER_LEGACY_DETECTION_ONLY_SUPPORTED` setting, that when set, will only give a warning if the `VkInstance`/`VkDevice` actually supports `VK_KHR_foo`.

There is even a `VK_LAYER_LEGACY_DETECTION_ONLY_ENABLED` setting, that goes further and will only give a warning you have enabled `VK_KHR_foo`, but still using `VK_EXT_foo`.

The argument for the default being to still report `VK_KHR_foo` is that an application developer can always opt out of warnings for things they know they want to ignore, but opting in to warnings requires them to already know those things exist, which would mean they didn't need the extension to tell them about them in the first place.

## Can I mute warnings?

Yes, if the user wants to mute a certain error they can simply go `VK_LAYER_MESSAGE_ID_FILTER=WARNING-legacy-gpdp2`

## Should I keep this on all the time?

The layer is very simple and likely will cause very little overhead and you likely **could**.

**Should** you... likely no.

The goal of this Legacy Detection was to provide a tool so people could learn about newer Vulkan functionality. Just turning it on once and awhile and seeing what it reports might be enough for the majority of people.

## Extra reference

There is a [Vulkan Guide chapter](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/deprecated.adoc) that will go more into depth how to replace older superseded code.
