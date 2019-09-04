<!-- markdownlint-disable MD041 -->
<!-- Copyright 2015-2019 LunarG, Inc. -->

[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Core Validation Checks

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/
Implemented as part of the `VK_LAYER_KHRONOS_validation` layer,  this validation object is reponsiblie for validating
the status of descriptor sets, command buffers, shader modules, pipeline states, renderpass usage, synchronization,
dynamic states, and many other types of valid usage. It is the main module responsible for validation requiring
substantial background state.

This module validates that:

- the descriptor set state and pipeline state at each draw call are consistent
- pipelines are created correctly, known when used and bound at draw time
- descriptor sets are known and consist of valid types, formats, and layout
- descriptor set regions are valid, bound, and updated appropriately
- command buffers referenced are known and valid
- command sequencing for specific state dependencies and renderpass use is correct
- memory is available
- dynamic state is correctly set.

This validation object will print errors if validation checks are not correctly met, and provide context related to
the failures.

## Memory/Resource related functionality

This validation additionally attempts to ensure that memory objects are managed correctly by the application.
These memory objects may be bound to pipelines, objects, and command buffers, and then submitted to the GPU
for work. Specifically the layer validates that:

- the correct memory objects have been bound
- memory objects are specified correctly upon command buffer submittal
- only existing memory objects are referenced
- destroyed memory objects are not referenced
- the application has confirmed any memory objects to be reused or destroyed have been properly unbound
- checks texture formats and render target formats.

Errors will be printed if validation checks are not correctly met and warnings if improper (but not illegal) use of
memory is detected.  Validation also dumps all memory references and bindings for each operation.

## Shader validation functionality

Additional checks apply to the VS->FS and FS->CB interfaces with the pipeline.  These checks include:

- validating that all variables which are part of a shader interface are  decorated with either `spv::DecLocation` or `spv::DecBuiltin` (that is, only the SSO rendezvous-by-location model is supported)
- emitting a warning if a location is declared only in the producing stage (useless work is being done)
- emitting an error if a location is declared only in the consuming stage (garbage will be read).

A special error checking case invoked when the FS stage writes a built-in corresponding to the legacy `gl_FragColor`.  In this case, an error is emitted if

- the FS also writes any user-defined output
- the CB has any attachment with a `UINT` or `SINT` type.

These extra checks are to ensure that the legacy broadcast of `gl_FragColor` to all bound color attachments is well-defined.

## Swapchain validation functionality

This area of functionality validates the use of the WSI (Window System Integration) "swapchain" extensions (e.g., `VK_EXT_KHR_swapchain` and `VK_EXT_KHR_device_swapchain`).
