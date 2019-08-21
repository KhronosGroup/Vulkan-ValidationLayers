<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2019 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# GPU-Assisted Validation

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/

GPU-Assisted validation is implemented in the SPIR-V Tools optimizer and the `VK_LAYER_KHRONOS_validation layer (or, in the
soon-to-be-deprecated `VK_LAYER_LUNARG_core_validation` layer). This document covers the design of the layer portion of the
implementation.

## Basic Operation

The basic operation of GPU-Assisted validation is comprised of instrumenting shader code to perform run-time checking in shaders and
reporting any error conditions to the layer.
The layer then reports the errors to the user via the same reporting mechanisms used by the rest of the validation system.

The layer instruments the shaders by passing the shader's SPIR-V bytecode to the SPIR-V optimizer component and
instructs the optimizer to perform an instrumentation pass to add the additional instructions to perform the run-time checking.
The layer then passes the resulting modified SPIR-V bytecode to the driver as part of the process of creating a ShaderModule.

The layer also allocates a buffer that describes the length of all descriptor arrays and the write state of each element of each array.
It only does this if the VK_EXT_descriptor_indexing extension is enabled.

As the shader is executed, the instrumented shader code performs the run-time checks.
If a check detects an error condition, the instrumentation code writes an error record into the GPU's device memory.
This record is small and is on the order of a dozen 32-bit words.
Since multiple shader stages and multiple invocations of a shader can all detect errors, the instrumentation code
writes error records into consecutive memory locations as long as there is space available in the pre-allocated block of device memory.

The layer inspects this device memory block after completion of a queue submission.
If the GPU had written an error record to this memory block,
the layer analyzes this error record and constructs a validation error message
which is then reported in the same manner as other validation messages.
If the shader was compiled with debug information (source code and SPIR-V instruction mapping to source code lines), the layer
also provides the line of shader source code that provoked the error as part of the validation error message.

## GPU-Assisted Validation Checks

The initial release (Jan 2019) of GPU-Assisted Validation includes checking for out-of-bounds descriptor array indexing
for image/texel descriptor types.

The second release (Apr 2019) adds validation for out-of-bounds descriptor array indexing and use of unwritten descriptors when the 
VK_EXT_descriptor_indexing extension is enabled.  Also added (June 2019) was validation for buffer descriptors.

### Out-of-Bounds(OOB) Descriptor Array Indexing

Checking for correct indexing of descriptor arrays is sometimes referred to as "bind-less validation".
It is called "bind-less" because a binding in a descriptor set may contain an array of like descriptors.
And unless there is a constant or compile-time indication of which descriptor in the array is selected,
the descriptor binding status is considered to be ambiguous, leaving the actual binding to be determined at run-time.

As an example, a fragment shader program may use a variable to index an array of combined image samplers.
Such a line might look like:

```glsl
uFragColor = light * texture(tex[tex_ind], texcoord.xy);
```

The array of combined image samplers is `tex` and has 6 samplers in the array.
The complete validation error message issued when `tex_ind` indexes past the array is:

```terminal
ERROR : VALIDATION - Message Id Number: 0 | Message Id Name: UNASSIGNED-Image descriptor index out of bounds
        Index of 6 used to index descriptor array of length 6.  Command buffer (CubeDrawCommandBuf)(0xbc24b0).
        Pipeline (0x45). Shader Module (0x43). Shader Instruction Index = 108.  Stage = Fragment.
        Fragment coord (x,y) = (419.5, 254.5). Shader validation error occurred in file:
        /home/user/src/Vulkan-ValidationLayers/external/Vulkan-Tools/cube/cube.frag at line 45.
