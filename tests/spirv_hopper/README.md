# SPIRV-Hopper

The goal of `SPIRV-Hopper` is to feed SPIR-V binaries into the Validation Layers

## How To Build

By default `SPIRV-Hopper` is turned on when building with the rest of the Validation Layers tests.

This can be turned off with `-D VVL_SPIRV_HOPPER=OFF` when building the tests.

## How To Run

```bash
./spirv-hopper file.spv

# or

./spirv-hopper folder/with/files/
```

## How it works

- Takes a SPIR-V file(s)
- Uses [SPIRV-Reflect](https://github.com/KhronosGroup/SPIRV-Reflect) to get the basic interface information
- Creates everything needed to make a valid `VkPipeline`
    - `VkPipelineLayout`
    - Descriptor Sets
    - Passthrough shaders
- `vkCreate*Pipelines()`

## Goals

This tool is designed to be simple, its main job is to help catch issues in Shader Validation by allowing us to run against MANY shaders quickly. Paired with a database of SPIR-V binaries from various SPIR-V generated tools (not just `glslang`), we can catch bugs/crashes at a per-commit testing level

Being able to execute the shaders is outside the scope of `SPIRV-Hopper`

## What about {Insert Testing Framework}

The idea behind `SPIRV-Hopper` is not to have to worry about creating a "testing file". This tool only wants to worry about the SPIR-V binary.

This is also faster because we can create a single `VkInstance`/`VkDevice` to run all the SPIR-V through

## Current limitations

### Only handles single Entry Point

For getting things up and running, only single Entry Points are tested.

A future goal is to take each entry point and run each as there own "sub-run"

### Single threaded

Currently `SPIRV-Hopper` is single-threaded. It would be nice to look into making it run multi-threaded.

### Need MockICD and Profiles

This is only designed to run with the [MockICD and Profiles layers](https://github.com/KhronosGroup/Vulkan-ValidationLayers/tree/main/tests#running-tests-on-mockicd-and-profiles-layer) (using [special profile designed for SPIRV-Hopper](../device_profiles/spirv_hopper_profile.json)).

The reasons to not use the [max_profile.json](../device_profiles/max_profile.json) instead are

- Some limits we want to be max `UINT32_T` for SPIRV-Hopper that we don't for normal testing
    - Because the tests need to be `limit + 1`
- Some features in `max_profile.json` are set to `false` so they can be tested
- This is smaller, more portable profile if there is a desire to decouple`SPIRV-Hopper` for VVL
