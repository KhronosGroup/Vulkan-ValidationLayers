# GPU Dump

As more extensions move towards the GPU, the objective is to ensure tools like [GPU-Assisted Validation](./gpu_validation.md) are utilized.

In reality, every tool occupies a specific spot on the debugging spectrum. A tool designed to simply "print out" GPU information was missing, so `GPU Dump` was added.

The goal is to provide a tool similar to API Dump, but with several key differences:

- **State Tracking:** It provides all addresses and GPU information that VVL maintains from its state tracking.
- **Conciseness:** While API Dump is verbose, this tool only prints the commands relevant to the user.
- **Data Capture:** It captures the data of the `VkBuffer` at the moment the command buffer is recorded.

The options for all extensions are available in **VkConfig** (and are also available as a preset).

# Goals

It is very valid to update the memory after recording a command buffer, this means GPU Dump will never be 100% accurate, that will require a more expensive GPU-AV to get that information.

The goal of GPU Dump was to provide a fast/lite tool that can still provide a lot of **useful** information.

It can be viewed as a glorified `printf` aimmed at extension, such as `VK_EXT_descriptor_heap`, where there is a lot of information to keep track of.

# Use GPU Dump as supplemental validation

GPU Dump will **never** give an error, but it will give both `warning` and `info` level messages

By turning off `info`, but leaving on `warning` you can use GPU Dump to provide warnings where it noticed at command buffer recording time a potential issue will occur.

## Wanting all the information

If both both `warning` and `info` level messages are on, all the info will be printed.

# Supported extensions

- `VK_EXT_descriptor_heap`
    - `VK_LAYER_GPU_DUMP_DESCRIPTORS`/`gpu_dump_descriptors`
- `VK_EXT_descriptor_buffer`
    - `VK_LAYER_GPU_DUMP_DESCRIPTORS`/`gpu_dump_descriptors`
- `VK_KHR_copy_memory_indirect`
    - `VK_LAYER_GPU_DUMP_COPY_MEMORY_INDIRECT`/`gpu_dump_copy_memory_indirect`
- `VK_EXT_device_generated_commands`
    - `VK_LAYER_GPU_DUMP_DEVICE_GENERATED_COMMANDS`/`gpu_dump_device_generated_commands`

# Options

There is currently a `VK_LAYER_GPU_DUMP_TO_STDOUT` option to allow printing directly to `stdout` incase you don't want to use the debug callback.

We are very happy to hear from people of additional options they feel would be helpful!

# Example

Using the "GPU Dump Descriptor" [setting](./settings.md):

```bash
export VK_LAYER_GPU_DUMP_DESCRIPTORS=1
# optional
export VK_LAYER_GPU_DUMP_TO_STDOUT=1
```

In this example, the dispatch shader will look this:

```glsl
layout (set = 0, binding = 0) buffer SSBO_0 {
    uint data;
} ssbo[8];
```

When performing a draw with `VK_EXT_descriptor_heap`, you will see output such as:

```
Validation Warning: [ GPU-DUMP ] | MessageID = 0xe5c5edc1
vkCmdDispatch(): [Dump Descriptor] (VkCommandBuffer 0x5ae561283160, VkPipeline 0x1e000000001e)
- vkCmdBindResourceHeapEXT last bound the resource heap to [0x300000000, 0x300000100) (no reserved range)
    - VkBuffer 0x30000000003, size: 256 bytes, range: [0x300000000, 0x300000100)
- Last bound pipeline: VkPipeline 0x1e000000001e (VK_PIPELINE_BIND_POINT_COMPUTE)
- Shader descriptors:
  [VK_SHADER_STAGE_COMPUTE_BIT] VkShaderModule 0x1c000000001c
  - SPIR-V Set 0:
    - SPIR-V Binding 0 ["ssbo"]
      - specified in pMappings[0] - VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT
      - heapOffset: 0x0, heapArrayStride: 64
      - Resource Heap address: 0x300000000 + (descriptor_index * 64)
            The final descriptor index at [8] will access [0x3000001c0, 0x300000200)
      - Descriptor size: 64 (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
      - [WARNING] OUT OF BOUNDS - descriptor has an array length of [8] but any element accessed starting at [4] will be OOB of the heap and invalid if accessed
```

GPU Dump prints the mapping of where the heap will be read from, but it also issues a `warning`:

> [WARNING] OUT OF BOUNDS - descriptor has an array length of [8] but any element accessed starting at [4] will be OOB of the heap and invalid if accessed

This helps catch errors where, if the user accesses `ssbo[4]`, it will actually be out of bounds of the bound descriptor heap memory.

This time if we update it to be a runtime descriptor array such as

```
layout (set = 0, binding = 0) buffer SSBO_0 {
    uint data;
} ssbo[];
```

GPU Dump will help print the specific range that is valid for your runtime descriptor arrays.

```
Validation Information: [ GPU-DUMP ] | MessageID = 0xe5c5edc1
vkCmdDispatch(): [Dump Descriptor] (VkCommandBuffer 0x5ae561283160, VkPipeline 0xc000000000c)
- vkCmdBindResourceHeapEXT last bound the resource heap to [0x300000000, 0x300000100) (no reserved range)
    - VkBuffer 0x30000000003, size: 256 bytes, range: [0x300000000, 0x300000100)
- Last bound pipeline: VkPipeline 0xc000000000c (VK_PIPELINE_BIND_POINT_COMPUTE)
- Shader descriptors:
  [VK_SHADER_STAGE_COMPUTE_BIT] VkShaderModule 0xa000000000a
  - SPIR-V Set 0:
    - SPIR-V Binding 0 ["ssbo"]
      - specified in pMappings[0] - VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT
      - heapOffset: 0x0, heapArrayStride: 64
      - Resource Heap address: 0x300000000 + (descriptor_index * 64)
            The final descriptor index at [3] is the last index in bounds of the heap buffer and will access [0x3000000c0, 0x300000100)
      - Descriptor size: 64 (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
```