# Vulkan Ecosystem Components

This project provides the Khronos official Vulkan validation layers for Windows, Linux, Android, and MacOS.

## master to main upcoming change (January 23, 2023)

See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5084 for details.

## CI Build Status
| Build Status |
|:------------|
| [![Build Status](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions/workflows/build_windows.yml/badge.svg?branch=master)](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions) |
| [![Build Status](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions/workflows/build_linux.yml/badge.svg?branch=master)](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions) |
| [![Build Status](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions/workflows/build_macos.yml/badge.svg?branch=master)](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions) |
| [![Build Status](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions/workflows/build_linux_gn.yml/badge.svg?branch=master)](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions) |
| [![Build Status](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions/workflows/build_android.yml/badge.svg?branch=master)](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions) |


## Introduction

Vulkan is an Explicit API, enabling direct control over how GPUs actually work. By design, minimal error checking is done inside
a Vulkan driver. Applications have full control and responsibility for correct operation. Any errors in
how Vulkan is used can result in a crash. This project provides Vulkan validation layers that can be enabled
to assist development by enabling developers to verify their applications correct use of the Vulkan API.

This repository contains both the [*Validation Layers*](layers/) source as well as [*Tests*](tests/) for them.

## Contact Information
* @KhronosGroup/VVL-CODEOWNERS
* VVL-CODEOWNERS members can also be found on the [Khronos Slack](https://khr.io/slack)

## Info
* [BUILD.md](BUILD.md) - Instructions for building the Validation Layers
* [LAYER_CONFIGURATION.md](LAYER_CONFIGURATION.md) - Instructions for configuring the Validation Layers at runtime.
* [CONTRIBUTING.md](CONTRIBUTING.md) - Information needed to make a contribution.
    * [./docs](./docs/) - Details of the Validation Layer source code. **For those wanting to make contributions**
    * [./tests](./tests) - Information about testing the Validation Layers.
    * [GOVERNANCE.md](GOVERNANCE.md) - Repository management details.

## Version Tagging Scheme

Updates to the `Vulkan-ValidationLayers` repository which correspond to a new Vulkan specification release are tagged using the following format: `v<`_`version`_`>` (e.g., `v1.1.96`).

**Note**: Marked version releases have undergone thorough testing but do not imply the same quality level as SDK tags. SDK tags follow the `sdk-<`_`version`_`>.<`_`patch`_`>` format (e.g., `sdk-1.1.92.0`).

This scheme was adopted following the 1.1.96 Vulkan specification release.

## License
This work is released as open source under a Apache-style license from Khronos including a Khronos copyright.

See [LICENSE.txt](LICENSE.txt) for a full list of licenses used in this repository.

## Acknowledgements
While this project has been developed primarily by LunarG, Inc., there are many other
companies and individuals making this possible: Valve Corporation, funding
project development; Google providing significant contributions to the validation layers;
Khronos providing oversight and hosting of the project.
