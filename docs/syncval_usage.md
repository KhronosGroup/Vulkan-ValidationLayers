<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2026 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Synchronization Validation

Synchronization Validation (SyncVal) is part of the `VK_LAYER_KHRONOS_validation` layer. It identifies resource access conflicts caused by missing or incorrect synchronization between commands that read or write the same regions of memory.

We recommend running Synchronization Validation regularly during development, after resolving errors reported by core validation.

Report problems or request additional checks by creating a [GitHub issue](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues).

## Enabling Synchronization Validation

Enable the `VK_LAYER_KHRONOS_validation` layer and turn on its `validate_sync` [layer setting](./khronos_validation_layer.md#configuring-the-validation-layer).

Three common ways to enable Synchronization Validation are:

1. Use [Vulkan Configurator (vkconfig)](https://www.lunarg.com/introducing-the-new-vulkan-configurator-vkconfig/) and select the **Synchronization Only** preset.

>  **NOTE** - This preset disables other validation checks to reduce overhead.

2. Use `VK_EXT_layer_settings`

```c++
// Will turn on as an additional setting with core validation
const VkBool32 enable_sync = VK_TRUE;
const VkLayerSettingEXT layer_setting = {"VK_LAYER_KHRONOS_validation", "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &enable_sync};
VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layer_setting};

VkInstanceCreateInfo instance_ci = GetYourCreateInfo();
instance_ci.pNext = &layer_settings_create_info;
```

3. Set as an environment variable (will turn on as an additional setting with core validation)

```bash
# Windows
set VK_VALIDATION_VALIDATE_SYNC=1

# Linux
export VK_VALIDATION_VALIDATE_SYNC=1

# Android
adb shell setprop debug.vulkan.khronos_validation.validate_sync 1
```

There are additional [SyncVal settings](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html) that use the `syncval_` prefix.

### When Errors Are Reported

Synchronization Validation runs its full set of checks when command buffers are submitted with `vkQueueSubmit` or `vkQueueSubmit2`. Every supported hazard is detected at this point.

**Record-time reporting**, enabled by default, runs a subset of these checks earlier, during command buffer recording. Errors it finds are reported at the `vkCmd*` call that causes the hazard, so a debugger can stop while the application is recording that command.

Record-time reporting adds CPU overhead. If you do not need errors at record time, disable it with the `syncval_record_time_validation` setting or by turning off `Record-time reporting` in Vulkan Configurator. Validation coverage is unchanged, all errors are reported at submission, and validation runs faster.

## Synchronization Validation Messages

A synchronization validation error message describes a race condition by identifying two memory accesses that caused the hazard and the state of applied synchronization.

Example of an error reported during `vkCmdExecuteCommands` with record-time reporting enabled:

> vkCmdExecuteCommands(): WRITE_AFTER_READ hazard detected. vkCmdCopyImage (from the secondary VkCommandBuffer 0x1fb2f224d40) writes to VkImage 0xf56c9b0000000004, which was previously read by another vkCmdCopyImage command (from the primary VkCommandBuffer 0x1fb245f4200).
>
> No sufficient synchronization is present to ensure that a write (VK_ACCESS_2_TRANSFER_WRITE_BIT) at VK_PIPELINE_STAGE_2_COPY_BIT does not conflict with a prior read (VK_ACCESS_2_TRANSFER_READ_BIT) at the same stage.
>
> Vulkan insight: an execution dependency is sufficient to prevent this hazard.

The error message usually includes the following:

* The Vulkan API function at which the error was reported. For errors reported during submission, this is the queue submission call, while the message identifies the recorded commands involved in the hazard.
* A brief description of the type of race condition (e.g., WRITE_AFTER_WRITE)
* The commands that performed memory accesses and the resource involved (e.g., vkCmdCopyBuffer and vkCmdDispatch accessing the same VkBuffer)
* Synchronization details: pipeline stages, access types, and applied synchronization
* In some cases, a "Vulkan insight" section at the end of the error message may provide additional information related to the current error

Unlike core validation error messages, where each message is identified by a VUID, synchronization validation primarily detects a single type of error: a race condition between two memory accesses. There are limitless ways to produce a race condition (combinations of command pairs and different synchronization methods), which is why race condition scenarios are not identified by VUIDs.

To suppress or filter synchronization validation error messages, one can use the optional `Extra properties` section. Extra properties contain key-value pairs that help identify the error message and are presented in a more structured format compared to the main error message, making parsing easier.

One of the benefits of parsing `Extra properties` rather than the main error message is that the former is more stable and changes less frequently. This creates a nice separation: on one side, we can take every opportunity to improve the error message wording while keeping the Extra properties values unchanged in most cases, so suppression and filtering logic does not need to be updated.

Example of error message with extra properties enabled:
> vkQueueSubmit(): WRITE_AFTER_WRITE hazard detected. vkCmdEndRenderPass (from VkCommandBuffer  submitted on the current VkQueue ) writes to resource, which was previously written by vkCmdClearColorImage (from VkCommandBuffer  submitted on VkQueue ).
>
>The current synchronization allows VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT accesses at VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, but to prevent this hazard, it must allow VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT accesses at VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT.

```
[Extra properties]
message_type = SubmitTimeError
hazard_type = WRITE_AFTER_WRITE
prior_access = VK_PIPELINE_STAGE_2_CLEAR_BIT(VK_ACCESS_2_TRANSFER_WRITE_BIT)
write_barriers = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT(VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT)
command = vkCmdEndRenderPass
prior_command = vkCmdClearColorImage
command_buffer_index = 1
submit_index = 2
batch_index = 0
batch_tag = 5
```

Extra properties can be enabled in Vulkan Configurator or by using the `syncval_message_extra_properties` validation layer setting.

## Synchronization Validation Functionality

### Overview

The pipelined and multi-threaded nature of Vulkan makes it particularly important for applications to correctly insert needed synchronization primitives, and for validation to diagnose unprotected memory access hazards. Synchronization Validation reports the presence of access hazards including information to identify the Vulkan operations which are in conflict. The reported hazards are:


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
   <td>Occurs when a subsequent operation overwrites a memory location read by a previous operation before that operation is complete (requires only execution dependency).
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
   <td>Occurs when unsynchronized subpasses/queues perform read and write operations on the same set of memory locations.
   </td>
  </tr>
</table>



### Current Features

- Hazard detection within and between command buffers, submissions, and queues.
- Pipeline barriers, event operations, and render pass dependencies, including synchronization2 commands from Vulkan 1.3 and `VK_KHR_synchronization2`.
- Image layout transition hazard and access tracking.
- Attachment load, store, and resolve operations in render passes and dynamic rendering.
- Secondary command buffers executed with `vkCmdExecuteCommands`.
- Semaphore and fence synchronization, and device and queue wait-idle operations.

### Known Limitations
- Does not support precise tracking of descriptors accessed by the shader (requires integration with GPU-AV). This includes both classic VkDescriptorSet and VK_EXT_descriptor_buffer APIs
- Hazards related to memory aliasing are not detected properly
- Indirectly accessed buffers (indirect data/indexed vertex data) are not validated
- Host set event not supported
- No dedicated support for sparse resources
- Host memory accesses are not tracked. Corresponding race conditions are not reported
- Does not include component granularity access tracking, or correctly support swizzling

## Synchronization blogs/articles

Synchronization Examples[ https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples](https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples)

Keeping your GPU fed without getting bitten [ https://www.youtube.com/watch?v=oF7vOTTaAh4](https://www.youtube.com/watch?v=oF7vOTTaAh4)

Yet another blog explaining Vulkan synchronization[ http://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization/](http://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization/)

A Guide to Vulkan Synchronization Validation https://www.khronos.org/news/permalink/blog-a-guide-to-vulkan-synchronization-validation
