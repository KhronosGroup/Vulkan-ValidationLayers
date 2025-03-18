# GPU-AV Post Process

The goal behind Post Process is to have a very lightweight way to get information from the GPU so that it can be used on the CPU.

In order to be lightweight, Post Process makes assumptions that it can run the GPU without crashing/hanging.

## Descriptor Indexing

For descriptor indexing we use Post Process to do the same validation on the CPU, but one at draw time and one after queue submission time.

Take the following two descriptors in a GLSL shader:

```glsl
layout(set = 0, binding = 0) buffer SSBO_0 { int data; } descriptor_a;
layout(set = 0, binding = 1) buffer SSBO_1 { int data; } descriptor_b[];
```

Normally we will validate `descriptor_a` at draw time as the Vulkan spec requires any static usage to be valid.

Once we start having bindless (or dynamic array) of descriptor, it is important to only check the index accessed. If `descriptor_b` has a million items, but only two are accessed, we use GPU-AV Post Process to detect that.

We create a buffer that hold information about each descriptor and when the GPU is ran, we quickly just mark what has been access. The instrumented GLSL would look like

```glsl
inst_post_process_descriptor_index(0 /*set*/, 1 /*binding*/, index);
descriptor_b[index].data = 0;
```

## Reusing the same code twice

Currently, we have a `DescriptorValidator` class that is created at draw time with Core Checks and also at post-queue submission with GPU-AV.

The goal is that the `ValidateDescriptor()` function can be called the same from both variations. To call `ValidateDescriptor()` we need 2 things:

1. Which descriptor was accessed
2. The `OpVariable` from the SPIR-V that it was accessed in

## Tracking Descriptors and SPIR-V

The `VkDescriptorSet` object contains a buffer with a `PostProcessDescriptorIndexSlot` for each descriptor in it. This "descriptor index slot" holds information that can only be known at GPU execution time (which index, OpVariable, etc... are used). Each time `vkCmdBindDescriptorSets` is called, we create a LUT mapping of 32 (current max limit) addresses to map to each `VkDescriptorSet` bound and its buffer.

The pipelines contain the `OpVariable` SPIR-V information, such as which type of image is used. This is wrapped up in a `BindingVariableMap` pointer. Using this we can map the accessed descriptor on the GPU back to all the state tracking on the CPU.

The following diagram aims to illustrate the flow of data:

![alt_text](images/post_processing.svg "Post Processing")

When processing on the CPU, we loop through every unique `VkDescriptorSet` to look at the contents of its `PostProcessDescriptorIndexSlot`. It contains both the instrumented `unqiue shader ID` and the `OpVariable` ID. Just like other GPU-AV checks, we can use the `unqiue shader ID` to lookup which `VkShaderModule`/`VkPipeline`/`VkShaderEXT` that reported the error.