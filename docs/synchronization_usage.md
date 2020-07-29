<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2020 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Synchronization Validation (Alpha)

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/

Synchronization Validation (alpha release)

Synchronization Validation is implemented in the `VK_LAYER_KHRONOS_validation layer`. When enabled, the Synchronization Object is intended to identify resource access conflicts due to missing or incorrect synchronization operations between actions (Draw, Copy, Dispatch, Blit) reading or writing the same regions of memory.

Synchronization will ideally be run periodically after resolving any outstanding validation checks from all other validation objects, so that issues may be addressed in early stages of development.

Synchronization can easily be enabled and configured using the [Vulkan Configurator](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) included with the Vulkan SDK. You can also manually enable Synchronization following instructions below.



The specific areas covered by this layer are currently tracked in
[Github Issue #72: Validate synchronization correctness: Sync Tracking](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/72)
Requests for additional checks can be posted through the same issue, or by creating a new Github issue.

## Synchronization Validation Functionality

### Overview

The pipelined and multi-threaded nature of Vulkan makes it particularly important for applications to correctly insert needed synchronization primitives, and for validation to diagnose unprotected memory access hazards. Synchronization reports the presence of access hazards including information to identify the Vulkan operations which are in conflict. The reported hazards are:


<table>
  <tr>
   <td>RAW
   </td>
   <td>Read-after-write
   </td>
   <td>Occurs when a subsequent operation uses the result of a previous operation without waiting for the result to be completed.
   </td>
  </tr>
  <tr>
   <td>WAR
   </td>
   <td>Write-after-read
   </td>
   <td>Occurs when a subsequent operation overwrites a memory location read by a previous operation before that operation is complete. (requires only execution dependency)
   </td>
  </tr>
  <tr>
   <td>WAW
   </td>
   <td>Write-after-write
   </td>
   <td>Occurs when a subsequent operation writes to the same set of memory locations (in whole or in part) being written by a previous operation.
   </td>
  </tr>
  <tr>
   <td>WRW
   </td>
   <td>Write-racing-write
   </td>
   <td>Occurs when unsynchronized subpasses/queues perform writes to the same set of memory locations.
   </td>
  </tr>
  <tr>
   <td>RRW
   </td>
   <td>Read-racing-write
   </td>
   <td>Occurs when unsynchronized subpasses/queues perform read and write operations on the same set of memory locations
   </td>
  </tr>
</table>



### Current Feature set

- Hazard detection for memory usage for commands within the *same* command buffer
- Synchronization operations vkCmdPipelineBarrier and renderpass/subpass barriers
- Image layout transition hazard and access tracking
- Load/Store/Resolve operations within Subpasses.

### Known Limitations

- Does not include implementation of multi-view renderpass support.
- Does not include vkCmd(Set|Wait)Event support.
- Host set event not supported
- ExecuteCommands and QueueSubmit hazards from are not tracked or reported
- Memory access checks not suppressed for VK_CULL_MODE_FRONT_AND_BACK
- Does not include component granularity access tracking.
- Host synchronization not supported

## Enabling Synchronization Validation

Synchronization Validation is disabled by default. To turn on Synchronization Validation, add the following to your layer settings file,
`vk_layer_settings.txt`:

```code
khronos_validation.enables = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION
```

To enable using environment variables, set the following variable:

```code
VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION
```

Some platforms do not support configuration of the validation layers with this configuration file.
Programs running on these platforms must then use the programmatic interface.

As Synchronization Validation is resource intensive, it is recommended to disable all other validation layer objects.

### Enabling and Specifying Options with the Programmatic Interface

The `VK_EXT_validation_features` extension can be used to enable Synchronization Validation at CreateInstance time.

Here is sample code illustrating how to enable it:

```code
VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION};
VkValidationFeaturesEXT features = {};
features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
features.enabledValidationFeatureCount = 1;
features.pEnabledValidationFeatures = enables;

VkInstanceCreateInfo info = {};
info.pNext = &features;
```
