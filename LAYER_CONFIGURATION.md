<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2019,2022 LunarG, Inc. -->

[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Layers Overview and Configuration

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/

Vulkan supports intercepting or hooking API entry points via a layer framework.  A layer can intercept all or any subset of Vulkan API entry points.  Multiple layers can be chained together to cascade their functionality in the appearance of a single, larger layer.

Vulkan validation and utility layers give Vulkan application developers the ability to add additional functionality to applications without modifying the application itself, e.g., dumping API entry points or generating screenshots of specified frames.

## Configuring Vulkan Layers using *Vulkan Configurator*

Developers can configure layers through a graphical user interface. *[Vulkan Configurator](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html)* allows full user control of Vulkan layers, including enabling or disabling specific layers, controlling layer order, changing layer settings, etc.

## Enabling Vulkan Layers using `VkCreateInstance()`
Applications may programmatically activate layers via the `vkCreateInstance()` entry point. This
is done by setting `enabledLayerCount` and `ppEnabledLayerNames` in the `VkInstanceCreateInfo`
structure.

## Enabling Vulkan Layers using Environment Variables

### Windows
Layers can be activated by using the `VK_INSTANCE_LAYERS` environment variable.
The variable should include a semicolon-separated list of layer names to activate.
Note that order is relevant, with the initial layer being the closest to the application, and the final layer being closest to the driver.

For example:

```
C:\> set VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation
```
In this example, the api_dump layer will be called _before_ the Khronos validation layer.

`VK_INSTANCE_LAYERS` may also be set in the system environment variables.

To activate layers located in a particular SDK installation, or layers built locally from source, specify the layer JSON manifest file directory using the `VK_LAYER_PATH` environment variable.
For example, if a Vulkan SDK is installed in `C:\VulkanSDK\1.2.198.0`, execute the following in a Command Window:

```
C:\> set VK_LAYER_PATH=C:\VulkanSDK\1.2.198.0\Bin
C:\> set VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation
C:\> vkcube
```

These commands will run vkcube, enabling the api_dump and validation layers that are installed in `C:\VulkanSDK\1.2.198.0\Bin`.

### Linux and macOS
Layers can be activated by using the `VK_INSTANCE_LAYERS` environment variable.
The variable should include a colon-separated list of layer names to activate.
Note that order is relevant, with the initial layer being the closest to the application, and the final layer being closest to the driver.

For example:

```
$ export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump:VK_LAYER_KHRONOS_validation
```
In this example, the api_dump layer will be called _before_ the Khronos validation layer.

To activate layers located in a particular SDK installation, or layers built locally from source, specify the layer JSON manifest file directory using the `VK_LAYER_PATH` environment variable.

For Linux, if Vulkan SDK 1.2.198.0 was locally installed in `/sdk` and `VULKAN_SDK=/sdk/1.2.198.0/x86_64`:

```
$ export VK_LAYER_PATH=$VULKAN_SDK/lib/vulkan/layers
$ export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$VULKAN_SDK/lib/vulkan/layers
$ export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump:VK_LAYER_KHRONOS_validation
$ ./vkcube
```

These commands will run vkcube, enabling the api_dump and validation layers that are installed in `/sdk/1.2.198.0/Bin`.

For macOS, if Vulkan SDK 1.2.198.0 was locally installed in `/sdk` and `VULKAN_SDK=/sdk/1.2.198/macOS`:

```
$ export VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layers.d
$ export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib
$ export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump:VK_LAYER_KHRONOS_validation
$ cd $VULKAN_SDK/../Applications/
$ open vkcube.app
```
These commands will run vkcube, enabling the api_dump and validation layers that are installed in `/sdk/1.2.198.0/Bin`.

## Layer Settings File
Options specific to each layer can be set via the `vk_layer_settings.txt` settings file.
By default, the settings file must be named `vk_layer_settings.txt` and reside in the working directory of the targeted application.
If `VK_LAYER_SETTINGS_PATH` is set and is a directory, then the settings file must be a file called `vk_layer_settings.txt` in the directory given by `VK_LAYER_SETTINGS_PATH`.
If `VK_LAYER_SETTINGS_PATH` is set and is not a directory, then it must point to a file (with any name) which is the layer settings file.

The settings file consists of comment lines and settings lines.  Comment lines begin with the `#` character.  Settings lines have the following format:

   `<`*`LayerName`*`>.<`*`setting_name`*`> = <`*`setting_value`*`>`

The settings and values available for each layer are listed in the documentation for each supported layer.

## Layer Environment Variables

Some settings from the settings file can also be set using environment variables. The settings that can be set using environment variables are 
listed in the documentation for each supported layer.  If an environment variable is set, its value takes precedence over the value in the settings file.
