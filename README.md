# Vulkan Ecosystem Components

This project provides the Khronos official Vulkan validation layers for Windows, Linux, Android, and MacOS.

## CI Build Status
| Platform | Build Status |
|:--------:|:------------:|
| Linux/Android | [![Build Status](https://travis-ci.org/KhronosGroup/Vulkan-ValidationLayers.svg?branch=master)](https://travis-ci.org/KhronosGroup/Vulkan-ValidationLayers) |
| Windows |[![Build status](https://ci.appveyor.com/api/projects/status/8wduq4r5d9j8nhdv?svg=true)](https://ci.appveyor.com/project/Khronoswebmaster/vulkan-validationlayers/branch/master) |


## Introduction

Vulkan is an Explicit API, enabling direct control over how GPUs actually work. By design, minimal error checking is done inside
a Vulkan driver. Applications have full control and responsibility for correct operation. Any errors in
how Vulkan is used can result in a crash. This project provides Vulkan validation layers that can be enabled
to assist development by enabling developers to verify their applications correct use of the Vulkan API.

The following components are available in this repository:
- [*Validation Layers*](layers/)
- [*Tests*](tests/)

## Contact Information
* [Tobin Ehlis](mailto:tobine@google.com)
* [Mark Lobodzinski](mailto:mark@lunarg.com)

## Information for Developing or Contributing:

Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file in this repository for more details.
Please see the [GOVERNANCE.md](GOVERNANCE.md) file in this repository for repository management details.

## How to Build and Run

[BUILD.md](BUILD.md)
Includes directions for building all components as well as running validation tests.

Information on how to enable the various Validation layers is in
[layers/README.md](layers/README.md).

## Version Tagging Scheme

Updates to the `Vulkan-ValidationLayers` repository which correspond to a new Vulkan specification release are tagged using the following format: `v<`_`version`_`>` (e.g., `v1.1.96`).

**Note**: Marked version releases have undergone thorough testing but do not imply the same quality level as SDK tags. SDK tags follow the `sdk-<`_`version`_`>.<`_`patch`_`>` format (e.g., `sdk-1.1.92.0`).

This scheme was adopted following the 1.1.96 Vulkan specification release.

## License
This work is released as open source under a Apache-style license from Khronos including a Khronos copyright.

See COPYRIGHT.txt for a full list of licenses used in this repository.

## Acknowledgements
While this project has been developed primarily by LunarG, Inc., there are many other
companies and individuals making this possible: Valve Corporation, funding
project development; Google providing significant contributions to the validation layers;
Khronos providing oversight and hosting of the project.