45:    uFragColor = light * texture(tex[tex_ind], texcoord.xy);
```
The VK_EXT_descriptor_indexing extension allows a shader to declare a descriptor array without specifying its size
```glsl
layout(set = 0, binding = 1) uniform sampler2D tex[];
```
In this case, the layer needs to tell the optimization code how big the descriptor array is so the code can determine what is out of 
bounds and what is not.

The extension also allows descriptor set bindings to be partially bound, meaning that as long as the shader doesn't use certain
array elements, those elements are not required to have been written.
The instrumentation code needs to know which elements of a descriptor array have been written, so that it can tell if one is used
that has not been written.

Note that currently, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT validation is not working and all accesses are reported as valid.


## GPU-Assisted Validation Options

Here are the options related to activating GPU-Assisted Validation:

1. Enable GPU-Assisted Validation - GPU-Assisted Validation is off by default and must be enabled.

    GPU-Assisted Validation is disabled by default because the shader instrumentation may introduce significant
    shader performance degradation and additional resource consumption.
    GPU-Assisted Validation requires additional resources such as device memory and descriptors.
    It is desirable for the user to opt-in to this feature because of these requirements.
    In addition, there are several limitations that may adversely affect application behavior,
    as described later in this document.

2. Reserve a Descriptor Set Binding Slot - Modifies the value of the `VkPhysicalDeviceLimits::maxBoundDescriptorSets`
   property to return a value one less than the actual device's value to "reserve" a descriptor set binding slot for use by GPU validation.

   This option is likely only of interest to applications that dynamically adjust their descriptor set bindings to adjust for
   the limits of the device.

### Enabling and Specifying Options with a Configuration File

The existing layer configuration file mechanism can be used to enable GPU-Assisted Validation.
This mechanism is described on the
[LunarXchange website](https://vulkan.lunarg.com/doc/sdk/latest/windows/layer_configuration.html),
in the "Layers Overview and Configuration" document.

To turn on GPU validation, add the following to your layer settings file, which is often
named `vk_layer_settings.txt`.

```code
khronos_validation.gpu_validation = all
```

To turn on GPU validation and request to reserve a binding slot:

```code
khronos_validation.gpu_validation = all,reserve_binding_slot
```

Note: When using the core_validation layer, the above settings should use `lunarg_core_validation` in place of
`khronos_validation`.

Some platforms do not support configuration of the validation layers with this configuration file.
Programs running on these platforms must then use the programmatic interface.

### Enabling and Specifying Options with the Programmatic Interface

The `VK_EXT_validation_features` extension can be used to enable GPU-Assisted Validation at CreateInstance time.

Here is sample code illustrating how to enable it:

```C
VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
VkValidationFeaturesEXT features = {};
features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
features.enabledValidationFeatureCount = 1;
features.pEnabledValidationFeatures = enables;

