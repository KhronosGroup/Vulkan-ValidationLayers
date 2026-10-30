# Limitations

While the Validation Layers thrive to catch all undefined behavior, because the checking is done at a Vulkan Layer level, it is not possible to validate everything.

This documentation is not an exhaustive list of known limitations, but rather a general outline so people are aware of some them.

## Unimplementable Validation

The Validation Layers use the [VUID](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/validation_overview.adoc#valid-usage-id-vuid) to know what validation is [covered](https://vulkan.lunarg.com/doc/sdk/latest/windows/validation_error_database.html). There are times where we are unable to validate some VUID, for various reasons. But we don't want to mark it as "yet to do" as there is nothing for us to actually do. The [unimplementable_validation.h](../layers/error_message/unimplementable_validation.h) file was created to list those. It lists all such VUIDs, and the reason why we cannot validate them.


## Source Code

There are things that are invalid, that can only be caught by the source language.

### pNext and sType

If an app is trying to use `VkBindImageMemoryInfo`, but is setting the `sType` as `VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO` (mixed up the buffer and image name), this might cause issues.

The Validation Layers (as well as any other layer or driver) uses the `sType` to know how to cast the `void* pNext`, so if it is wrong, there is no way a Vulkan Layer could know.

There are VUID such as `VUID-VkBufferCreateInfo-pNext-pNext` that limit which `sType` can appear in the pNext chain, but sometimes the 2 mixed up `sType` can both be valid by chance, and there will be no validation error.

### Dereferencing Pointers

There are VUs that will be worded something like `must be a valid pointer to ...`.

The Validation Layers will check that the pointer is not null, but if the pointer is pointing to garbage, there is no way to safely dereference it.

## stdlib exit()

It is possible for an app to call `exit()` and do cleanup in their `atexit()` callback.
Unfortunately, this will destroy our static allocation from under us and there is no way to detect it at runtime.
We register our own `atexit()` handler on the first `vkCreateInstance()` call to clean up the layer's dispatch objects.
Because `atexit()` handlers run in the reverse order of registration, an application handler registered **before** the first `vkCreateInstance()` will run **after** the layer has cleaned up, and any Vulkan call made from it will abort (with a message saying the `VkDevice` dispatch handle was not found).
If the application needs to make Vulkan calls in its `atexit()` handler, it must register it **after** the first `vkCreateInstance()`.

## Re-validating later in validation

Once the Validation Layers report an error, treat everything after it as **undefined behavior**. The application is in a bad state, and the layers do not promise to survive it.

The layers do not re-validate the same thing at every stage. Once an earlier stage has reported an error, the later stages assume the input is valid and are free to crash on it. For example, if `vkCreateImageView` reports an error and the app keeps going, a later crash in `vkCreateFramebuffer` when it uses that bad `VkImageView` is **not a bug** in the layers.

The Validation Layers make a **best effort** to remain stable after an error is reported, but this is **not guaranteed**. Handling every such case would add runtime cost and code complexity, and we do not sacrifice performance or maintainability to do so.

The **Stateless validation** catches basic issues (such as a null pointer, or an invalid enum or flag). This means thing such as `CoreChecks` should not need to redo these validation checks.

**CoreChecks** catches things that are invalid based on state. Things such as **GPU-AV** and **SyncVal** come next. They do not try redo these done previously, so in case of an error before in the validation stack, they may work on invalid input.

The Validation Layers for API command looks like:

1. Validate
2. Call the driver
3. State tracking

Sometimes there is no `VkResult` and a invalid call to the driver might not crash, but is still in a undefined behavior state. The state tracking here will not attempt to handle every invalid case that was already caught in the validation stage.

### Reporting these crashes

If a real world application hits one of these crashes, we are much more likely to look into a fix. Please report it, and tell us what the app was doing!

Crashes found only by scanning, fuzzing, or an ASAN sweep, where the usage is just theoretical, are a slippery slope. There is an unbounded number of these cases, and we do not plan to handle them. This is common with AI generated issues and PRs, so to avoid re-validating the same things over and over, we ignore these reports.

To report a crash, provide a test that reproduces it **without** `SetAllowedFailureMsg`. If the test needs `SetAllowedFailureMsg` to get to the crash, validation has already caught the invalid usage and the crash is expected.
