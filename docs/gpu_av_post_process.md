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
inst_post_process_descriptor_index(0 /*binding*/, 1 /*binding*/, index);
descriptor_b[index].data = 0;
```

## Reusing the same code twice

Currently, we have a `DescriptorValidator` class that is created at draw time with Core Checks and also at post-queue submission with GPU-AV.

The goal is that the `ValidateDescriptor()` function can be called the same from both variations.

## Tracking Descriptors and SPIR-V

When tracking data for Post Process, we need both information from `vkCmdBindDescriptorSets` and `vkCmdBindPipeline` (or shader objects), but not all are present at the same time.

At action command time we need to bind a `VkBuffer` that contains the Buffer Device Address to each descriptor set. If we have 4 descriptor set and the following command buffer stream:

```c++
vkCmdBindDescriptorSets(firstSet = 0, setCount = 2, [set_a, set_b]);
vkCmdDraw()
vkCmdBindDescriptorSets(firstSet = 3, setCount = 1, [set_a]);
vkCmdDraw()
```

the first draw the `VkBuffer` contents would look like

```
[&set_a, &set_b, null, null]
   |        |
   |      [descriptor_0, descriptor_1, etc]
   |
 [descriptor_0, descriptor_1, etc]
```

and the second draw would look like

```
[&set_a, &set_b, null, &set_a]
   |        |            |
   |        -------------|------[descriptor_0, descriptor_1, etc]
   |                     |
 [descriptor_0, descriptor_1, etc]
```

With this information, we now need to pair that up with the SPIR-V information. This information is saved in a `BindingVariableMap` pointer. This contains a mapping between each binding in the descriptor set, and what each `OpVariable` tied to it looks like.