VkInstanceCreateInfo info = {};
info.pNext = &features;
```

Use the `VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT` enum to reserve a binding slot.

## GPU-Assisted Validation Limitations

There are several limitations that may impede the operation of GPU-Assisted Validation:

### Vulkan 1.1

Vulkan 1.1 or later is required because the GPU instrumentation code uses SPIR-V 1.3 features.
Vulkan 1,1 is required to ensure that SPIR-V 1.3 is available.

### Descriptor Types

The current implementation works with image, texel, and buffer descriptor types.
A complete list appears later in this document.

### Descriptor Set Binding Limit

This is probably the most important limitation and is related to the
`VkPhysicalDeviceLimits::maxBoundDescriptorSets` device limit.

When applications use all the available descriptor set binding slots,
GPU-Assisted Validation cannot be performed because it needs a descriptor set to
locate the memory for writing the error report record.

This problem is most likely to occur on devices, often mobile, that support only the
minimum required value for `VkPhysicalDeviceLimits::maxBoundDescriptorSets`, which is 4.
Some applications may be written to use 4 slots since this is the highest value that
is guaranteed by the specification.
When such an application using 4 slots runs on a device with only 4 slots,
then GPU-Assisted Validation cannot be performed.

In this implementation, this condition is detected and gracefully recovered from by
building the graphics pipeline with non-instrumented shaders instead of instrumented ones.
An error message is also displayed informing the user of the condition.

Applications don't have many options in this situation and it is anticipated that
changing the application to free a slot is difficult.

### Device Memory

GPU-Assisted Validation does allocate device memory for the error report buffers, and if
descriptor indexing is enabled, for the input buffer of descriptor sizes and write state.
This can lead to a greater chance of memory exhaustion, especially in cases where
the application is trying to use all of the available memory.
The extra memory allocations are also not visible to the application, making it
impossible for the application to account for them.

Note that if descriptor indexing is enabled, the input buffer size will be equal to
(1 + (number_of_sets * 2) + (binding_count * 2) + descriptor_count) words of memory where
binding_count is the binding number of the largest binding in the set.  
This means that sparsely populated sets and sets with a very large binding will cause
the input buffer to be much larger than it could be with more densely packed binding numbers.
As a best practice, when using GPU-Assisted Validation with descriptor indexing enabled,
make sure descriptor bindings are densely packed.

If GPU-Assisted Validation device memory allocations fail, the device could become
unstable because some previously-built pipelines may contain instrumented shaders.
This is a condition that is nearly impossible to recover from, so the layer just
prints an error message and refrains from any further allocations or instrumentations.
There is a reasonable chance to recover from these conditions,
especially if the instrumentation does not write any error records.

### Descriptors

This is roughly the same problem as the device memory problem mentioned above,
but for descriptors.
Any failure to allocate a descriptor set means that the instrumented shader code
won't have a place to write error records, resulting in unpredictable device
behavior.

### Other Device Limits

This implementation uses additional resources that may count against the following limits,
and possibly others:

* `maxMemoryAllocationCount`
* `maxBoundDescriptorSets`
* `maxPerStageDescriptorStorageBuffers`
* `maxPerStageResources`
* `maxDescriptorSetStorageBuffers`
* `maxFragmentCombinedOutputResources`

The implementation does not take steps to avoid exceeding these limits
and does not update the tracking performed by other validation functions.

### A Note About the `VK_EXT_buffer_device_address` Extension

The recently introduced `VK_EXT_buffer_device_address` extension can be used
to implement GPU-Assisted Validation without some of the limitations described above.
This approach would use this extension to obtain a GPU device pointer to a storage
buffer and make it available to the shader via a specialization constant.
This technique removes the need to create descriptors, use a descriptor set slot,
modify pipeline layouts, etc, and would relax some of the limitations listed above.

This alternate implementation is under consideration.

## GPU-Assisted Validation Internal Design

This section may be of interest to readers who are interested on how GPU-Assisted Validation is implemented.
It isn't necessarily required for using the feature.

### General

In general, the implementation does:

* For each draw, dispatch, and trace rays call, allocate a buffer with enough device memory to hold a single debug output record written by the
    instrumented shader code.
    If descriptor indexing is enabled, calculate the amount of memory needed to describe the descriptor arrays sizes and
    write states and allocate device memory and a buffer for input to the instrumented shader.
    The Vulkan Memory Allocator is used to handle this efficiently.

    There is probably little advantage in providing a larger output buffer in order to obtain more debug records.
    It is likely, especially for fragment shaders, that multiple errors occurring near each other have the same root cause.

    A block is allocated on a per draw basis to make it possible to associate a shader debug error record with
    a draw within a command buffer.
    This is done partly to give the user more information in the error report, namely the command buffer handle/name and the draw within that command buffer.
    An alternative design allocates this block on a per-device or per-queue basis and should work.
    However, it is not possible to identify the command buffer that causes the error if multiple command buffers
    are submitted at once.
* For each draw, dispatch, and trace rays call, allocate a descriptor set and update it to point to the block of device memory just allocated.
    If descriptor indexing is enabled, also update the descriptor set to point to the allocated input buffer.
    Fill the input buffer with the size and write state information for each descriptor array.
    There is a descriptor set manager to handle this efficiently.
    Also make an additional call down the chain to create a bind descriptor set command to bind our descriptor set at the desired index.
    This has the effect of binding the device memory block belonging to this draw so that the GPU instrumentation
    writes into this buffer for when the draw is executed.
    The end result is that each draw call has its own buffer containing GPU instrumentation error
    records, if any occurred while executing that draw.
* Determine the descriptor set binding index that is eventually used to bind the descriptor set just allocated and updated.
    Usually, it is `VkPhysicalDeviceLimits::maxBoundDescriptorSets` minus one.
    For devices that have a very high or no limit on this bound, pick an index that isn't too high, but above most other device
    maxima such as 32.
* When creating a ShaderModule, pass the SPIR-V bytecode to the SPIR-V optimizer to perform the instrumentation pass.
    Pass the desired descriptor set binding index to the optimizer via a parameter so that the instrumented
    code knows which descriptor to use for writing error report data to the memory block.
    If descriptor indexing is enabled, turn on OOB and write state checking in the instrumentation pass.
    Use the instrumented bytecode to create the ShaderModule.
* For all pipeline layouts, add our descriptor set to the layout, at the binding index determined earlier.
    Fill any gaps with empty descriptor sets.

    If the incoming layout already has a descriptor set placed at our desired index, the layer must not add its
    descriptor set to the layout, replacing the one in the incoming layout.
    Instead, the layer leaves the layout alone and later replaces the instrumented shaders with
    non-instrumented ones when the pipeline layout is later used to create a graphics pipeline.
    The layer issues an error message to report this condition.
* When creating a GraphicsPipeline, ComputePipeline, or RayTracingPipeline, check to see if the pipeline is using the debug binding index.
    If it is, replace the instrumented shaders in the pipeline with non-instrumented ones.
* Before calling QueueSubmit, if descriptor indexing is enabled, check to see if there were any unwritten descriptors that were declared
    update-after-bind.
    If there were, update the write state of those elements.
* After calling QueueSubmit, perform a wait on the queue to allow the queue to finish executing.
    Then map and examine the device memory block for each draw or trace ray command that was submitted.
    If any debug record is found, generate a validation error message for each record found.

The above describes only the high-level details of GPU-Assisted Validation operation.
More detail is found in the discussion of the individual hooked functions below.

### Initialization

When the validation layer loads, it examines the user options from both the layer settings file and the
`VK_EXT_validation_features` extension.
Note that it also processes the subsumed `VK_EXT_validation_flags` extension for simple backwards compatibility.
From these options, the layer sets instance-scope flags in the validation layer tracking data to indicate if
GPU-Assisted Validation has been requested, along with any other associated options.

### "Calling Down the Chain"

Much of the GPU-Assisted Validation implementation involves making "application level" Vulkan API
calls outside of the application's API usage to create resources and perform its required operations
inside of the validation layer.
These calls are not routed up through the top of the loader/layer/driver call stack via the loader.
Instead, they are simply dispatched via the containing layer's dispatch table.

These calls therefore don't pass through any validation checks that occur before the gpu validation checks are run.
This doesn't present any particular problem, but it does raise some issues:

* The additional API calls are not fully validated

  This implies that this additional code may never be checked for validation errors.
  To address this, the code can "just" be written carefully so that it is "valid" Vulkan,
  which is hard to do.

  Or, this code can be checked by loading a khronos validation layer with
  GPU validation enabled on top of "normal" standard validation in the
  layer stack, which effectively validates the API usage of this code.
  This sort of checking is performed by layer developers to check that the additional
  Vulkan usage is valid.

  This validation can be accomplished by:

  * Building the validation layer with a hack to force GPU-Assisted Validation to be enabled.
  Can't use the exposed mechanisms because we probably don't want it on twice.
  * Rename this layer binary to something else like "khronos_validation2" to keep it apart from the
  "normal" khronos validation.
  * Create a new JSON file with the new layer name.
  * Set up the layer stack so that the "khronos_validation2" layer is on top of or before the actual khronos
    validation layer
  * Then run tests and check for validation errors pointing to API usage in the "khronos_validation2" layer.

  This should only need to be done after making any major changes to the implementation.

  Another approach involves capturing an application trace with `vktrace` and then playing
  it back with `vkreplay`.

* The additional API calls are not state-tracked

  This means that things like device memory allocations and descriptor allocations are not
  tracked and do not show up in any of the bookkeeping performed by the validation layers.
  For example, any device memory allocation performed by GPU-Assisted Validation won't be
  counted towards the maximum number of allocations allowed by a device.
  This could lead to an early allocation failure that is not accompanied by a validation error.

  This shortcoming is left as not addressed in this implementation because it is anticipated that
  a later implementation of GPU-Assisted Validation using the `VK_EXT_buffer_device_address`
  extension will have less of a need to allocate these
  tracked resources and it therefore becomes less of an issue.

### Code Structure and Relationship to the Core Validation Layer

The GPU-Assisted Validation code is largely contained in one
[file](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/layers/gpu_validation.cpp), with "hooks" in
the other validation code that call functions in this file.
These hooks in the validation code look something like this:

```C
if (GetEnables(dev_data)->gpu_validation) {
    GpuPreCallRecordDestroyPipeline(dev_data, pipeline_state);
}
```

The GPU-Assisted Validation code is linked into the shared library for the khronos and core validation layers.

#### Review of Khronos Validation Code Structure

Each function for a Vulkan API command intercepted in the khronos validation layer is usually split up
into several decomposed functions in order to organize the implementation.
These functions take the form of:

* PreCallValidate&lt;foo&gt;: Perform validation steps before calling down the chain
* PostCallValidate&lt;foo&gt;: Perform validation steps after calling down the chain
* PreCallRecord&lt;foo&gt;: Perform state recording before calling down the chain
* PostCallRecord&lt;foo&gt;: Perform state recording after calling down the chain

The GPU-Assisted Validation functions follow this pattern not by hooking into the top-level validation API shim, but
by hooking one of these decomposed functions.

The design of each hooked function follows:

#### GpuPreCallRecordCreateDevice

* Modify the `VkPhysicalDeviceFeatures` to turn on two additional physical device features:
  * `fragmentStoresAndAtomics`
  * `vertexPipelineStoresAndAtomics`

#### GpuPostCallRecordCreateDevice

* Determine and record (save in device state) the desired descriptor set binding index.
* Initialize Vulkan Memory Allocator
  * Determine error record block size based on the maximum size of the error record and alignment limits of the device.
* Initialize descriptor set manager
* Make a descriptor set layout to describe our descriptor set
* Make a descriptor set layout to describe a "dummy" descriptor set that contains no descriptors
  * This is used to "pad" pipeline layouts to fill any gaps between the used bind indices and our bind index
* Record these objects in the per-device state

#### GpuPreCallRecordDestroyDevice

* Destroy descriptor set layouts created in CreateDevice
* Clean up descriptor set manager
* Clean up Vulkan Memory Allocator (VMA)
* Clean up device state

#### GpuAllocateValidationResources

* For each Draw, Dispatch, or TraceRays call:
  * Get a descriptor set from the descriptor set manager
  * Get an output buffer and associated memory from VMA
  * If descriptor indexing is enabled, get an input buffer and fill with descriptor array information
  * Update (write) the descriptor set with the memory info
  * Check to see if the layout for the pipeline just bound is using our selected bind index
  * If no conflict, add an additional command to the command buffer to bind our descriptor set at our selected index
* Record the above objects in the per-CB state
Note that the Draw and Dispatch calls include vkCmdDraw, vkCmdDrawIndexed, vkCmdDrawIndirect, vkCmdDrawIndexedIndirect, vkCmdDispatch, vkCmdDispatchIndirect, and vkCmdTraceRaysNV.

#### GpuPreCallRecordFreeCommandBuffers

* For each command buffer:
  * Destroy the VMA buffer(s), releasing the memory
  * Give the descriptor sets back to the descriptor set manager
  * Clean up CB state

#### GpuOverrideDispatchCreateShaderModule

This function is called from PreCallRecordCreateShaderModule.
This routine sets up to call the SPIR-V optimizer to run the "BindlessCheckPass", replacing the original SPIR-V with the instrumented SPIR-V
which is then used in the call down the chain to CreateShaderModule.

This function generates a "unique shader ID" that is passed to the SPIR-V optimizer,
which the instrumented code puts in the debug error record to identify the shader.
This ID is returned by this function so it can be recorded in the shader module at PostCallRecord time.
It would have been convenient to use the shader module handle returned from the driver to use as this shader ID.
But the shader needs to be instrumented before creating the shader module and therefore the handle is not available to use
as this ID to pass to the optimizer.
Therefore, the layer keeps a "counter" in per-device state that is incremented each time a shader is instrumented
to generate unique IDs.
This unique ID is given to the SPIR-V optimizer and is stored in the shader module state tracker after the shader module is created, which creates the necessary association between the ID and the shader module.

The process of instrumenting the SPIR-V also includes passing the selected descriptor set binding index
to the SPIR-V optimizer which the instrumented
code uses to locate the memory block used to write the debug error record.
An instrumented shader is now "hard-wired" to write error records via the descriptor set at that binding
if it detects an error.
This implies that the instrumented shaders should only be allowed to run when the correct bindings are in place.

The original SPIR-V bytecode is left stored in the shader module tracking data.
This is important because the layer may need to replace the instrumented shader with the original shader if, for example,
there is a binding index conflict.
The application cannot destroy the shader module until it has used the shader module to create the pipeline.
This ensures that the original SPIR-V bytecode is available if we need it to replace the instrumented shader.

#### GpuOverrideDispatchCreatePipelineLayout

This is function is called through PreCallRecordCreatePipelineLayout.

* Check for a descriptor set binding index conflict.
  * If there is one, issue an error message and leave the pipeline layout unmodified
  * If no conflict, for each pipeline layout:
    * Create a new pipeline layout
    * Copy the original descriptor set layouts into the new pipeline layout
    * Pad the new pipeline layout with dummy descriptor set layouts up to but not including the last one
    * Add our descriptor set layout as the last one in the new pipeline layout
* Create the pipeline layouts by calling down the chain with the original or modified create info

#### GpuPreCallQueueSubmit

* For each primary and secondary command buffer in the submission:
  * Call helper function to see if there are any update after bind descriptors whose write state may need to be updated
    and if so, map the input buffer and update the state.

#### GpuPostCallQueueSubmit

* Submit a command buffer containing a memory barrier to make GPU writes available to the host domain.
* Call QueueWaitIdle.
* For each primary and secondary command buffer in the submission:
  * Call a helper function to process the instrumentation debug buffers (described later)

#### GpuPreCallValidateCmdWaitEvents

* Report an error about a possible deadlock if CmdWaitEvents is recorded with VK_PIPELINE_STAGE_HOST_BIT set.

#### GpuPreCallRecordCreateGraphicsPipelines

* Examine the pipelines to see if any use the debug descriptor set binding index
* For those that do:
  * Create non-instrumented shader modules from the saved original SPIR-V
  * Modify the CreateInfo data to use these non-instrumented shaders.
    * This prevents instrumented shaders from using the application's descriptor set.

#### GpuPostCallRecordCreateGraphicsPipelines

* For every shader in the pipeline:
  * Destroy the shader module created in GpuPreCallRecordCreateGraphicsPipelines, if any
    * These are found in the CreateInfo used to create the pipeline and not in the shader_module
  * Create a shader tracking record that saves:
    * shader module handle
    * unique shader id
    * graphics pipeline handle
    * shader bytecode if it contains debug info

This tracker is used to attach the shader bytecode to the shader in case it is needed
later to get the shader source code debug info.

The current shader module tracker in the validation code stores the bytecode,
but this tracker has the same life cycle as the shader module itself.
It is possible for the application to destroy the shader module after
creating graphics pipeline and before submitting work that uses the shader,
making the shader bytecode unavailable if needed for later analysis.
Therefore, the bytecode must be saved at this opportunity.

This tracker exists as long as the graphics pipeline exists,
so the graphics pipeline handle is also stored in this tracker so that it can
be looked up when the graphics pipeline is destroyed.
At that point, it is safe to free the bytecode since the pipeline is never used again.

#### GpuPreCallRecordDestroyPipeline

* Find the shader tracker(s) with the graphics pipeline handle and free the tracker, along with any bytecode it has stored in it.

### Shader Instrumentation Scope

The shader instrumentation process performed by the SPIR-V optimizer applies descriptor index bounds checking
to descriptors of the following types:

    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC

Instrumentation is applied to the following SPIR-V operations:

    OpImageSampleImplicitLod
    OpImageSampleExplicitLod
    OpImageSampleDrefImplicitLod
    OpImageSampleDrefExplicitLod
    OpImageSampleProjImplicitLod
    OpImageSampleProjExplicitLod
    OpImageSampleProjDrefImplicitLod
    OpImageSampleProjDrefExplicitLod
    OpImageGather
    OpImageDrefGather
    OpImageQueryLod
    OpImageSparseSampleImplicitLod
    OpImageSparseSampleExplicitLod
    OpImageSparseSampleDrefImplicitLod
    OpImageSparseSampleDrefExplicitLod
    OpImageSparseSampleProjImplicitLod
    OpImageSparseSampleProjExplicitLod
    OpImageSparseSampleProjDrefImplicitLod
    OpImageSparseSampleProjDrefExplicitLod
    OpImageSparseGather
    OpImageSparseDrefGather
    OpImageFetch
    OpImageRead
    OpImageQueryFormat
    OpImageQueryOrder
    OpImageQuerySizeLod
    OpImageQuerySize
    OpImageQueryLevels
    OpImageQuerySamples
    OpImageSparseFetch
    OpImageSparseRead
    OpImageWrite

Also, OpLoad and OpStore with an AccessChain into a base of OpVariable with
either Uniform or StorageBuffer storage class and a type which is either a
struct decorated with Block, or a runtime or statically-sized array of such
a struct.


### Shader Instrumentation Error Record Format

The instrumented shader code generates "error records" in a specific format.

This description includes the support for future GPU-Assisted Validation features
such as checking for uninitialized descriptors in the partially-bound scenario.
These items are not used in the current implementation for descriptor array
bounds checking, but are provided here to complete the description of the
error record format.

The format of this buffer is as follows:

```C
struct DebugOutputBuffer_t
{
   uint DataWrittenLength;
   uint Data[];
}
```

`DataWrittenLength` is the number of uint32_t words that have been attempted to be written.
It should be initialized to 0.

The `Data` array is the uint32_t words written by the shaders of the pipeline to record bindless validation errors.
All elements of `Data` should be initialized to 0.
Note that the `Data` array has runtime length.
The shader queries the length of the `Data` array to make sure that it does not write past the end of `Data`.
The shader only writes complete records.
The layer uses the length of `Data` to control the number of records written by the shaders.

The `DataWrittenLength` is atomically updated by the shaders so that shaders do not overwrite each others data.
The shader takes the value it gets from the atomic update.
If the value plus the record length is greater than the length of `Data`, it does not write the record.

Given this protocol, the value in `DataWrittenLength` is not very meaningful if it is greater than the length of `Data`.
However, the format of the written records plus the fact that `Data` is initialized to 0 should be enough to determine
the records that were written.

### Record Format

The format of an output record is the following:

    Word 0: Record size
    Word 1: Shader ID
    Word 2: Instruction Index
    Word 3: Stage
    <Stage-Specific Words>
    <Validation-Specific Words>

The Record Size is the number of words in this record, including the the Record Size.

The Shader ID is a handle that was provided by the layer when the shader was instrumented.

The Instruction Index is the instruction within the original function at which the error occurred.
For bindless, this will be the instruction which consumes the descriptor in question,
or the instruction that consumes the OpSampledImage that consumes the descriptor.

The Stage is the integer value used in SPIR-V for each of the Execution Models:

| Stage         | Value |
|---------------|:-----:|
|Vertex         |0      |
|TessCtrl       |1      |
|TessEval       |2      |
|Geometry       |3      |
|Fragment       |4      |
|Compute        |5      |
|RayGenerationNV|5313   |
|IntersectionNV |5314   |
|AnyHitNV       |5315   |
|ClosestHitNV   |5316   |
|MissNV         |5317   |
|CallableNV     |5318   |

### Stage Specific Words

These are words that identify which "instance" of the shader the validation error occurred in.
Here are words for each stage:

| Stage         | Word 0           | Word 1     | World 2    |
|---------------|------------------|------------|------------|
|Vertex         |VertexID          |InstanceID  | unused     |
|Tess*          |InvocationID      |unused      | unused     |
|Geometry       |PrimitiveID       |InvocationID| unused     |
|Fragment       |FragCoord.x       |FragCoord.y | unused     |
|Compute        |GlobalInvocationID|unused      | unused     |
|RayGenerationNV|LaunchIdNV.x      |LaunchIdNV.y|LaunchIdNV.z|
|IntersectionNV |LaunchIdNV.x      |LaunchIdNV.y|LaunchIdNV.z|
|AnyHitNV       |LaunchIdNV.x      |LaunchIdNV.y|LaunchIdNV.z|
|ClosestHitNV   |LaunchIdNV.x      |LaunchIdNV.y|LaunchIdNV.z|
|MissNV         |LaunchIdNV.x      |LaunchIdNV.y|LaunchIdNV.z|
|CallableNV     |LaunchIdNV.x      |LaunchIdNV.y|LaunchIdNV.z|

"unused" means not relevant, but still present.

### Validation-Specific Words

These are words that are specific to the validation being done.
For bindless validation, they are variable.

The first word is the Error Code.

For the *OutOfBounds errors, two words will follow: Word0:DescriptorIndex, Word1:DescriptorArrayLength

For the *Uninitialized errors, one word will follow: Word0:DescriptorIndex

| Error                       | Code | Word 0         | Word 1                |
|-----------------------------|:----:|----------------|-----------------------|
|IndexOutOfBounds             |0     |Descriptor Index|Descriptor Array Length|
|DescriptorUninitialized      |1     |Descriptor Index|unused                 |

So the words written for an image descriptor bounds error in a fragment shader is:

    Word 0: Record size (9)
    Word 1: Shader ID
    Word 2: Instruction Index
    Word 3: Stage (4:Fragment)
    Word 4: FragCoord.x
    Word 5: FragCoord.y
    Word 6: Error (0: ImageIndexOutOfBounds)
    Word 7: DescriptorIndex
    Word 8: DescriptorArrayLength

If another error is encountered, that record is written starting at Word 10, if the whole record will not overflow Data.
If overflow will happen, no words are written..

The validation layer can continue to read valid records until it sees a Record Length of 0 or the end of Data is reached.

#### Programmatic interface

The programmatic interface for the above informal description is codified in the
[SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) repository in file
[`instrument.hpp`](https://github.com/KhronosGroup/SPIRV-Tools/blob/master/include/spirv-tools/instrument.hpp).
It consists largely of integer constant definitions for the codes and values mentioned above and
offsets into the record for locating each item.

## GPU-Assisted Validation Error Report

This is a fairly simple process of mapping the debug report buffer associated with
each draw in the command buffer that was just submitted and looking to see if the GPU instrumentation
code wrote anything.
Each draw in the command buffer should have a corresponding result buffer in the command buffer's list of result buffers.
The report generating code loops through the result buffers, maps each of them, checks for errors, and unmaps them.
The layer clears the buffer to zeros when it is allocated and after processing any
buffer that was written to.
The instrumented shader code expects these buffers to be cleared to zeros before it
writes to them.

The layer then prepares a "common" validation error message containing:

* command buffer handle - This is easily obtained because we are looping over the command
  buffers just submitted.
* draw number - keep track of how many draws we've processed for a given command buffer.
* pipeline handle - The shader tracker discussed earlier contains this handle
* shader module handle - The "Shader ID" (Word 1 in the record) is used to lookup
  the shader tracker which is then used to obtain the shader module and pipeline handles
* instruction index - This is the SPIR-V instruction index where the invalid array access occurred.
  It is not that useful by itself, since the user would have to use it to locate a SPIR-V instruction
  in a SPIR-V disassembly and somehow relate it back to the shader source code.
  But it could still be useful to some and it is easy to report.
  The user can build the shader with debug information to get source-level information.

For all objects, the layer also looks up the objects in the Debug Utils object name map in
case the application used that extension to name any objects.
If a name exists for that object, it is included in the error message.

The layer then adds on error message text obtained from decoding the stage-specific and
validation-specific data as described earlier.

This completes the error report when there is no source-level debug information in the shader.

### Source-Level Debug Information

This is one of the more complicated and code-heavy parts of the GPU-Assisted Validation feature
and all it really does is display source-level information when the shader is compiled
with debugging info (`-g` option in the case of `glslangValidator`).

The process breaks down into two steps:

#### OpLine Processing

The SPIR-V generator (e.g., glslangValidator) places an OpLine SPIR-V instruction in the
shader program ahead of code generated for each source code statement.
The OpLine instruction contains the filename id (for an OpString),
the source code line number and the source code column number.
It is possible to have two source code statements on the same line in the source file,
which explains the need for the column number.

The layer scans the SPIR-V looking for the last OpLine instruction that appears before the instruction
at the instruction index obtained from the debug report.
This OpLine then contains the correct filename id, line number, and column number of the
statement causing the error.
The filename itself is obtained by scanning the SPIR-V again for an OpString instruction that
matches the id from the OpLine.
This OpString contains the text string representing the filename.
This information is added to the validation error message.

For online compilation when there is no "file", only the line number information is reported.

#### OpSource Processing

The SPIR-V built with source-level debug info also contains OpSource instructions that
have a string containing the source code, delimited by newlines.
Due to possible pre-processing, the layer just cannot simply use the source file line number
from the OpLine to index into this set of source code lines.

Instead, the correct source code line is found by first locating the "#line" directive in the
source that specifies a line number closest to and less than the source line number reported
by the OpLine located in the previous step.
The correct "#line" directive must also match its filename, if specified,
with the filename from the OpLine.

Then the difference between the "#line" line number and the OpLine line number is added
to the place where the "#line" was found to locate the actual line of source, which is
then added to the validation error message.

For example, if the OpLine line number is 15, and there is a "#line 10" on line 40
in the OpSource source, then line 45 in the OpSource contains the correct source line.

### Shader Instrumentation Input Record Format

Although the input buffer is a linear array of unsigned integers, conceptually there are arrays within the linear array

Word 1 starts an array (denoted by sets_to_sizes) that is number_of_sets long, with an index that indicates the start of that set's entries in the sizes array

After the sets_to_sizes array is the sizes array, that contains the array size (or 1 if descriptor is not an array) of each descriptor in the set.  Bindings with no descriptor are filled in with zeros

After the sizes array is the sets_to_bindings array that for each descriptor set, indexes into the bindings_to_written array.  Word 0 contains the index that is the start of the sets_to_bindings array

After the sets_to_bindings array, is the bindings_to_written array that for each binding in the set, indexes to the start of that binding's entries in the written array

Lastly comes the written array, which indicates whether a given binding / array element has been written

Example:
```
Assume Descriptor Set 0 looks like:                        And Descriptor Set 1 looks like:
  Binding                                                    Binding
     0          Array[3]                                       2          Array[4]
     1          Non Array                                      3          Array[5]
     3          Array[2]

