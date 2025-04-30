<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2025 Valve Corporation -->
<!-- Copyright 2015-2025 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# GPU Assisted Validation

While most validation can be done on the CPU, some things like the content of a buffer or how a shader invocation accesses a descriptor array cannot be known until a command buffer is executed. The Validation Layers have a dedicated tool to perform validation in those cases: **GPU Assisted Validation** (aka GPU-AV).

GPU-AV directly inspects GPU resources and instrument shaders to validate their run time invocations, and reports back any error. Due to dedicated state tracking, shader instrumentation, necessity to read back data from the GPU, etc, there is a performance overhead, so **GPU-AV is off by default**.

## How to use

For an overview of how to configure layers, refer to the [Layers Overview and Configuration](https://vulkan.lunarg.com/doc/sdk/latest/windows/layer_configuration.html) document.

The GPU-AV settings are managed by configuring the Validation Layer. These settings are described in the [VK_LAYER_KHRONOS_validation](https://vulkan.lunarg.com/doc/sdk/latest/windows/khronos_validation_layer.html#user-content-layer-details) document.

GPU-AV settings can also be managed using the [Vulkan Configurator](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) included with the Vulkan SDK.

> Note - it is **highly** recommended to not have both normal Core validation and GPU-AV on together as performance will be slow.

## Requirements

There are several limitations that may impede the operation of GPU Assisted Validation:

- Vulkan 1.1+ required
- A Descriptor Slot
    - GPU-AV requires one descriptor set, which means if an application is using all sets in `VkPhysicalDeviceLimits::maxBoundDescriptorSets`, GPU-AV will not work.
    - There is a `VK_LAYER_GPUAV_RESERVE_BINDING_SLOT` that will reduce `maxBoundDescriptorSets` by one if the app uses that value to determine its logic.
- `fragmentStoresAndAtomics` and `vertexPipelineStoresAndAtomics` so we can write out information from those stages.
- `timelineSemaphore` so we don't have a big `vkQueueWaitIdle` after each submission.

There are various other feature requirements, but if not met, GPU-AV will turn off the parts of GPU-AV for you automatically (and produce a warning message).

If a feature is not enabled, we will try to enable it for you at device creation time.

## Reality

There are 5 goals we constantly think about when developing GPU-AV

1. It is fast enough to use
3. It won't crash on you
2. It is accurate (no false positive!)
4. It has good error messages
5. It actually catches your invalid code

The development of GPU-AV has taught us showed that to perfectly validate your GPU workload, it can be painfully slow (like multiple seconds a frame slow).

There are 2 main reasons people use GPU-AV: `regression mode` and `debug mode`

### Regression Mode

If you find yourself turning on GPU-AV all the time to "make sure no hidden issues" then this is you. You likely want to have things fast and can compromise not having have try and track multiple error in a single shader.

**By default**, we assume this is the use case and why we have the "Safe Mode" setting turned **off** by default.

By assuming things "should likely be working", we can make GPU-AV much faster

### Debug Mode

We realize if we don't stop your Device Lost, no one else will. If you are stuck on a nasty bug and need the extra help, this is for you.

**Please turn off Safe Mode**, this will sacrifice performance, but GPU-AV will try to stop things from crashing. We have a way to [select the bad shader](./gpu_av_selective_shader.md) as this will **greatly** improve performance if we only need to validate a smaller surface area.

#### Force on Robustness

There is also a "Force robustness on" setting we provide in GPU-AV, this has 2 main purposes

1. A way for developers to toggle this on and off
2. Improve GPU-AV performance by assuming the developer relies on [robust behavior to work](https://docs.vulkan.org/guide/latest/robustness.html)

## Internal Details

The state of GPU-AV is constantly evolving as we find out what does and doesn't work.

The following are extra information around GPU-AV for those who want to know:

- [General development advice](./gpu_av_development.md)
- [How descriptor indexing works](./gpu_av_descriptor_indexing.md)
- [How post processing works](./gpu_av_post_process.md)
- [How shader instrumentation works](./gpu_av_shader_instrumentation.md)