# Error Message Overview

> Note: This is written for the 1.4.309 SDK and afterwards, the older versions of the Validation Layers will be slightly different

When an error message is found, the Validation Layers use the "debug callback" mechanism (`PFN_vkDebugUtilsMessengerCallbackEXT`) to report the error message.

There are 2 options

1. Use the default callback (provided by the validation layers)
2. Use a custom callback (more details below)

By default, the Validation Layers will format the `VkDebugUtilsMessengerCallbackDataEXT` struct for you and print to the following to the OS-dependent standard output (`stdout`, `logcat`, `OutputDebugString`, etc):

> Validation Error: [ VUID-vkCmdSetScissor-firstScissor-00593 ] | MessageID = 0x603fac6b
>
> vkCmdSetScissor(): firstScissor is 1 but the multiViewport feature was not enabled.
>
> The Vulkan spec states: If the multiViewport feature is not enabled, firstScissor must be 0 (https://docs.vulkan.org/spec/latest/chapters/fragops.html#VUID-vkCmdSetScissor-firstScissor-00593)
>
> Objects: 1
>    [0] VkCommandBuffer 0x5c11921d2d10
>
> (new line)

Here we see a few things, listed in the order they appear

1. The message severity (error, warning, etc)
2. The [VUID](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/validation_overview.adoc#valid-usage-id-vuid)
3. Message ID
    - This is just a hash of the VUID used internally to detect if the message duplication rate was hit
4. Which function the error occured at
5. The faulty parameter / struct member
    - Helps if there is an array to know which index it is in
6. The "main message"
    - This is normally hand written by the Validation Layer developer
7. The Vulkan spec langague for this VUID
8. Link to the VUID
9. List of Objects
    - Contain handle type, hex value, and optional debug util name
10. There is a new line under to allow for easy seperation of multiple error messages

# Custom Callback

For those who want to handle the callback to format the message the way they want, here is how the `VkDebugUtilsMessengerCallbackDataEXT` is laid out

```c++
// VkDebugUtilsMessengerCallbackDataEXT
const char*                           pMessageIdName;  // VUID
int32_t                               messageIdNumber; // Hash of the VUID
const char*                           pMessage;        // The "main message" (includes the spec text on seperate line)

// Debug Objects that Validaiton will print for you in a default callback
uint32_t                              queueLabelCount;
const VkDebugUtilsLabelEXT*           pQueueLabels;
uint32_t                              cmdBufLabelCount;
const VkDebugUtilsLabelEXT*           pCmdBufLabels;
uint32_t                              objectCount;
const VkDebugUtilsObjectNameInfoEXT*  pObjects;
```

The following is an example of how one could do their custom callback

```c++
VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                       VkDebugUtilsMessageTypeFlagsEXT message_type,
                       const VkDebugUtilsMessengerCallbackDataEXT *callback_data) {

    // Other layers might also be trying to report via the callback, so can filter using the type
    bool is_validation = messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    // Only report Errors and Warnings
    bool is_error = messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    bool is_warning = messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    if (is_error || is_warning) {
        std::cout << "Validation " << (is_error ? "Error:" : "Warning:");
        std::cout << " [ " << pCallbackData->pMessageIdName << " ]\n";
        std::cout << pCallbackData->pMessage;

        for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
            std::cout << '\n';
            if (pCallbackData->pObjects[i].objectHandle) {
                std::cout << "    Object Handle [" << i << "] = " << " 0x" << std::hex << pCallbackData->pObjects[i].objectHandle;
            }
            if (pCallbackData->pObjects[i].pObjectName) {
                std::cout << "[" << pCallbackData->pObjects[i].pObjectName << "]";
            }
        }
    }
    std::cout << '\n';

    // Return false to move on and still call the function to the driver
    return VK_FALSE;
    // Return true to have validation skip passing down the call to the driver
    return VK_TRUE;
};
```

# JSON Error Format

For those who want something more machine readable, or don't want to deal with regex parsing, there is an option to output the error in JSON.

The string returned in `VkDebugUtilsMessengerCallbackDataEXT::pMessage` will be a valid JSON object.

The JSON schema:

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

# Logging to a file

By default the validation layers will try to print the error message out to `OutputDebugString` on Windows, `logcat` on Android, and `stdout` for Linux/MacOS.

You have the option to have the files write out to a file as well.

To try it out, use `vkconfig` or set with

```bash
# Windows
set VK_LAYER_DEBUG_ACTION=VK_DBG_LAYER_ACTION_LOG_MSG
set VK_LAYER_LOG_FILENAME=C:\vvl_errors.txt

# Linux
export VK_LAYER_DEBUG_ACTION=VK_DBG_LAYER_ACTION_LOG_MSG
export VK_LAYER_LOG_FILENAME=/tmp/vvl_errors.txt

# Android
adb setprop debug.vulkan.khronos_validation.debug_action=VK_DBG_LAYER_ACTION_LOG_MSG
adb setprop debug.vulkan.khronos_validation.log_filename=/data/local/tmp/vvl_errors.txt
```

# Additional information

Synchronization validation detects memory hazards and has custom error conventions. Additional information about SyncVal errors can be found in the [Synchronization Validation Messages](./syncval_usage.md#synchronization-validation-messages) document.