Here is what the input buffer should look like:

   Index of                     sets_to_sizes                     sizes             sets_to_bindings                       bindings_to_written    written
   sets_to_bindings

     0 |11| sets_to_bindings     1 |3| set 0 sizes start at 3     3  |3| S0B0       11 |13| set 0 bindings start at 13        13 |21| S0B0        21 |1| S0B0I0 was written
            starts at 11         2 |7| set 1 sizes start at 7     4  |1| S0B1       12 |17| set 1 bindings start at 17        14 |24| S0B1        22 |1| S0B0I1 was written
                                                                  5  |0| S0B2                                                 15 |0 | S0B2        23 |1| S0B0I3 was written
                                                                  6  |2| S0B3                                                 16 |25| S0B3        24 |1| S0B1 was written
                                                                  7  |0| S1B0                                                 17 |0 | S1B0        25 |1| S0B3I0 was written
                                                                  8  |0| S1B1                                                 18 |0 | S1B1        26 |1| S0B3I1 was written
                                                                  9  |4| S1B2                                                 19 |27| S1B2        27 |0| S1B2I0 was not written
                                                                  10 |5| S1B3                                                 20 |31| S1B3        28 |1| S1B2I1 was written
                                                                                                                                                  29 |1| S1B2I2 was written
                                                                                                                                                  30 |1| S1B2I3 was written
                                                                                                                                                  31 |1| S1B3I0 was written
                                                                                                                                                  32 |1| S1B3I1 was written
                                                                                                                                                  33 |1| S1B3I2 was written
                                                                                                                                                  34 |1| S1B3I3 was written
                                                                                                                                                  35 |1| S1B3I4 was written
```
Alternately, you could describe the array size and write state data as:
(set = s, binding = b, index = i) is not initialized if 
```
Input[ i + Input[ b + Input[ s + Input[ Input[0] ] ] ] ] == 0
```
and the array's size = Input[ Input[ s + 1 ] + b ] 

## GPU-Assisted Validation Testing

Validation Layer Tests (VLTs) exist for GPU-Assisted Validation.
They cannot be run with the "mock ICD" in headless CI environments because they need to
actually execute shaders.
But they are still useful to run on real devices to check for regressions.

There isn't anything else that remarkable or different about these tests.
They activate GPU-Assisted Validation via the programmatic
interface as described earlier.

The tests exercise the extraction of source code information when the shader
is built with debug info.
