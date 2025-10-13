# GPU-AV Descriptor Buffer

[Background to read prior to reading this](https://docs.vulkan.org/guide/latest/descriptor_buffer.html)

Descriptor Buffers (`VK_EXT_descriptor_buffer`) adds a whole set of challenges for GPU-AV and this walks through the design decisions made.

## No SPIR-V changes

The one silver lining of Descriptor Buffers is they don't touch the SPIR-V at all, so there is no change to our shader instrumentation to support it.

## Properties to watchout for

The following `VkPhysicalDeviceDescriptorBufferPropertiesEXT` are worth keeping in mind as they shape how we need to think about adding GPU-AV.

- `maxResourceDescriptorBufferBindings`
  - Because this can be 1, we can't assume we can just create our own Descriptor Buffer to bind, we need to latch onto the user's buffer.
- `storageBufferDescriptorSize`
  - We know we want to inject SSBO, but we won't be able to call `vkGetDescriptorSetLayoutSizeEXT` before device creation time, so we might need to use this as an estimate how much memory we need to take for descriptors.
- `resourceDescriptorBufferAddressSpaceSize`
  - If we are going to consume some of the Descriptor Buffer, we need to make sure we adjust this so the user doesn't allocate more than what is allowed.
- `maxResourceDescriptorBufferRange`
  - This limit means the app could allocate a 2GB Descriptor Buffer, but if the max range is only 1GB, then we need to be cautious that it might not be able to see memory we added from the offset the user binds.
- `descriptorBufferOffsetAlignment`
  - However we do our offset, need to make sure they are aligned.

## Injecting our descriptors

The main first step to add support for GPU-AV/DebugPrintf is finding a way to inject our descriptors inside the Descriptor Buffer such that our shaders can access it.

### The core issues

There are some core problems that prevent use from easily adding GPU-AV (or even DebugPrintf) support

1. The Descriptor Buffer memory could be non host visible.

If we want to inject our descriptors, we want to be able to use `memcpy` or just point `vkGetDescriptorEXT` to the descriptor buffer directly. The issue occurs if the memory is not host visible. We would want to call `vkCmdCopyBuffer`, but that can't be called inside a render pass instance.

2. There is not "reserved memory" in the Descriptor Buffer.

Using small numbers, lets say the `resourceDescriptorBufferAddressSpaceSize` is 1024 bytes, but `maxResourceDescriptorBufferRange` is only 256 bytes.
In this case, the user in a command buffer might bind the offset at `0`, then `256`, then `512`, then `256` and `0` again. In this example, if we want to add our 64 bytes somewhere, we would need to keep track and replace the old memory.

3. Push Descriptor won't work unless to restrict user from using it.

The idea of Push Descriptors is one `pSetLayout` in your `VkPipelineLayout` can be "push descriptors". Instead of calling `vkCmdSetDescriptorBufferOffsetsEXT(set = x)` you just call `vkCmdPushDescriptorSetKHR(set = x)`. The advantage of this, we can just push when we want and fully ignore the other problems listed above.
The disadvantage of this, is we basically need to restrict users from using Push Descriptors with Descriptor Buffers now. Since we can only have a single `VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR` we need to use it. If the user uses it we run into 2 new problems.

- `descriptorBufferPushDescriptors` is now required (seems every GPU I checked does support it though!).
- We might hit the `maxPushDescriptors` limit.
- We can't control which `set`/`binding` which has already been baked into the instrumented shader code.

### The real solution

The sad answer is there is not going to be a single "magic bullet" here and we will either need to

1. Accept some apps won't be able to make use of `VK_EXT_descriptor_buffer` tooling
2. Have 2 internal ways to handle `VK_EXT_descriptor_buffer` depending on what the user does

### Trade off - Host Visible

The first trade off is around the Descriptor Buffer being host visible or not. We could

- Turn off GPU-AV/DebugPrintf if not host visible
  - Pro: Can always `memcpy`/`vkGetDescriptorEXT` our descriptor
  - Con: Apps might be forced to use slower memory to work with GPU-AV/DebugPrintf

### Trade off - Per draw fidelity

For Classic descriptors we use the dynamic offset in `vkCmdBindDescriptorSets` to mark on the GPU which draw we are at, which can't be used now.
`vkCmdCopyBuffer` was never used because the restriction of using it inside a render pass instance.

- Give up and accept all render pass draws are grouped together
  - Pro: Just use `vkCmdCopyBuffer` at top of a render pass and call it a day!
  - Con: All errors inside a render pass will be aweful to the user to know which draw to look at.

- Use Push Descriptors to set which draw
  - Pro: Easy to do and would would even work with classic descriptors as well if we wanted.
  - Con: From above, we would need to either restrict users from using Push Descriptor, or be willing to re-instrument at draw time to match the `set`/`binding` the user picks for Push Descriptors.

- Allocate all possible combination and copy (with `memcpy` or `vkCmdCopyBuffer`) them inside the Descriptor Buffer somewhere
  - Pro: We know all our descriptors are there and can use it to get the per-draw fidelity
  - Con: Will need somewhere between 64kb to maybe 1MB of memory to any buffer marked with `VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT`.
  - Con: We might need to constantly call `vkCmdBindDescriptorBuffersEXT` to make sure we can see the buffer. But if `maxResourceDescriptorBufferRange` is small, still might not be able to see

## What we decided

TODO :smile: