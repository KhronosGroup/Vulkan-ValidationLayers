# Vulkan Validation Layers (VVL)

## Introduction

Vulkan is an Explicit API, enabling direct control over how GPUs actually work. By design, minimal error checking is done inside
a Vulkan driver. Applications have full control and responsibility for correct operation. Any errors in
how Vulkan is used can result in a crash. This project provides Vulkan validation layers that can be enabled
to assist development by enabling developers to verify their applications correct use of the Vulkan API.

## Community Assistance

Before submitting an issue to the validation layers or reaching out to the developers it may be prudent to reach out to the community first.
These resources can be helpful to refine your issue, work out an application/driver bug, etc.

- Discord: https://discord.com/invite/vulkan
- Reddit: https://www.reddit.com/r/vulkan
- Stackoverflow: https://stackoverflow.com/questions/tagged/vulkan
- Slack: https://khr.io/slack

## Contact Information
* @KhronosGroup/VVL-CODEOWNERS
* VVL-CODEOWNERS can also be found on aforementioned Slack channel for direct contact.

## Info
* [BUILD.md](BUILD.md) - Instructions for building the Validation Layers
* [KHRONOS_VALIDATION_LAYER.md](docs/khronos_validation_layer.md) - Instructions for configuring the Validation Layers
* [CONTRIBUTING.md](CONTRIBUTING.md) - Information needed to make a contribution.
    * [./docs](./docs/) - Details of the Validation Layer source code. **For those wanting to make contributions**
    * [./tests](./tests) - Information about testing the Validation Layers.
    * [GOVERNANCE.md](GOVERNANCE.md) - Repository management details.

## Version Tagging Scheme

Updates to this repository which correspond to a new Vulkan specification release are tagged using the following format: `v<`_`version`_`>` (e.g., `v1.3.266`).

**Note**: Marked version releases have undergone thorough testing but do not imply the same quality level as SDK tags. SDK tags follow the `vulkan-sdk-<`_`version`_`>.<`_`patch`_`>` format (e.g., `vulkan-sdk-1.3.266.0`).

This scheme was adopted following the `1.3.266` Vulkan specification release.

## License
This work is released as open source under a Apache-style license from Khronos including a Khronos copyright.

See [LICENSE.txt](LICENSE.txt) for a full list of licenses used in this repository.

## Acknowledgements
While this project has been developed primarily by LunarG, Inc., there are many other
companies and individuals making this possible: Valve Corporation, funding
project development; Google providing significant contributions to the validation layers;
Khronos providing oversight and hosting of the project.
