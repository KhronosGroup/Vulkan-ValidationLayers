<!-- markdownlint-disable MD041 -->
<!-- Copyright 2021-2022 LunarG, Inc. -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Fine Grained Locking

The Vulkan-ValidationLayer code has been updated to not globally lock every Vulkan call. Waiting on this lock causes performance problems for multi-threaded applications, and most Vulkan games are heavily multi-threaded.  This feature has been tested with 15+ released games and improves performance in almost all of them, and many improve by about 150%.

However, changes to locking strategy in a multi threaded program are a frequent cause of crashes, incorrect results, or deadlock. For the first release of this feature, it is off by default and can be enabled using the instructions below. Once the new locking strategy has been proven to not cause stability problems, it will be on by default and then can be turned off, if necessary for debugging.

Currently, this Fine Grained Locking is only available for Core Validation.  Best Practices, Synchronization Validation, GPU Assisted Validation and Debug Printf still always run with global locking. Support for these types of validation will be added in a future release.

Thread Safety, Object Lifetime, Handle Wrapping and Stateless validation have always avoided global locking and they are thus unaffected by this feature.

Fine grained locking can easily be enabled and configured using the [Vulkan Configurator](https://vulkan.lunarg.com/doc/sdk/latest/windows/vkconfig.html) included with the Vulkan SDK. You can also manually enable it following instructions below.



## Enabling Fine Grained Locking

This feature is disabled by default. To turn it on, add the following to your layer settings file,
`vk_layer_settings.txt`:

```code
khronos_validation.fine_grained_locking = true
```

To enable using environment variables, set the following variable:

```code
VK_LAYER_FINE_GRAINED_LOCKING=1
```

### Known Limitations

Currently there is not a way to enable this setting via `VK_EXT_validation_features` or other programmatic interface. This will be addressed in a future release.

The locking in Vulkan-ValidationLayer is not intended to provide correct results for programs that violate the thread safety guidelines described in *section 3.6 Threading Behavior* of the Vulkan specification. We will attempt to fix crash bugs in the layer resulting from insufficient external synchronization, but incorrect or inconsistent error messages will be likely. Please run Thread Safety violation to find problems like this.


### Debugging Tips

As mentioned above, solving all problems found by Thread Safety validation is highly recommended before trying to use Core Validation with Fine Grained Locking. 

If you encounter a crash, deadlock or incorrect behavior, re-run with Fine Grained Locking disabled. If your problem goes away, it is most likely a problem with the layer's locking code. If it remains, then it is probably caused by something else and will require further debugging.

For crashes or deadlocks, it will be extremely helpful you provide stack traces for any threads in your program that were executing in the layer at the time the problem occurred.  In Microsoft Visual Studio, the [Parallel Stacks](https://docs.microsoft.com/en-us/visualstudio/debugger/using-the-parallel-stacks-window?view=vs-2022) window is a good way to check the status of all threads in your program.  With [gdb](https://sourceware.org/gdb/current/onlinedocs/gdb/Threads.html#Threads), you usually need to use the `info thread` and `thread` commands to view the stacks from each thread.

Replay tools such as `gfxrecon` are not always helpful when debugging problems in multi-threaded code. Because these tools usually need to serialize all vulkan commands into a single stream, they will not be able to recreate interactions between threads during replay.  Often, the only way to recreate problems is to rerun the program, but sometimes changes in timing may cause problems to only happen on some types of hardware.

Please file a [Github issue](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues) if you need help or have feedback on this feature. The more information you can provide about what was happening, the better we will be able to help!
