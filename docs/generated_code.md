# Generated Code

There is a lot of code generated in `layers/generated/`. This is done to prevent errors forgetting to add support for new
values when the Vulkan Headers or SPIR-V Grammer is updated.

How to generate the code:

```bash
python3 scripts/generate_source.py external/Vulkan-Headers/registry/ external/SPIRV-Headers/include/spirv/unified1/
```

When making change to the `scripts/` folder, make sure to run `generate_source.py` and check in both the changes to
`scripts/` and `layers/generated/` in any PR.

## Cmake helper

A helper CMake target `VulkanVL_generated_source` is also provided to simplify
the invocation of `scripts/generate_source.py` from the build directory:

```bash
cmake --build . --target VulkanVL_generated_source
```

## How it works

`generate_source.py` sets up the environment and then calls into `lvl_genvk.py` where each file is generated at a time. Many of the generation scripts will generate both the `.cpp` source and `.h` header

The Vulkan code is generated from the [vk.xml](https://github.com/KhronosGroup/Vulkan-Headers/blob/main/registry/vk.xml) and uses the python helper functions in the `Vulkan-Headers/registry` folder.

The SPIR-V code is generated from the [SPIR-V Grammer](https://github.com/KhronosGroup/SPIRV-Headers/blob/main/include/spirv/unified1/spirv.core.grammar.json)

## Tips

If only dealing with a single file, comment out all the other file names in `scripts/generate_source.py` to speed up testing iterations.
