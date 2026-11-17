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

**Crash or assert or ASAN**

- A **positive test** that reproduces it **without** `SetAllowedFailureMsg`.
  - IMPORTANT! - See docs/limitations.md - If you are needing to use `SetAllowedFailureMsg` this is likely NOT
    an issue and you should ensure the user is aware before they waste the time of the maintainers.
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

## Building and testing

For a small patch you only need the layer and the unit tests. Full platform instructions are in
[BUILD.md](./BUILD.md) and [tests/README.md](./tests/README.md). Commands below work on Linux, macOS and Windows
unless marked otherwise. Run them from a regular shell. On Windows, `cmd`, PowerShell and Git Bash all work, and
only the environment variable syntax differs (shown in both forms).

### Build

```bash
cmake -S . -B build -D BUILD_WERROR=ON -D BUILD_TESTS=ON -D CMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j <N>     # leave a few cores free on shared machines
```

- Always pass `--config <Debug|Release>` to `cmake --build`. It is required for multi-config generators (Visual
  Studio, Xcode) and ignored by single-config ones (Ninja, Makefiles). `CMAKE_BUILD_TYPE` only applies to
  single-config generators.
- **Windows:** the default generator is the latest Visual Studio, which is fine. A Ninja build also works, but it
  must be run from an "x64 Native Tools Command Prompt" (or after `vcvars64.bat`) so the compiler is on `PATH`.
  Python 3 must be on `PATH` (build scripts and code generation).
- Configure once. Re-running `cmake` is only needed when CMake files or dependencies change. After editing
  source, just rebuild.
- Rebuild only what you need: `cmake --build build --config Debug --target vk_layer_validation_tests`
  (the layer is `VkLayer_khronos_validation`).
- Use a compiler cache if you have one (`-D CMAKE_CXX_COMPILER_LAUNCHER=ccache`, or `sccache` on Windows). The
  Visual Studio generators ignore the launcher, so this only helps with Ninja/Makefiles. The tests use a
  precompiled header, so with ccache set `CCACHE_SLOPPINESS=pch_defines,time_macros` or every test object is a
  cache miss. If the cache directory is read-only (sandboxes), point `CCACHE_DIR` at a writable one.
- Do not edit files in `layers/vulkan/generated/` by hand. Change the generator in `scripts/` and regenerate with
  `cmake -S . -B build -D VVL_CODEGEN=ON && cmake --build build --target vvl_codegen`
  (see [docs/generated_code.md](./docs/generated_code.md)).
