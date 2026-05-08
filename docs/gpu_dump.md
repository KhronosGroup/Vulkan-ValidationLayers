# GPU Dump

**⚠️ This was released in the 1.4.350 SDK as a preview. The goal is for the July 2026 SDK to contain the final version, so please be aware that details may change. ⚠️**

As more extensions move towards the GPU, the objective is to ensure tools like [GPU-Assisted Validation](./gpu_validation.md) are utilized.

In reality, every tool occupies a specific spot on the debugging spectrum. A tool designed to simply "print out" GPU information was missing, so `GPU Dump` was added.

The goal is to provide a tool similar to API Dump, but with several key differences:

- **State Tracking:** It provides all addresses and GPU information that VVL maintains from its state tracking.
- **Conciseness:** While API Dump is verbose, this tool only prints the commands relevant to the user.
- **Data Capture:** It captures the data of the `VkBuffer` at the moment the command buffer is recorded.
    - While it is still possible to update these values later on the CPU or GPU, the tool targets the 90% of use cases that do not involve that level of indirection.

The options for all extensions are available in **VkConfig** (and are also available as a preset).

## Ways to use GPU Dump

### Supplemental Validation

Currently validation won't attempt to report errors where it is not 100% sure. Ultimately it is valid to update the memory on the GPU prior to a given draw/dispatch call. This will require a much more expensive GPU-AV in order to validate these GPU centric extension.

If you only plan to update the memory once on the CPU, than validation could be catching these issues for you already!

The idea is to turn on `GPU Dump` (with `warning`, not `info` level messages) on top of "normal core validation". From here `GPU Dump` will only report a `warning` when it detect a possible issue. This will provide a good portion of GPU-AV level checks, without needing to pay the performance cost of GPU-AV.

### Wanting all the information

Turn on GPU Dump, make sure to enable both `warning` and `info` level messages, and all the info will be printed.

## Options

There is currently a `VK_LAYER_GPU_DUMP_TO_STDOUT` option to allow printing directly to `stdout` incase you don't want to use the debug callback.

We are very happy to hear from people of additional options they feel would be helpful!

## Example

Using the "GPU Dump Descriptor" [setting](./settings.md):

```bash
export VK_LAYER_GPU_DUMP_DESCRIPTORS=1
# optional
export VK_LAYER_GPU_DUMP_TO_STDOUT=1
```

When performing a draw with `VK_EXT_descriptor_heap`, you will see output such as:

```
Validation Warning: [ GPU-DUMP ] | MessageID = 0xe5c5edc1
vkCmdDispatch(): [Dump Descriptor] (VkCommandBuffer 0x56e87348ea70, VkPipeline 0xb000000000b)
vkCmdBindResourceHeapEXT last bound the resource heap to [0x300000000, 0x300000100) (no reserved range)
  - VkBuffer 0x30000000003, size: 256 bytes, range: [0x300000000, 0x300000100)
[VK_SHADER_STAGE_COMPUTE_BIT]
  - Set 0:
    - Binding 0, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT (from pMappings[0])
      - heapOffset: 0x0, heapArrayStride: 0x40
        Resource Heap address: 0x300000000 + (descriptor_index * 0x40)
            The final descriptor index at [8] will access [0x3000001c0, 0x300000200)
        Descriptor size: 0x40 (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
        [WARNING] OUT OF BOUNDS - descriptor has an array length of [8] but any element accessed starting at [4] will be invalid if accessed


Validation Information: [ GPU-DUMP ] | MessageID = 0xe5c5edc1
vkCmdDispatch(): [Dump Descriptor] (VkCommandBuffer 0x56e87348ea70, VkPipeline 0xe000000000e)
vkCmdBindResourceHeapEXT last bound the resource heap to [0x300000000, 0x300000100) (no reserved range)
  - VkBuffer 0x30000000003, size: 256 bytes, range: [0x300000000, 0x300000100)
[VK_SHADER_STAGE_COMPUTE_BIT]
  - Set 0:
    - Binding 0, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT (from pMappings[0])
      - heapOffset: 0x0, heapArrayStride: 0x40
        Resource Heap address: 0x300000000 + (descriptor_index * 0x40)
            The final descriptor index in bounds is [3] which would be accessed at [0x3000000c0, 0x300000100)
        Descriptor size: 0x40 (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
```

In this example, the first dispatch shader looked like this:

```glsl
layout (set = 0, binding = 0) buffer SSBO_0 {
    uint data;
} ssbo[8];
```

GPU Dump prints the mapping of where the heap will be read from, but it also issues a `warning`:

> [WARNING] OUT OF BOUNDS - descriptor has an array length of [8] but any element accessed starting at [4] will be invalid if accessed

This helps catch errors where, if the user accesses `ssbo[4]`, it will actually be out of bounds of the bound descriptor heap memory.

The second dispatch has a shader that looks like this:

```
layout (set = 0, binding = 0) buffer SSBO_0 {
    uint data;
} ssbo[];
```

GPU Dump will help print the specific range that is valid for your runtime descriptor arrays.