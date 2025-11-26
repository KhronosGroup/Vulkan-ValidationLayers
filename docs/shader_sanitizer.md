# Shader Sanitizer

Most SPIR-V validation can be divided into two groups, `Standalone` and `Runtime`.

**Standalone** are things caught in `spirv-val` because it's invalid SPIR-V. For example, trying to use a `OpTypeFloat` in your `OpUDiv` integer divide.

**Runtime** are things that need API information. For example, using an instruction without the required Vulkan feature.

There is a 3rd group that, while small, doesn't quite fall into neither. A simple example is dividing an integer by zero. The [spirv-v spec](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#OpUDiv) says

> Behavior is undefined if Operand 2 is 0.

`spirv-val` [won't validate](https://github.com/KhronosGroup/SPIRV-Tools/pull/6428) this, even if using a constant, because SPIR-V is valid and might not be ran. It doesn't make sense to mark this as **Runtime** in the sense it has nothing to do with the Vulkan API.

For these, we use the term `Sanitizer`!

The idea of Shader Sanitization is a way to group these types of checks that need to be done in VVL. They all have 2 parts:

1. In core validation we will check cases where using `OpConstant`, we can detect potential issues and raise a warning.
2. In GPU-AV we will inject a check and report an error if, and only if, the invalid code is actually executed.