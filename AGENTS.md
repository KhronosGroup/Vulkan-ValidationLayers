# Instructions for AI agents

These apply when you file an issue or open a PR on
[Vulkan-ValidationLayers](https://github.com/KhronosGroup/Vulkan-ValidationLayers).
See also [CONTRIBUTING.md](./CONTRIBUTING.md), [tests/README.md](./tests/README.md) and
[docs/creating_tests.md](./docs/creating_tests.md).

## Filing issues

Use the `Bug report` issue template. First work out where the issue belongs.

### Pick the right repo

- The message comes from `spirv-val` (the SPIR-V validator)? Report it at
  [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools/issues) instead.
- The layer does exactly what the spec says, but the spec is wrong? Report it at
  [Vulkan-Docs](https://github.com/KhronosGroup/Vulkan-Docs/issues) instead.
- Nothing crashes validation but something should be disallowed and **no VUID exists** for it? Report it at
  [Vulkan-Docs](https://github.com/KhronosGroup/Vulkan-Docs/issues), asking what should not be allowed.
  A crash can be a driver bug that validation was never meant to catch, which is often why there is no VUID.

### What to include, by type of issue

**False positive** (the layer reports an error that should not be reported)

- The full error message you got.
- A spec reference that shows why it is a false positive.
- A **positive test** in `tests/unit/` that reproduces it.

**Missing validation** (the layer reports nothing when it should)

- The VUID in the spec that should have caught it.
- A **negative test** in `tests/unit/` that shows the error that should have been reported.

**Crash or assert**

- A **positive test** that reproduces it **without** `SetAllowedFailureMsg`.
- If validation already reports an error earlier (e.g. an invalid image from `vkCreateImage` that is later
  used while recording a command buffer), that earlier error has to be fixed first. The layers do not
  re-validate every object at every use. Quick checks such as a null pointer may still be accepted.

**Memory leak or slow performance**

- A VVL unit test that reproduces it.

### For every issue

- Unit tests are the best way to report. They let us reproduce and debug quickly.
- State the OS, the GPU (and driver) you ran on, and the VVL version (SDK version, or commit if built from source).
- State any non-default settings (e.g. synchronization validation, GPU-AV). With none listed we assume the
  default (core validation only).
- Keep the summary short and simple. The test shows most of what is going on. A wall of text will almost never
  be read.
- Do not point at random lines of the layer source. The test is what makes it easy to debug.
- If you did not actually run the test against the layers, say so.
- Do not invent VUIDs, spec text, or root causes. Say "unknown" instead.

## Opening PRs

- Match the style of the surrounding code, and run `clang-format` (see [CONTRIBUTING.md](./CONTRIBUTING.md)).
- Comments should be helpful: explain why, not what the code obviously does.
- Do not add a test just for the sake of adding a test. Every test must be meaningful.
- Do not add defensive changes (for example a `uint64_t` cast) unless you can provide a test that reaches the
  problem (for example the 32-bit overflow) without hitting other errors and without `SetAllowedFailureMsg`.
