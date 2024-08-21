# Limitations

While the Validation Layers thrive to catch all undefined behavior, because the checking is done at a Vulkan Layer level, not everything is possible to validate.

This documentation is not served as an "errata" or a "list of all known limitations", but rather just a general outline so people are aware there are some limitations.

## Unimplementable Validation

The Validation Layers use the [VUID](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/validation_overview.adoc#valid-usage-id-vuid) to know what validation is [covered](https://vulkan.lunarg.com/doc/sdk/latest/windows/validation_error_database.html). There are times, for various reasons, we are unable to validate the VUID, but don't want it to marked as "yet to do" as there is nothing for us to actually do. The [unimplementable_validation.h](../layers/error_message/unimplementable_validation.h) file was created to list those.

This file breaks down all VUID with a reason why the Validation Layers are not able to actually validate them.

## Source Code

There are things that are invalid, that could only be caught by the source language.

### pNext and sType

If an app is trying to use `VkBindImageMemoryInfo`, but is setting the `sType` as `VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO` (mixed up the buffer and image name), this might cause issues.

The Validation Layers (as well as any other layer or driver) uses the `sType` to know how to cast the `void* pNext`, so if it is wrong, there is no way a Vulkan Layer could know.

There are VUID such as `VUID-VkBufferCreateInfo-pNext-pNext` that limit which `sType` can appear in the pNext chain, but something the 2 `sType` being mixed up both might be valid to use in that case.

### Dereferencing Pointers

There are VUs that will be worded something like `must be a valid pointer to ...`.

The Validation Layers will check the pointer is not null, but if the pointer is pointing to garbage, there is no way to safely dereference it.