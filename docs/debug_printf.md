<!-- markdownlint-disable MD041 -->
<!-- Copyright 2020-2026 LunarG, Inc. -->
<!-- Copyright 2020-2026 Valve Corporation -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Using Debug Printf

## Introduction

This is an overview of how to use C-style `printf` statements in shaders for real-time inspection. It is currently supported in GLSL, HLSL, and Slang, and is **simple** to add to your debugging workflow right now!

## Quick Overview of Debug Printf

Without going into too much detail, this is all possible because of the interface provided by the [VK_KHR_shader_non_semantic_info](https://github.com/KhronosGroup/SPIRV-Guide/blob/main/chapters/nonsemantic.md) extension, which was promoted to Vulkan 1.3.

To get Debug Printf to work you need 2 things:

1. A way to add the printf to the shader (which GLSL/HLSL/Slang provide)
2. An implementation to consume the printf statement and print it out for you

The two main implementations are the Validation Layers and RenderDoc.

## Turning on Debug Printf in the Validation Layers

We suggest using Vulkan Configurator (`VkConfig`) to enable Debug Printf, using the `Debug Printf Only` preset.

For those who "just need to use it quickly" there is a `VK_LAYER_PRINTF_ONLY_PRESET` environment variable (Vulkan SDK 1.4.304 or later) that will turn on Debug Printf and turn off all of the other validation logic.

```bash
# Windows
set VK_LAYER_PRINTF_ONLY_PRESET=1

# Linux
export VK_LAYER_PRINTF_ONLY_PRESET=1

# Android
adb shell setprop debug.vulkan.khronos_validation.printf_only_preset=1
```

Note this will print to the debug callback and you might want it sent directly to `stdout` instead.

```bash
# Optional - will print to `stdout` **instead** of the normal Debug Callback
export VK_LAYER_PRINTF_TO_STDOUT=1
```

### Turn on with other validation

The `VK_LAYER_PRINTF_ENABLE=1` environment variable (`printf_enable` for people using `VK_EXT_layer_settings`) will turn on Debug Printf alongside the other validation.

## Settings

There are a few additional Debug Printf settings

> All settings are also found in `VkConfig`

- `VK_LAYER_PRINTF_TO_STDOUT`
    - Print the messages directly to `stdout` instead of the debug callback
    - `VK_LAYER_PRINTF_TO_STDOUT=1` (env variable)
    - `printf_to_stdout` (`VK_EXT_layer_settings`)
- `VK_LAYER_PRINTF_VERBOSE`
    - Will print extra information (pipeline, shader, command, etc.)
    - `VK_LAYER_PRINTF_VERBOSE=1` (env variable)
    - `printf_verbose` (`VK_EXT_layer_settings`)
- `VK_LAYER_PRINTF_BUFFER_SIZE`
    - Set the size in bytes of the buffer used to hold the messages. A separate buffer of this size is used for each draw/dispatch/trace rays command that uses a shader containing Debug Printf.
    - Each message takes 40 bytes plus 4 bytes for each 32-bit value (8 bytes for each 64-bit value) being printed
    - If the buffer fills up, the remaining messages are dropped and a warning is printed with the size that would have been needed
    - Default: 1024 bytes
    - `VK_LAYER_PRINTF_BUFFER_SIZE=4096` (env variable)
    - `printf_buffer_size` (`VK_EXT_layer_settings`)

## Using Debug Printf in GLSL Shaders

To use Debug Printf in GLSL shaders, you need to enable the `GL_EXT_debug_printf` extension.
Then add `debugPrintfEXT()` calls at the locations in your shader where you want to print
messages and/or values.
Here is a very simple example ([Try Online](https://godbolt.org/z/MnYGj8azM)):

```glsl
#version 450
#extension GL_EXT_debug_printf : enable
void main() {
    float myfloat = 3.1415f;
    debugPrintfEXT("My float is %f", myfloat);
}
```

`glslang` will automatically add the Debug Printf instructions.

## Using Debug Printf in HLSL and Slang Shaders

In HLSL and Slang, Debug Printf can be invoked as follows ([Try Online](https://godbolt.org/z/3ThznsdK8)):

```hlsl
void main() {
    float myfloat = 3.1415;
    printf("My float is %f", myfloat);
}
```

Both `dxc` and `slangc` will automatically add the Debug Printf instructions.

## Recommendations

If you print every time a shader is executed you can easily get millions of things trying to print. It is recommended to use built-ins to limit what is printed.

```glsl
// Vertex Shader
if (gl_VertexIndex == 0) {
    debugPrintfEXT("Only print for a single vertex shader invocation\n");
}

// Fragment Shader
if (gl_FragCoord.x > 0.0 && gl_FragCoord.x < 0.1 &&
    gl_FragCoord.y > 0.0 && gl_FragCoord.y < 0.1) {
    debugPrintfEXT("Only print for a few fragment shader invocations\n");
}

// Compute Shader
if (gl_LocalInvocationIndex == 0) {
    debugPrintfEXT("Only print for a single compute invocation\n");
}
```

## Debug Printf Output

Debug Printf messages are returned as `VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT`.

For your custom callback, you can check for `VVL-DEBUG-PRINTF` in `VkDebugUtilsMessengerCallbackDataEXT::pMessageIdName`, or its hash `0x4fe1fef9` in `VkDebugUtilsMessengerCallbackDataEXT::messageIdNumber`, to know if it is a Debug Printf message.

When Debug Printf is enabled (and not printing to `stdout`), the Validation Layers will turn on `info` level messages so the message is not filtered out.

The `VkDebugUtilsMessengerCallbackDataEXT::pMessage` will contain the location and then, on a new line, the printed message, such as:

> vkQueueSubmit(): pSubmits[0] DebugPrintf:
>
> x == 100

## Debug Printf Format String

The format string for this implementation of Debug Printf is more restricted than the traditional `printf` format string.

The format for a specifier is `%`[`0`][*width*][`.`*precision*]*specifier*, where *specifier* is one of:

| Specifier | Type | Notes |
| --- | --- | --- |
| `d`, `i` | signed int | |
| `u`, `o`, `x`, `X` | unsigned int | |
| `a`, `A`, `e`, `E`, `f`, `F`, `g`, `G` | float | Use these for 16, 32, and 64-bit floats (**not** `%lf`) |
| `lu` | 64-bit unsigned int | Printed in decimal |
| `lx`, `ul` | 64-bit unsigned int | Printed in hex (`ul` is a non-standard alias of `lx`) |
| `ld` | 64-bit signed int | Printed in decimal |
| `p` | pointer (`PhysicalStorageBuffer`) | |

The format for a **vector** specifier is `%`[`0`][*width*][`.`*precision*]`v`[`2`, `3`, or `4`]*specifier*, where *specifier* is one from the list above (except `p`), for example `%v4f` or `%1.2v3f`.

- The vector value separator is `", "`
- `%%` will print as `%`
- *width* and *precision* behave like the C `printf`, so `%8x` pads to 8 characters, `%08x` pads with zeros, and `%1.2f` prints 2 digits after the decimal point
- The only flag supported is `0`. The `-`, `+`, ` ` (space), and `#` flags are not supported
- `*` for *width* or *precision* is not supported
- The only length modifier is `l`, and only in the 64-bit forms listed above (no `h`, `hh`, `ll`, `j`, `z`, etc.). 8-bit and 16-bit integers and 16-bit floats are printed with the normal 32-bit specifiers
- Booleans are printed as `0` or `1` with an integer specifier
- No strings or characters (`%s` and `%c`) allowed

The Validation Layers check the format string against the arguments when the shader is instrumented, and report any problem as an error or warning with the `DEBUG-PRINTF-FORMATTING` message ID. If an error is found (such as an unknown specifier, a vector specifier with a non-vector argument, or fewer arguments than specifiers), that Debug Printf call is skipped and will print nothing.

For example:

```glsl
float myfloat = 3.1415f;
vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);
uint64_t bigvar = 0x2000000000000001ul;
uint myuint = 42;
```

`debugPrintfEXT("Here's a float value to 2 decimals %1.2f", myfloat);`
Would print **"Here's a float value to 2 decimals 3.14"**

`debugPrintfEXT("Here's a vector of floats %1.2v4f", floatvec);`
Would print **"Here's a vector of floats 1.20, 2.20, 3.20, 4.20"**

`debugPrintfEXT("Unsigned long as decimal %lu and as hex 0x%lx", bigvar, bigvar);`
Would print **"Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001"**

`debugPrintfEXT("Padded [%8u] and zero padded hex [%08x]", myuint, myuint);`
Would print **"Padded [      42] and zero padded hex [0000002a]"**

## Debug Printf messages in RenderDoc

As of RenderDoc release 1.14, Debug Printf statements can be added to shaders, and Debug
Printf messages will be received and logged in the Event Browser window.

Using the debugmarker sample from Sascha Willems' Vulkan samples repository:

1. Capture a frame:
![Rd Frame](images/rd_frame.png)

2. Edit the shader:
- Add `#extension GL_EXT_debug_printf : enable` to the beginning of the shader
- Add `debugPrintfEXT("Position = %v4f", pos);` to the shader after the `pos` definition
- Hit Refresh

![Refresh](images/refresh.png)

The vkCmdDrawIndexed in question now has 51 messages.

3. Click on msg(s) to see Debug Printf output per draw:
![Values](images/values.png)

## Using Debug Printf in SPIR-V Shaders

Normally, developers will use a high-level language like HLSL or GLSL to generate SPIR-V.
However, in some cases, developers may wish to insert Debug Printfs directly into SPIR-V.

To execute Debug Printf in a SPIR-V shader, a developer will need the following two
instructions specified:

```
OpExtension "SPV_KHR_non_semantic_info"
%N0 = OpExtInstImport "NonSemantic.DebugPrintf"
```

Debug Printf operations can then be specified in any function with the following instruction:
`%NN = OpExtInst %void %N0 1 %N1 %N2 %N3` ...
where:
* `N0` is the result id of the `OpExtInstImport "NonSemantic.DebugPrintf"`
* `1` is the opcode of the Debug Printf instruction in `NonSemantic.DebugPrintf`
* `N1` is the result id of an `OpString` containing the format for the Debug Printf
* `N2`, `N3`, ... are result ids of scalar and vector values to be printed
* `NN` is the result id of the Debug Printf operation. This value is undefined.

> `OpExtInstImport` of any `NonSemantic.*` instruction set is properly supported with the `VK_KHR_shader_non_semantic_info` device extension. Some older compiler stacks might not handle these unknown instructions well, while others will ignore them as desired.

## Debug Printf messages from Validation Layers via VkConfig (Vulkan Configurator)

Here's an example of adding a Debug Printf statement to the shader in the `vkcube` demo (from
the `Vulkan-Tools` repository), and then using `VkConfig` to enable Debug Printf, launch vkcube,
and see the Debug Printf output.

1. Add Debug Printf to the vkcube demo:
 - Add `VK_KHR_shader_non_semantic_info` to cube's `CreateDevice` function (not needed with newer Validation Layers, which enable it for you)
 - Add the extension and a `debugPrintfEXT` call to the shader
 - Use `glslangValidator` to compile the new shader
 - (Offscreen) Rebuild vkcube

![Add Dbpf](images/add_dbpf.png)

2. Configure `VkConfig` to enable Debug Printf
 - Select the Debug Printf preset
 - Set the executable path to the vkcube demo and add `--c 1` to the command line to render one frame
 - Click the "Launch" button

![Vkconfig Setup](images/vkconfig_setup.png)

3. See the Debug Printf output in the Launcher window:
![Vkconfig Result](images/vkconfig_result.png)

## Limitations

* Debug Printf consumes a descriptor set (the last one, at `maxBoundDescriptorSets - 1`). If your application uses every last
descriptor set on the GPU, Debug Printf will not work.
* Debug Printf consumes device memory on the GPU. Large or numerous Debug Printf
messages can exhaust device memory.
  * Can be controlled with `VK_LAYER_PRINTF_BUFFER_SIZE` (see [Settings](#settings))
* Validation Layers version: `1.2.135.0` or later is required
* Vulkan API version 1.1 or greater is required
* When using the Validation Layers, the `fragmentStoresAndAtomics`, `vertexPipelineStoresAndAtomics`, and `timelineSemaphore` features are required
  * The Validation Layers will enable these for you if the device supports them
  * Printing a 64-bit float also requires the `shaderInt64` feature (also enabled for you if supported)
* The `VK_KHR_shader_non_semantic_info` extension should be supported
  * The Validation Layers will enable it for you if the device supports it. If it is not supported, the Validation Layers strip the Debug Printf instructions out of the instrumented shader to allow a wider range of users to still use Debug Printf
* RenderDoc release 1.14 or later

## Other References

Documentation for the GL_EXT_debug_printf extension can be found
[here](https://github.com/KhronosGroup/GLSL/blob/main/extensions/ext/GLSL_EXT_debug_printf.txt).

There are many Validation Layer tests that demonstrate the simple and programmatic use of Debug
Printf. See `tests/unit/debug_printf.cpp` in the Vulkan-ValidationLayers repository.

Earlier implementations implicitly included stage-specific built-in variables such as `gl_InvocationID`, `gl_VertexID` and `gl_FragCoord` in Debug Printf messages. This functionality has been removed because it made Debug Printf unusable in shader modules that defined entry points for multiple pipeline stages. If necessary, you can add these values to your printf statements explicitly. However, you must then make sure that the printf statement can only be executed from a pipeline stage where the built-in variable is available.
