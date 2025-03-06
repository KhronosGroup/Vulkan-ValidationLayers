<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2025 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Synchronization Validation

Synchronization Validation is implemented in the `VK_LAYER_KHRONOS_validation layer`. When enabled, the Synchronization Object is intended to identify resource access conflicts due to missing or incorrect synchronization operations between actions (Draw, Copy, Dispatch, Blit) reading or writing the same regions of memory.

Synchronization will ideally be run periodically after resolving any outstanding validation checks from all other validation objects, so that issues may be addressed in early stages of development.

The specific areas covered by this layer are currently tracked in the
[Synchronization Validation Project](https://github.com/KhronosGroup/Vulkan-ValidationLayers/projects/5).
Requests for additional checks can be requested by creating a [Github issue](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues).

## Enabling Synchronization Validation

Synchronization Validation is just a [normal layer setting](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/docs/settings.md) that can be turned on. 

The main 3 ways to turn on Sync

1. We **highly** suggest people to use [VkConfig](https://www.lunarg.com/introducing-the-new-vulkan-configurator-vkconfig/) and use the Synchronization Preset.

>  **NOTE** - This will turn off other non-sync validation for you makeing Synchronization Validation run faster.

2. Use `VK_EXT_layer_settings`

```c++
// Will turn on as an additional setting with core validation
const VkBool32 verbose_value = true;
const VkLayerSettingEXT layer_setting = {"VK_LAYER_KHRONOS_validation", "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &verbose_value};
VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layer_setting};

VkInstanceCreateInfo instance_ci = GetYourCreateInfo();
instance_ci.pNext = &layer_settings_create_info;
```

3. Set as an environment variable (will turn on as an additional setting with core validation)

```bash
# Windows
set VK_LAYER_VALIDATE_SYNC=1

# Linux
export VK_LAYER_VALIDATE_SYNC=1

# Android
adb setprop debug.vulkan.khronos_validation.validate_sync=1
```

[Additional configuration settings](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html) will all share the `VK_LAYER_SYNCVAL_`/`khronos_validation.syncval_*` prefix namespace.
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



### Current Feature set

- Hazard detection for memory usage for commands within the *same* command buffer.
- Synchronization operations .
  - vkCmdPipelineBarrier.
  - vkCmdSetEvent/vkCmdWaitEvents/vkCmdResetEvent.
  - renderpass/subpass barriers.
- The `VK_KHR_synchronization2` extension
  - vkCmdPipelineBarrier2KHR
  - vkCmdSetEvent2KHR/vkCmdWaitEvents2KHR/vkCmdResetEvent2KHR.
- Image layout transition hazard and access tracking.
- Load/Store/Resolve operations within Subpasses.
- ExecuteCommands detection of hazard from or with secondary command buffers

- QueueSubmit/QueueSubmit2 time hazard detection
- Semaphore and Fence synchronization operations/effects
- Device and Queue WaitIdle support
- Dynamic Rendering support

### Known Limitations
- Does not support precise tracking of descriptors accessed by the shader (requires integration with GPU-AV)
- Hazards related to memory aliasing are not detected properly
- Indirectly accessed (indirect/indexed) buffers validated at *binding* granularity. (Every valid location assumed to be accessed.)
- Queue family ownership transfer not supported
- Host set event not supported.
- No dedicated support for sparse resources. Need to investigate which kind of support is needed.
- Host memory accesses are not tracked. Corresponding race conditions are not reported.
- Does not include implementation of multi-view renderpass support.
- Memory access checks not suppressed for VK_CULL_MODE_FRONT_AND_BACK.
- Does not include component granularity access tracking, or correctly support swizzling.

## Typical Synchronization Validation Usage

### Debugging Synchronization Validation Issues

To debug synchronization validation issues (all platforms):

- Create a debug callback with `vkCreateDebugUtilsMessengerEXT` with the `VK_DEBUG_REPORT_ERROR_BIT_EXT` set.
- Enable synchronization as noted above. On Linux and Windows this can be simplified by enabling Synchronization Validation using [Vulkan Configurator (vkconfig)](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html).
- Set a breakpoint in the debug callback and run your application in the debugger.
- The callback will be called when a `vkCmd`... command with a hazard is recorded.

On Windows, Synchronization Validation can be run using just vkconfig and the debugger without defining a callback:

*   In vkconfig.
    *   Enable Synchronization Validation.
    *   Select 'Debug Actions' 'Break' and 'Debug Output'.
*   Debug application in Visual Studio.
*   Hazard messages will appear in the debugger output window and the debugger will break (in the validation layer code)  when a `vkCmd`... command with a hazard is recorded.


### Synchronization Validation Messages

A synchronization validation error message describes a race condition by identifying two memory accesses that caused the hazard and the state of applied synchronization.

Error message example:

> vkCmdExecuteCommands(): WRITE_AFTER_READ hazard detected. vkCmdCopyImage (from the secondary VkCommandBuffer 0x1fb2f224d40) writes to VkImage 0xf56c9b0000000004, which was previously read by another vkCmdCopyImage command (from the primary VkCommandBuffer 0x1fb245f4200).
>
> No sufficient synchronization is present to ensure that a write (VK_ACCESS_2_TRANSFER_WRITE_BIT) at VK_PIPELINE_STAGE_2_COPY_BIT does not conflict with a prior read (VK_ACCESS_2_TRANSFER_READ_BIT) at the same stage.
>
> Vulkan insight: an execution dependency is sufficient to prevent this hazard.

The error message usually includes the following:
* The Vulkan API function that triggered the synchronization error
* A brief description of the type of race condition (e.g., WRITE_AFTER_WRITE)
* The commands that performed memory accesses and the resource involved (e.g., vkCmdCopyBuffer and vkCmdDispatch accessing the same VkImage)
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

Extra properties can be enabled in Vulkan Configurator or by using `khronos_validation.syncval_message_extra_properties` validation layer setting.

### Frequently Found Issues

*   Assuming Pipeline stages are logically extended with respect to memory access barriers.  Specifying the vertex shader stage in a barrier will **not** apply to all subsequent shader stages read/write access.
*   Invalid stage/access pairs (specifying a pipeline stage for which a given access is not valid) that yield no barrier.
*   Relying on implicit subpass dependencies with `VK_SUBPASS_EXTERNAL` when memory barriers are needed.
*   Missing memory dependencies with Image Layout Transitions from pipeline barrier or renderpass Begin/Next/End operations.
*   Missing stage/access scopes for load operations, noting that color and depth/stencil are done by different stage/access.


### Debugging Tips

*   Read and write barriers in the error message can help identify the synchronization operation (either subpass dependency or pipeline barrier) with insufficient or incorrect destination stage/access masks (second scope).
*   `Access info read_barrier` and `write_barrier` values of 0, reflect the absence of any barrier, and can indicate an insufficient or incorrect source mask (first scope).
*   Insert additional barriers with stage/access `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`, `VK_ACCESS_MEMORY_READ_BIT`|`VK_ACCESS_MEMORY_WRITE_BIT` for both` src*Mask` and `dst*Mask` fields to locate missing barriers. If the inserted barrier _resolves_ a hazard, the conflicting access _happens-before_ the inserted barrier. (Be sure to delete later.)


## Synchronization blogs/articles

Synchronization Examples[ https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples](https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples)

Keeping your GPU fed without getting bitten [ https://www.youtube.com/watch?v=oF7vOTTaAh4](https://www.youtube.com/watch?v=oF7vOTTaAh4)

Yet another blog explaining Vulkan synchronization[ http://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization/](http://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization/)

A Guide to Vulkan Synchronization Validation https://www.khronos.org/news/permalink/blog-a-guide-to-vulkan-synchronization-validation

