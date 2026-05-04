# GPU Dump

**⚠️ This was released in 1.4.350 SDK as a preview and the goal is the July 2026 SDK will have the final version, so warning things may change. ⚠️**

As more extensions move towards the GPU the goal is tools like[GPU-Assisted Validation](./gpu_validation.md) will be used.

The reality is each tool fits on the spectrum for debugging, and something to just "print out" the GPU information was missing, so `GPU Dump` was added.

The goal is something like API Dump but:

- Provide all the addresses and GPU information that VVL has from all our state tracking.
- API Dump is verbose, this only print the commands we care about.
- Grab the data of the `VkBuffer` at that moment of recording the command buffer.
    - While it is still very possible to update these values later on the CPU or even on the GPU, the goal is 90% of use cases are not doing that level of indirection.

The options for all the extensions are available on vkconfig (as a preset as well).

## Example

Using the "GPU Dump Descriptor" setting:

```bash
export VK_LAYER_GPU_DUMP_DESCRIPTORS=1
# optional
export VK_LAYER_GPU_DUMP_TO_STDOUT=1
```

When doing a draw with `VK_EXT_descriptor_heap` you will see something such as

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

In this example, the first dispatch shader looked like:

```glsl
layout (set = 0, binding = 0) buffer SSBO_0 {
    uint data;
} ssbo[8];
```

GPU Dump prints out the mapping where in the heap it will be read from, but also prints as `warning level`

> [WARNING] OUT OF BOUNDS - descriptor has an array length of [8] but any element accessed starting at [4] will be invalid if accessed

To help catch that if the user goes `ssbo[4]` it will actually be out of bounds of the bound descriptor heap memory.

The second dispatch has a shader that looks like:

```
layout (set = 0, binding = 0) buffer SSBO_0 {
    uint data;
} ssbo[];
```

GPU Dump will help print out the range that is valid to use your runtime descriptor arrays.