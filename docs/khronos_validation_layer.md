<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2025 LunarG, Inc. -->

[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# VK\_LAYER\_KHRONOS\_validation

Vulkan is an Explicit API, enabling direct control over how GPUs actually work. By design, minimal error
checking is done inside a Vulkan driver - applications have full control and responsibility for correct operation.
Any errors in Vulkan usage can result in unexpected behavior or even a crash.  The `VK_LAYER_KHRONOS_validation` layer
can be used to to assist developers in isolating incorrect usage, and in verifying that applications
correctly use the API.

## Configuring the Validation Layer

There are 4 ways to configure the settings: `vkconfig`, `application defined`, `vk_layer_settings.txt`, `environment variables`

## VkConfig

We suggest people to use [VkConfig](https://www.lunarg.com/introducing-the-new-vulkan-configurator-vkconfig/).

The GUI comes with the SDK, and takes the `VkLayer_khronos_validation.json` file and does **everything** for you!

## Application Defined

The application can now use the `VK_EXT_layer_settings` extension to do everything at `vkCreateInstance` time. (Don't worry, we implement the extension, so it will be supported 100% of the time!).

```c++
// Example how to turn on verbose mode for DebugPrintf
const VkBool32 verbose_value = true;
const VkLayerSettingEXT layer_setting = {"VK_LAYER_KHRONOS_validation", "printf_verbose", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &verbose_value};
VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layer_setting};

VkInstanceCreateInfo instance_ci = GetYourCreateInfo();
instance_ci.pNext = &layer_settings_create_info;
```

## vk_layer_settings.txt

There is info [elsewhere](https://vulkan.lunarg.com/doc/view/latest/windows/layer_configuration.html) to describe this file, but the short answer is to set the `VK_LAYER_SETTINGS_PATH` like the following:

```bash
# windows
set VK_LAYER_SETTINGS_PATH=C:\path\to\vk_layer_settings.txt

# linux
export VK_LAYER_SETTINGS_PATH=/path/to/vk_layer_settings.txt
```

and it will set things for you in that file. We have a [default example](../layers/vk_layer_settings.txt) file you can start with.

## Environment Variables

This is done for us via the `vkuCreateLayerSettingSet` call in the [Vulkan-Utility-Libraries](https://github.com/KhronosGroup/Vulkan-Utility-Libraries/).

As an example, in our `VkLayer_khronos_validation.json` file you will find something like `"key": "message_id_filter",`.

From here you just need to adjust it the naming and prefix depending on your platform:

```bash
# Windows
set VK_LAYER_MESSAGE_ID_FILTER=VUID-VkInstanceCreateInfo-pNext-pNext

# Linux
export VK_LAYER_MESSAGE_ID_FILTER=VUID-VkInstanceCreateInfo-pNext-pNext

# Android
adb setprop debug.vulkan.khronos_validation.message_id_filter=VUID-VkInstanceCreateInfo-pNext-pNext
```

## Layer Options

> We suggest using `VkConfig` to discover the options, but the following is generated per SDK version

The options for this layer are specified in VkLayer_khronos_validation.json. The option details are in [khronos_validation_layer.html](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html).
