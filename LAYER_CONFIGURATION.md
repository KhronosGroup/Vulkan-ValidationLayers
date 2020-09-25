<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2019 LunarG, Inc. -->

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

## Configuring Vulkan Layers using Environment Variables

Application validation in Vulkan is implemented solely by the [Khronos validation layer](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/docs/khronos_validation_layer.md),
 **`VK_LAYER_KHRONOS_validation`**.

Initially, validation was implemented as a dozen or so individual layers with limited scope -- i.e., `mem_tracker`, `object_tracker`, `device_limits`,
`unique_objects`, etc., and a standard validation meta-layer, `VK_LAYER_LUNARG_standard_validation`, was supplied to enforce a particular layer order.
In order to simplify configuration, improve build times, and standardize layer interfaces, validation was
further consolidated into the single, configurable Khronos validation layer, while the individual component layers and meta-layer have been
deprecated. For additional detail, see the [Unified Validation Layer whitepaper](https://www.lunarg.com/wp-content/uploads/2019/04/UberLayer_V3.pdf).

**Validation Layer History**

| Layer Name                                  | Introduced          |    Status        |
|---------------------------------------------|---------------------|------------------|
|   **VK_LAYER_KHRONOS_validation**           | **March 2019**      |  **Supported**   |
|   VK_LAYER_LUNARG_standard_validation       | February 2016       |  Deprecated      |
|   VK_LAYER_LUNARG_core_validation           | March 2016          |  Deprecated      |
|   VK_LAYER_LUNARG_object_tracker            | October 2014        |  Deprecated      |
|   VK_LAYER_GOOGLE_threading                 | April 2015          |  Deprecated      |
|   VK_LAYER_GOOGLE_unique_objects            | December 2015       |  Deprecated      |
|   VK_LAYER_LUNARG_parameter_validation      | December 2014       |  Deprecated      |
|   VK_LAYER_LUNARG_mem_tracker               | November 2014       |  Deprecated      |
|   VK_LAYER_LUNARG_draw_state                | October 2014        |  Deprecated      |
|   VK_LAYER_LUNARG_swapchain                 | September 2015      |  Deprecated      |
|   VK_LAYER_LUNARG_device_limits             | September 2015      |  Deprecated      |
|   VK_LAYER_LUNARG_shader_checker            | April 2015          |  Deprecated      |
|   VK_LAYER_LUNARG_image                     | June 2015           |  Deprecated      |

### Activating Layers on Windows
Before or during execution of a Vulkan application, the loader must be informed of the layers to activate.
This can be done in two ways: programmatically, or by using environment variables.

Applications may programmatically activate layers via the `vkCreateInstance()` entry point.

Layers may also be activated by using the `VK_INSTANCE_LAYERS` environment variable.
The variable should include a semi-colon separated list of layer names to activate.
Note that order is relevant, with the initial layer being the closest to the application, and the final layer being closest to the driver.

The list of layers to activate can be specified by executing the following in a Command Window:

```
C:\> set VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation
```
In this example, the api_dump layer will be called _before_ the Khronos validation layer.

`VK_INSTANCE_LAYERS` may also be set in the system environment variables.

To activate layers located in a particular SDK installation, or layers built locally from source, specify the layer JSON manifest file directory using the `VK_LAYER_PATH` environment variable.
For example, if a Vulkan SDK is installed in `C:\VulkanSDK\1.1.121`, execute the following in a Command Window:

```
C:\> set VK_LAYER_PATH=C:\VulkanSDK\1.1.121\Bin
C:\> set VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation
C:\> vkcube
```

### Activating Layers on Linux and macOS
Before or during execution of a Vulkan application, the loader must be informed of the layers to activate.
This can be done in two ways:  programmatically, or by using environment variables.

Applications may programmatically activate layers via the `vkCreateInstance()` entry point.

Layers may also be activated by using the `VK_INSTANCE_LAYERS` environment variable.
The variable should include a colon-separated list of layer names to activate.
Note that order is relevant, with the initial layer being the closest to the application, and the final layer being closest to the driver.

For example, the list of explicit layers to activate can be specified with:

```
$ export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump:VK_LAYER_KHRONOS_validation
```

To activate layers in a particular SDK installation, or layers built locally from source, identify certain library paths and the layer JSON manifest file directory in addition to the layers to activate.  

For Linux, if the Vulkan SDK was locally installed to `/sdks`, `VULKAN_SDK=/sdks/VulkanSDK/1.1.121/x86_64`:

```
$ export VK_LAYER_PATH=$VULKAN_SDK/lib/vulkan/layers
$ export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$VULKAN_SDK/lib/vulkan/layers
$ export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump:VK_LAYER_KHRONOS_validation
$ ./vkcube
```

For macOS, if the Vulkan SDK was locally installed to `/sdks`, `VULKAN_SDK=/sdks/VulkanSDK/1.1.121/macOS`:

```
$ export VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layers.d
$ export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib
$ export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_api_dump:VK_LAYER_KHRONOS_validation
$ cd $VULKAN_SDK/../Applications/
$ open vkcube.app
```

### Layer Settings File
In addition to activating the layers, available layer options can be set via the `vk_layer_settings.txt` settings file.
By default, the settings file must be named `vk_layer_settings.txt` and reside in the working directory of the targeted application.
If `VK_LAYER_SETTINGS_PATH` is set and is a directory, then the settings file must be a file called `vk_layer_settings.txt` in the directory given by `VK_LAYER_SETTINGS_PATH`.
If `VK_LAYER_SETTINGS_PATH` is set and is not a directory, then it must point to a file (with any name) which is the layer settings file.

Note:  To control layer reporting output, a layer settings file must be provided that identifies specific reporting levels for the layers enabled via the `VK_INSTANCE_LAYERS` environment variable.

The settings file consists of comment lines and settings lines.  Comment lines begin with the `#` character.  Settings lines have the following format:

   `<`*`LayerName`*`>.<`*`setting_name`*`> = <`*`setting_value`*`>`

| Setting                  | Values                       | Description                                                      |
| ------------------------ | ---------------------------- | ----------------------------------------------------------------- |
| *`LayerName`*`.report_flags` | `info`      | Report information level messages                               |
|                            | `warn`      | Report warning level messages                 |
|                            | `perf`      | Report performance level warning messages                        |
|                            | `error`     | Report error level messages                                 |
|                            | `debug`      | Reserved                                                |
| *`LayerName`*`.debug_action` | `VK_DBG_LAYER_ACTION_IGNORE`   | Ignore message reporting                                          |
|                            | `VK_DBG_LAYER_ACTION_LOG_MSG`  | Report messages to log                                            |
|                            | `VK_DBG_LAYER_ACTION_DEBUG_OUTPUT`    | (Windows) Report messages to debug console of Microsoft Visual Studio
|                            | `VK_DBG_LAYER_ACTION_BREAK`    | Break on messages (not currently used)                                  |
| *`LayerName`*`.log_filename` | *`filename`*`.txt`             | Name of file to log `report_flags` level messages; default is `stdout` |
| *`LayerName`*`.enables` | comma separated list of `VkValidationFeatureEnableEXT` enum values as defined in the Vulkan Specification      | Enables the specified validation features         |
| *`LayerName`*`.disables` | comma separated list of `VkValidationFeatureDisableEXT` enum values as defined in the Vulkan Specification      | Disables the specified validation features         |



Sample layer settings file contents:

```
khronos_validation.report_flags = info,error
khronos_validation.debug_action = VK_DBG_LAYER_ACTION_LOG_MSG
khronos_validation.disable = VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT 
# VK_LAYER_LUNARG_api_dump custom settings
lunarg_api_dump.no_addr = TRUE
lunarg_api_dump.file = FALSE
```
In the Vulkan-ValidationLayers repository, a sample layer settings file can be found in the 'layers' directory, [here](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/layers/vk_layer_settings.txt).
In the Vulkan SDK, this Linux version of this file is located in `config/vk_layer_settings.txt` of your local Vulkan SDK install.
On Windows, you can find a sample layer settings file in `Config\vk_layer_settings.txt` of your local Vulkan SDK install.
Consult these sample layer settings files for additional information and detail related to available options and settings.

Note: If layers are activated via `VK_INSTANCE_LAYERS` environment variable and if neither an application-defined callback is defined nor a layer settings file is present, the loader/layers will provide default callbacks enabling output of error-level messages to standard out (and via `OutputDebugString` on Windows).

### Advanced Layer Configuration, Installation, and Discovery Details
The Vulkan loader searches specific platform-specific locations to find installed layers.
For additional details, see the `LoaderAndLayerInterface.md` available [here](https://vulkan.lunarg.com/doc/sdk/latest/windows/loader_and_layer_interface.html),
or in the Vulkan-Loader repository, [here](https://github.com/KhronosGroup/Vulkan-Loader/blob/master/loader/LoaderAndLayerInterface.md).

Setting the `VK_LAYER_PATH` environment variable overrides the default loader layer search mechanism.
When set, the loader will search only the directory(s) identified by the `VK_LAYER_PATH` environment variable for layer manifest files.

Applications can query available layers via the `vkEnumerateInstanceLayerProperties()` command.
