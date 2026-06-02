# GPU-AV Descriptor Heap

[Background to read prior to reading this](https://docs.vulkan.org/guide/latest/descriptor_heap.html)

Descriptor Heaps (`VK_EXT_descriptor_heap`) add a whole set of challenges for GPU-AV, and this document walks through the design decisions made.

# Global indirect buffer

Originally, we tried to store our internal descriptors in the heap, but we found that you are allowed to use `VK_EXT_descriptor_heap` without ever binding the heap! (See tests like `NegativeDebugPrintf.DescriptorHeapPushConstantOnly`)

Instead, we decided to use `VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT` and just have the indirect buffer be our lookup table.

Each time we do a draw/dispatch, we update the address to our bindings.

![gpu_av_descriptor_heap_mapping](images/gpu_av_descriptor_heap_mapping.png)