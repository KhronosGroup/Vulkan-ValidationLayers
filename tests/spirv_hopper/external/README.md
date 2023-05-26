# SPIRV-Reflect

These files are taken from https://github.com/KhronosGroup/SPIRV-Reflect

The reasons to check these in instead of adding to `known_good.json` are

- This repo is not updated often
- These files are designed for people to include in their build
- Ideally there will not need to be fixes once `SPIRV-Hopper` is working
    - We only use it so we don't have to re-write the same SPIR-V parsing logic

## Current state

The files were taken from `1fd43331f0bd77cc0f421745781f79a14d8f2bb1` commit with the following 3 PR cherry picked on top
- https://github.com/KhronosGroup/SPIRV-Reflect/pull/181
- https://github.com/KhronosGroup/SPIRV-Reflect/pull/182
- https://github.com/KhronosGroup/SPIRV-Reflect/pull/183