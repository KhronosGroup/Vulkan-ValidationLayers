<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2020 LunarG, Inc. -->

[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# VK\_LAYER\_KHRONOS\_validation

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/
The `VK_LAYER_KHRONOS_validation` layer supports the following validation areas:

- Thread safety validation 
- Stateless parameter validation 
- Object lifetime validation 
- Core validation checks
- GPU-Assisted validation
- Best practices validation
- Debug Printf functionality
- Handle wrapping functionality

**See the** [Layers Overview and Configuration](./layer_configuration.md) **document for more information on how to configure Vulkan layers.**

### <a name="validationfeatures"></a>VK\_EXT\_validation\_features
The preferred method for an application to programmatically control validation layer features is through the `VK_EXT_validation_features` extension.
Using `VK_EXT_validation_features` extension allows an application to enable or disable specific Khronos validation features.
Note that this extension provides low-level control to an application, and that some combinations of enable/disable settings may produce undefined behavior.

The `VK_EXT_validation_features` flags can be used to disable validation corresponding to the following deprecated layers:

| Setting this `VK_EXT_validation_features` disable flag | Corresponds to not loading this deprecated layer |
| -------------------------------------------------------|--------------------------------------------------|
| `VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT` | `VK_LAYER_GOOGLE_threading` |
| `VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT` | `VK_LAYER_LUNARG_parameter_validation` |
| `VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT` | `VK_LAYER_LUNARG_object_tracker` |
| `VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT` | `VK_LAYER_LUNARG_core_validation` |
| `VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT` | `VK_LAYER_GOOGLE_unqiue_objects` |

**Refer to the Validation Features extension section of the Vulkan specification for details.

## Layer Controls
Most layers support one or both of the available methods for controlling layer behavior: through a layer settings file or an extension.
The layer settings file allows a user to control various layer features and behaviors by providing easily modifiable settings.
Various Vulkan extensions also provide layer controls:

| Extension                 | Description                       | 
| ------------------------ | ---------------------------- | 
|  [VK_EXT_debug_utils](#debugutils)  | allows applications control and capture of expanded debug reporting information   | 
|  [VK_EXT_validation_features](#validationfeatures)  | allows applications expanded control of various layer features      |  

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

#### Message Types As Reported By VK\_EXT\_debug\_utils flags:

| Type     |    Debug Utils Severity          |    Debug Utils Type          |
| ---------|----------------------------------|------------------------------|
| Error | `VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT` | `VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT` |
| Warn | `VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT` | `VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT` |
| Perf Warn | `VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT` | `VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT` |
| Info | `VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT` | `VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT` or `VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT` |

**Refer to the Validation Features section of the Vulkan Specification for details on this feature.

### <a name="validationfeatures"></a>VK\_EXT\_validation\_flags [DEPRECATED]
One method for an app to control validation layer features is through the `VK_EXT_validation_flags` extension.
**This extension has been deprecated and should not be used for new applications.**

Refer to the Validation Flags extension section of the Vulkan Specification for additional details.