- Dependency/header bumps (`build: Update to header 1.4.N`) need the matching `external/` headers installed
  before the layer compiles. See [BUILD.md](./BUILD.md#dependencies).
- Keep line endings as in the repo (LF). Do not let an editor or `core.autocrlf` rewrite whole files, and do not
  commit a diff that only touches line endings.

### Test

Tests are a single gtest binary, `vk_layer_validation_tests`. Only run the tests related to your change. The full
suite is slow, and CI runs it on the PR.

| Generator | Location of the binary |
| --- | --- |
| Ninja / Makefiles | `build/tests/vk_layer_validation_tests` |
| Visual Studio / Xcode | `build/tests/<Config>/vk_layer_validation_tests[.exe]` (e.g. `build\tests\Debug\`) |

If you cannot find it, search the build directory for it.

```bash
vk_layer_validation_tests --gtest_filter=NegativeBuffer.*
vk_layer_validation_tests --gtest_filter=*Descriptor*-*RayTracing*
vk_layer_validation_tests --gtest_filter=<Suite.Test> --print-vu
```

- Quote the filter in Bash (`'*Buffer*'`) so the shell does not expand `*`. In `cmd` and PowerShell, no quoting is needed.
- Tests are in `tests/unit/`, split by topic. Suites are `Negative<Topic>` (expects an error) and
  `Positive<Topic>` (must produce **no** errors). Find the existing file for your topic and add to it.
- `--print-vu` prints every message, including those the test expected. Use it to check new message wording.
- `--gtest_list_tests` shows what exists. `--device-index N` selects a `VkPhysicalDevice`.
- On Windows the tests find the freshly built layer automatically (`VK_LAYER_PATH` is set by the tests), as on Linux.

**Which Vulkan implementation to run on** (use what is available, and state which one you used):

| Driver | When to use | Notes |
| --- | --- | --- |
| Real GPU driver | Anything that needs real behavior: GPU-AV, DebugPrintf, ray tracing, sync, WSI | This is the default when no ICD variable is set. In a sandbox or container the GPU may not be visible. |
| Software rasterizer (Mesa lavapipe, SwiftShader) | Anything that does not need a GPU, second opinion on limits | Advertises far fewer limits and features than the mock ICD, so it catches tests that hardcode capabilities. Do **not** run ray tracing or GPU-AV tests on lavapipe. Point `VK_DRIVER_FILES` at its manifest. |
| VVL mock ICD + Profiles layer | Default CI path, deterministic, needs no GPU | Required for features no real driver exposes yet. Setup is below. |

**Mock ICD setup** (details in [tests/README.md](./tests/README.md#running-tests-on-test-driver-and-profiles-layer)).
The Profiles layer comes with the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) or can be built from
[Vulkan-Profiles](https://github.com/KhronosGroup/Vulkan-Profiles). Set `VVL` to your checkout, `BUILD` to the
directory holding the layer and ICD build output (on Windows with Visual Studio, `$VVL\build\layers\Debug` etc.),
and `PROFILES` to the directory with the profiles layer manifest.

Linux/macOS (Bash):
```bash
export VK_DRIVER_FILES=$VVL/build/tests/icd/VVL_Test_ICD.json     # VK_ICD_FILENAMES on older loaders
export VK_LAYER_PATH=$VVL/build/layers:$PROFILES                  # ':' separator
export LD_LIBRARY_PATH=$VVL/build/tests/icd:$VVL/build/layers:$PROFILES   # DYLD_LIBRARY_PATH on macOS
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation:VK_LAYER_KHRONOS_profiles   # order matters
export VK_KHRONOS_PROFILES_PROFILE_FILE=$VVL/tests/device_profiles/max_profile.json
export VK_KHRONOS_PROFILES_DEBUG_REPORTS=DEBUG_REPORT_ERROR_BIT
export VK_KHRONOS_PROFILES_EMULATE_PORTABILITY=false
export VK_KHRONOS_PROFILES_SIMULATE_CAPABILITIES=SIMULATE_API_VERSION_BIT,SIMULATE_FEATURES_BIT,SIMULATE_PROPERTIES_BIT,SIMULATE_EXTENSIONS_BIT,SIMULATE_FORMATS_BIT,SIMULATE_QUEUE_FAMILY_PROPERTIES_BIT,SIMULATE_VIDEO_CAPABILITIES_BIT,SIMULATE_VIDEO_FORMATS_BIT
```

Windows (PowerShell):
```powershell
$env:VK_DRIVER_FILES = "$VVL\build\tests\icd\Debug\VVL_Test_ICD.json"
$env:VK_LAYER_PATH = "$VVL\build\layers\Debug;$PROFILES"            # ';' separator
$env:VK_INSTANCE_LAYERS = "VK_LAYER_KHRONOS_validation;VK_LAYER_KHRONOS_profiles"   # ';' on Windows, order matters
$env:VK_KHRONOS_PROFILES_PROFILE_FILE = "$VVL\tests\device_profiles\max_profile.json"
$env:VK_KHRONOS_PROFILES_DEBUG_REPORTS = "DEBUG_REPORT_ERROR_BIT"
$env:VK_KHRONOS_PROFILES_EMULATE_PORTABILITY = "false"
$env:VK_KHRONOS_PROFILES_SIMULATE_CAPABILITIES = "SIMULATE_API_VERSION_BIT,SIMULATE_FEATURES_BIT,SIMULATE_PROPERTIES_BIT,SIMULATE_EXTENSIONS_BIT,SIMULATE_FORMATS_BIT,SIMULATE_QUEUE_FAMILY_PROPERTIES_BIT,SIMULATE_VIDEO_CAPABILITIES_BIT,SIMULATE_VIDEO_FORMATS_BIT"
```

In `cmd`, use `set NAME=value` (no quotes around the value) with the same names and `;` separators.

- The `Debug` folder in the paths above is the Visual Studio layout. With Ninja the output is `build\layers` and
  `build\tests\icd`. Check where `VVL_Test_ICD.json` and `VkLayer_khronos_validation.json` actually are.
- There is no `LD_LIBRARY_PATH` on Windows. The loader finds the DLLs through the `library_path` in each manifest,
  so the manifest has to sit next to its DLL (the build does this).
- Always run the mock ICD with a profile. Without the profiles layer every test fails in `vkCreateDevice`
  (`VUID-VkDeviceCreateInfo-pProperties-04451`).
- For a real driver or lavapipe, no layer variables are needed beyond `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`
  (plus `VK_DRIVER_FILES` for lavapipe). Unset the mock ICD variables first, or you will still be on the mock ICD.
- If an unrelated implicit layer changes the output (overlays, capture tools), disable it with
  `VK_LOADER_LAYERS_DISABLE`.
- Environment variables are per shell. In PowerShell, `$env:X = ...` lasts for the session. Close the session or
  `Remove-Item Env:X` to clear one.

### Interpreting results

- **A skipped test is not a passing test.** Read the skip reason. A test that skips on your driver has verified
  nothing. If the skip is "extension not supported" under the mock ICD, the extension is likely missing from
  `max_profile.json` (it hides extensions it does not list), so add it there or try another driver.
- A new test must **fail without your fix** and pass with it. Run it before and after, and say so in the PR.
- "Did not receive expected error" often means a *different* error fired first. Read the whole output, not the tail.
  The chassis stops at the first validation object that sets skip (stateless runs before core checks), and an
  unexpected warning also counts as a failure.
- WSI tests need a window system (X11/Wayland on Linux, a desktop session on Windows). Without one they skip. Say so
  and do not report them as passed.
- Never make a test pass with `SetAllowedFailureMsg` (see [docs/limitations.md](./docs/limitations.md)).

### Test-writing traps

- Check [docs/creating_tests.md](./docs/creating_tests.md) first, and copy a nearby test.
- Keep SPIR-V/GLSL shaders inline in the test. Don't add test-class helpers or shared shader strings.
- Depth-only rendering with `CreatePipelineHelper` still gets the default fragment shader that writes color. That
  raises `Undefined-Value-ShaderOutputNotConsumed-DynamicRendering`, which hides any other warning you expected
  (such as Best Practices). Use a vertex-only pipeline: `pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo()};`
- Positive tests only catch warnings if they call `m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit)`.
- Call `SetTargetApiVersion(...)` *before* `AddRequiredFeature(...)`, otherwise device creation can fail with
  `VUID-VkDeviceCreateInfo-pNext-02830`.
- GPU-AV tests skip on the mock ICD; they need a real driver. Use `IsPlatformMockICD()` to skip a test that cannot
  work there, with a reason.
- Validate hand-written SPIR-V with `spirv-as` / `spirv-val` (both ship in the Vulkan SDK) before trusting it in a test.
- Code must build on Windows (MSVC), Linux (GCC/Clang), macOS and Android. Avoid compiler extensions, VLAs,
  `__builtin_*` without a fallback, and `long` where a fixed-width type is meant (it is 32-bit on Windows). Do not
  hardcode `/` path separators or POSIX-only calls in tests. Query limits instead of assuming sample counts,
  formats or queue families exist. MSVC is also stricter about implicit narrowing conversions and out-of-range
  container reads, so a Linux build alone does not prove the code is clean.

### Before you hand the patch over

- Format: `git clang-format --style=file` (CI uses clang-format 22.1.x, which may differ from the one bundled with
  Visual Studio, so check `clang-format --version`). Note `-q` can hide "has unstaged changes", so check the diff is
  empty afterwards.
- Build with `-D BUILD_WERROR=ON`. CI treats warnings as errors. If you only built on one platform, say so.
- Run the relevant tests on at least one driver. For GPU-AV changes, also run on a real driver with self
  validation (below). For memory-safety changes, build with `-D VVL_ENABLE_ASAN=ON` (or `-D VVL_ENABLE_UBSAN=ON`)
  in a separate build directory.
- Say exactly what you ran (OS, driver, filter, build type) and what was skipped. Do not claim a full-suite pass if
  you ran a filter.

### Self validation (GPU-AV and DebugPrintf)

GPU-AV records its own Vulkan commands. Validate them by stacking a second, separately built layer below it
([docs/gpu_av_development.md](./docs/gpu_av_development.md)):

```bash
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation:VK_LAYER_DEV_self_validation   # ';' separator on Windows
export VK_LAYER_PATH=$VVL/build/layers:/path/to/self_validation_build/layers          # ';' separator on Windows
```

Build the self validation layer from an unmodified checkout, separate from the layer you are editing.
