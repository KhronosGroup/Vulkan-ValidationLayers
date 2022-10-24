<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2022 LunarG, Inc. -->

[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# VK\_LAYER\_KHRONOS\_validation

Vulkan is an Explicit API, enabling direct control over how GPUs actually work. By design, minimal error
checking is done inside a Vulkan driver - applications have full control and responsibility for correct operation.
Any errors in Vulkan usage can result in unexpected behavior or even a crash.  The `VK_LAYER_KHRONOS_validation` layer
can be used to to assist developers in isolating incorrect usage, and in verifying that applications
correctly use the API.

**Note:**

* Most *Khronos Validation layer* features can be used simultaneously. However, this could result in noticeable performance degradation. The best practice is to run *Core validation*, *GPU-Assisted validation*, *Synchronization Validation* and *Best practices validation* features individually.

* *Debug Printf functionality* and *GPU-Assisted validation* cannot be run at the same time.

## Configuring the Validation Layer

For an overview of how to configure layers, refer to the [Layers Overview and Configuration](https://vulkan.lunarg.com/doc/sdk/latest/windows/layer_configuration.html) document.

The Validation Layer settings are documented in detail in the
[VK_LAYER_KHRONOS_validation](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html#user-content-layer-details) document.

The Validation Layer can also be enabled and configured using vkconfig. See the [vkconfig](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) documentation for more information.


## Layer Controls
Layer behavior is controlled through either a layer settings file or an extension.
The layer settings file allows a user to control various layer features and behaviors by providing easily modifiable settings.
The Vulkan Validation Features extension provides layer controls, while the Vulkan Debug Utils extension provides methods of
accessing and controlling feedback from the layers:

| Extension                 | Description                       |
| ------------------------ | ---------------------------- |
|  [VK_EXT_validation_features](#validationfeatures)  | allows application control of various layer features      |
|  [VK_EXT_debug_utils](#debugutils)  | allows application control and capture of debug reporting information   |


See the [Layers Overview and Configuration](../LAYER_CONFIGURATION.md) document for more detailed information on configuring Vulkan layers.

### <a name="validationfeatures"></a>VK\_EXT\_validation\_features
The preferred method for an application to programmatically control validation layer features is through the `VK_EXT_validation_features` extension.
Using this extension allows an application to enable or disable specific Khronos validation features.
Note that this extension provides low-level control to an application, and that some combinations of enable/disable settings may produce undefined behavior.

The `VK_EXT_validation_features` flags can be used to disable validation corresponding to the following deprecated layers:

| Setting this `VK_EXT_validation_features` disable flag | Corresponds to not loading this deprecated layer |
| -------------------------------------------------------|--------------------------------------------------|
| `VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT` | `VK_LAYER_GOOGLE_threading` |
| `VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT` | `VK_LAYER_LUNARG_parameter_validation` |
| `VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT` | `VK_LAYER_LUNARG_object_tracker` |
| `VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT` | `VK_LAYER_LUNARG_core_validation` |
| `VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT` | `VK_LAYER_GOOGLE_unique_objects` |

Refer to [VK_EXT_validation_features](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VK_EXT_validation_features)
in the Vulkan specification for details on this extension.

### <a name="debugutils"></a>VK\_EXT\_debug\_utils
The preferred method for an app to control layer logging is via the `VK_EXT_debug_utils` extension.
Using the `VK_EXT_debug_utils` extension allows an application to register multiple messengers with the layers.
Each messenger can trigger a message callback when a log message occurs.
Some messenger callbacks may log the information to a file, others may cause a debug break point or other-application defined behavior.
An application can create a messenger even when no layers are enabled, but they will only be called for loader and, if implemented, driver events.
Each message is identified by both a severity level and a message type.
Severity levels indicate the severity of the message that should be logged including: error, warning, etc.
Message types indicate the specific type of message including: validation, performance, etc.
Some layers return a unique message ID string per message as well.
Using the severity, type, and message ID, an application can easily filter the messages received by their messenger callback.

When reporting an error, the KHRONOS validation layer returns relevant specification information and a link to that information
in the official Vulkan specification. Layers included in a Vulkan SDK will link to a version of the Vulkan specification
annotated with valid usage identifiers.

#### Message Types As Reported By VK\_EXT\_debug\_utils flags:

| Type     |    Debug Utils Severity          |    Debug Utils Type          |
| ---------|----------------------------------|------------------------------|
| Error | `VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT` | `VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT` |
| Warn | `VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT` | `VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT` |
| Perf Warn | `VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT` | `VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT` |
| Info | `VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT` | `VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT` or `VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT` |

By default, if an app uses the VK_EXT_debug_utils extension and registers a messenger, the validation layer default messenger log callback will not
execute, as it is considered to be handled by the app. However, using the validation layer callback can be very useful, as it provides a unified log
that can be easily parsed. On Android, the validation layer default callback can be forced to always execute, and log its contents to logcat, using
the following system property:

```bash
adb shell setprop debug.vvl.forcelayerlog 1
```

The debug.vvl namespace signifies validation layers, and setting this property forces the validation layer callback to always execute, even if the app registers
a messenger callback itself. This is especially useful for automation tasks, ensuring that errors can be read in a parseable format.

Refer to [VK_EXT_debug_utils](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VK_EXT_debug_utils)
in the Vulkan Specification for details on this feature.

## Layer Options

The options for this layer are specified in VkLayer_khronos_validation.json. The option details are in [khronos_validation_layer.html](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html).



