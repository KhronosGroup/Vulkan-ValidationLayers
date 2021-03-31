<!-- markdownlint-disable MD041 -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Thread Safety Validation

The thread safety validation object checks multi-threading of API calls for validity.  Checks performed
include ensuring that only one thread at a time uses an object in free-threaded API calls.
