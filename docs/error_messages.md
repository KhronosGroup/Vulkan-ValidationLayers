# Error Message Overview

> Note: This is written for the 1.4.309 SDK and afterwards, the older versions of the Validation Layers will be slightly different

When an error message is found, the Validation Layers use the "debug callback" mechanism to report the error message.

By default, the Validation Layers will format the message, but it is possible for the user to provide their own `PFN_vkDebugUtilsMessengerCallbackEXT` and parse the string in `VkDebugUtilsMessengerCallbackDataEXT::pMessage` ([working example using vk-bootstrap](https://github.com/charles-lunarg/vk-bootstrap/blob/main/example/custom_debug_callback.cpp#L9-L20)).

The default error message will look like:

> Validation Error: [ VUID-vkCmdSetScissor-x-00595 ] Objects: VkCommandBuffer 0x639cdf870510[command_buffer_name] | MessageID = 0xa54a6ff8
>
> vkCmdSetScissor(): pScissors[0].offset.x (-1) is negative.
>
> The Vulkan spec states: The x and y members of offset member of any element of pScissors must be greater than or equal to 0 (https://docs.vulkan.org/spec/latest/chapters/fragops.html#VUID-vkCmdSetScissor-x-00595)

Here we see a few things, listed in the order they appear

1. The message severity
    - (error, warning, etc)
2. The [VUID](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/validation_overview.adoc#valid-usage-id-vuid)
3. List of Objects
    - Contain handle type, hex value, and optional debug util name
4. Message ID
    - This is just a hash of the VUID used internally to detect if the message duplication rate was hit
5. Which function the error occured at
6. The faulty parameter / struct member
    - Helps if there is an array to know which index it is in
7. The "main message"
    - This is normally hand written by the Validation Layer developer
8. The Vulkan spec langague for this VUID
9. Link to the VUID

# Message Formatting Options

There are a few ways to adjust the message format.

## Verbose

By default, the error messages are verbose, but if you turn off the setting the new error messages will look like

> [ VUID-vkCmdSetScissor-x-00595 ] vkCmdSetScissor(): pScissors[0].offset.x (-1) is negative.
>
> The Vulkan spec states: The x and y members of offset member of any element of pScissors must be greater than or equal to 0

To try it out, use `vkconfig` or turn off verbose with

```bash
# Windows
set VK_LAYER_MESSAGE_FORMAT_VERBOSE=0

# Linux
export VK_LAYER_MESSAGE_FORMAT_VERBOSE=0

# Android
adb setprop debug.vulkan.khronos_validation.message_format_verbose=0
```

## JSON

There is a way to print the error message as JSON to be machine readable, here we document the schema used:

> All fields are always printed, an empty string will be given if no value is provided

```json
{
	"Severity" : "string",
	"VUID" : "string",
	"Objects" : [
		{"type" : "string", "handle" : "string", "name" : "string"},
		...
	],
	"MessageID" : "string",
	"Function" : "string",
	"Location" : "string",
	"MainMessage" : "string", // Newline '\n' will be inlined here
	"DebugRegion" : "string",
	"SpecText" : "string",
	"SpecUrl" : "string"
}
```

Here is an example of what it looks like

```json
{
	"Severity" : "Error",
	"VUID" : "VUID-vkCmdSetScissor-x-00595",
	"Objects" : [
		{"type" : "VkCommandBuffer", "handle" : "0x618497491590", "name" : "command_buffer_name"},
	],
	"MessageID" : "0xa54a6ff8",
	"Function" : "vkCmdSetScissor",
	"Location" : "pScissors[0].offset.x",
	"MainMessage" : "(-1) is negative.",
	"DebugRegion" : "",
	"SpecText" : "The x and y members of offset member of any element of pScissors must be greater than or equal to 0",
	"SpecUrl" : "https://docs.vulkan.org/spec/latest/chapters/fragops.html#VUID-vkCmdSetScissor-x-00595"
}
```

> On Android, we will print it all in a single line to work better with `logcat`.

To try it out, use `vkconfig` or set with

```bash
# Windows
set VK_LAYER_MESSAGE_FORMAT_JSON=1

# Linux
export VK_LAYER_MESSAGE_FORMAT_JSON=1

# Android
adb setprop debug.vulkan.khronos_validation.message_format_json=1
```
