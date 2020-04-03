<!-- markdownlint-disable MD041 -->
<!-- Copyright 2020 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Debug Printf

[![Creative Commons][3]][4]

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/

Debug Printf is implemented in the SPIR-V Tools optimizer and the `VK_LAYER_KHRONOS_validation` layer.
It allows developers to debug their shader code by "printing" any values of interest to the debug callback or stdout.
This document covers the operation of the layer portion of the implementation.

## Basic Operation

The basic operation of Debug Printf is comprised of instrumenting shader code to return any values used in a debugPrintfEXT operation when the shader is executed.
The instrumentation is similiar to the process described in the GPU Assisted Validation documentation.
The debugPrintfEXT operations in the shader are replaced with code to copy the values to be printed to a buffer provided by the validation layer.
If the shader is executed without instrumentation, the driver will ignore all debugPrintfEXT operations.
After the shader is executed, the layer uses the values returned to construct a string and send the string in a message to the debug callback.

Note that the printf will generate a string each time the shader containing it is run.
A vertex shader running to draw a triangle will result in 3 messages from a single printf in the shader, unless care is taken in the shader to do otherwise.


## Enabling Debug Printf

Debug Printf is an object in the KHRONOS_validation layer, so the VK_LAYER_KHRONOS_validation layer must be loaded.
See the LAYER_CONFIGURATION document for information on enabling the VK_LAYER_KHRONOS_validation layer.
Validation itself is not necessary for Debug Printf and can be disabled without affecting Debug Printf functionality.

Debug Printf can be enabled either through a vk_layer_settings.txt file that must be in the program's working directory, or by setting an environment variable  Within a settings file, specify:
khronos_validation.enables = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT or set VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT as an environment variable.

Debug Printf has been implemented as a state tracker validation object, as a peer to GPU Assisted Validation.
Because of this, and coupled with the philosophy that validation objects will not communicate with each other, one should never enable both GPU Assisted Validation and Debug Printf at the same time.
Debug Printf will be disabled if GPU Assisted Validation is enabled.

The strings resulting from a Debug Printf will be sent to the debug callback which is either specified by the app, or by default sent to stdout.
It is sent at the INFO or DEBUG level

When using Debug Printf with the debug callback, it is recommended to disable validation, as the debug level of INFO or DEBUG causes the validation layers to produce many messages unrelated to Debug Printf, making it difficult to find the desired output.

### Debug Printf Requirements

* Validation Layers version: 1.2.135.0
* Vulkan API version 1.1 or greater
* VkPhysicalDevice features: fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics
* VK_KHR_shader_non_semantic_info extension supported and enabled

### Debug Printf Settings

* khronos_validation.printf_buffer_size =  size in bytes

This setting allows you to specify the size of the per draw buffer, in bytes of device memory, for returning Debug Printf values.
The default is 1024 bytes.
Each printf will require 32 bytes for header information and an additonal 4 bytes for each 32 bit value being printed, and an additional 8 bytes for each 64 bit value.
If printfs are truncated due to lack of memory, a warning will be sent to the Vulkan debug callback.

* khronos_validation.printf_verbose = 'false' or 'true'

The default value is 'false'. Verbose output will contain stage, shader id, line number, and other information in addition to the resulting string.

* khronos_validation.printf_to_stdout = 'false' or 'true'

By default, Debug Printf messages are sent to the debug callback, but this setting will instead send Debug Printf strings to stdout.
This can also be enabled by setting the environment variable DEBUG_PRINTF_TO_STDOUT.

### Debug Printf Format String

The format string for this implementation of debug printf is more restricted than the traditional printf format string.

Format for specifier is "%"*precision* <d, i, o, u, x, X, a, A, e, E, f, F, g, G, or ul>

Format for vector specifier is "%"*precision*"v" [2, 3, or 4] [specifiers list above]   

- The vector value separator is ", "
- "%%" will print as "%"
- No length modifiers.  Everything except ul is 32 bits, and ul values are printed only in hex
- No strings or characters allowed
- No flags or width specifications allowed
- No error checking for invalid format strings is done.

### Debug Printf Resources

Analogous to GPU Assisted Validation, Debug Printf uses device memory and a descriptor set to allow the shader instrumentation code to return values to the layer.  
See the gpu_validation document for more information

### Debug Printf Test / Example

There is a positive layer validation test that demonstrates simple use of Debug Printf.
It is called "GpuDebugPrintf" and is in vklayertests_gpu.cpp in the Vulkan-ValidationLayers repository.
