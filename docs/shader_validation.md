# Shader Validation

Shader validation is run at `vkCreateShaderModule` and `vkCreate*Pipelines` time. It makes sure both the `SPIR-V` is valid
as well as the `VkPipeline` object interface with the shader. Note, this is all done on the CPU and different than
[GPU-Assisted Validation](gpu_validation.md).

## Standalone VUs with spirv-val

There are many [VUID labeled](https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#spirvenv-module-validation-standalone) as `VUID-StandaloneSpirv-*` and all the
[Built-In Variables](https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#interfaces-builtin-variables)
VUIDs that can be validated on a single shader module and require no runtime information.

All of these validations are passed off to `spirv-val` in [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools/).

## spirv-opt

There are a few special places where `spirv-opt` is run to reduce recreating work already done in `SPIRV-Tools`.

## Different sections

The code is currently split up into the following main sections

- `layers/shader_instruction.cpp`
    - This contains information about each SPIR-V instruction.
- `layers/shader_module.cpp`
    - This contains information about the `VkShaderModule` object
- `layers/shader_validation.cpp`
    - This takes the following above and does the actual validation. All errors are produced here.
    - `layers/generated/spirv_validation_helper.cpp`
        - This is generated file provides a way to generate checks for things found in the `vk.xml` related to SPIR-V
- `layers/generated/spirv_grammar_helper.cpp`
    - This is a general util file that is [generated](generated_code.md) from the SPIR-V grammar