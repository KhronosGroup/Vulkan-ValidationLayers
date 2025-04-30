# GPU-AV Development Guide

For those brave souls, here are some overall basic tips/advice that should not get outdated as things actively change

## You need to Validate with "Self Valiation"

Since GPU-AV itself utilizes the Vulkan API to perform its tasks,
Vulkan function calls have to valid. To ensure that, those calls have to
go through another instance of the Vulkan Validation Layer. We refer to this
as "self validation".

How to setup self validation:
- Build the self validation layer:
    - Make sure to use a Release build
        - Otherwise might be really slow with double validation
    - Use the the `-DBUILD_SELF_VVL=ON` cmake option when generating the CMake project
        - The build will produce a manifest file used by the Vulkan loader, `VkLayer_dev_self_validation.json`.
        The `name` field in this file is `VK_LAYER_DEV_self_validation` to differentiate the self validation layer from the one you work on.
            - If the name were the same, the loader/os would mark both layers as duplicates and not load the second instance
- Then use it:
    - you need to ask the loader to load the self validation layer, and tell it where to find it.
        Do this by modifying the `VK_INSTANCE_LAYERS` and `VK_LAYER_PATH`, like so for instance:
```bash
# Windows
VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation;VK_LAYER_DEV_self_validation
VK_LAYER_PATH=C:\Path\To\Vulkan-ValidationLayers\build\debug\layers\Debug;C:\Path\To\Vulkan-ValidationLayers\build_self_vvl\layers\Release

# Linux
VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation:VK_LAYER_DEV_self_validation
VK_LAYER_PATH=/Path/To/Vulkan-ValidationLayers/build/debug/layers/Debug:/Path/To/Vulkan-ValidationLayers/build_self_vvl/layers/Release
```

⚠️ Make sure to load the self validation layer **after** the validation layer you work on, by putting its name in `VK_INSTANCE_LAYERS` after the validation layer you work on. Otherwise your Vulkan calls will not be intercepted by the self validation layer.
To make sure you did it properly, you can use the environment variable `VK_LOADER_DEBUG=layer` to see how the loader sets up layers.

## We generate our SPIR-V offline

There is a `scripts/generate_spirv.py` that will take GLSL (maybe Slang in the future) and creates SPIR-V blobs baked in a C++ file. [We have more details here](../layers/gpuav/shaders/README.md)

