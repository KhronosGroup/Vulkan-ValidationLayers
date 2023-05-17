# GPU Assisted Validation Shaders

This directory is for holding shaders that are used for [GPU Assisted Validation](../../docs/gpu_validation.md). These are turned in SPIR-V when generating code with `generate_spirv.py`　and turned into a header file in [layers/vulkan/generated](../generated/).

To regenerate the validation shader, run the following:

```bash
# generate all the shaders with glslangValidator at external/glslang/build/install/bin/glslangValidator
python3 ./scripts/generate_spirv.py

# Using own glslangValidator executable
python3 ./scripts/generate_spirv.py --glslang path/to/glslangValidator

# generate a single shader
python3 ./scripts/generate_spirv.py --shader layers/gpu_shaders/gpu_pre_draw.vert
```

## Adding a new shader

1. Add the GLSL shader to this folder with the appropriate naming (consistent with the other files)
2. Include the generated header file
    - example: `#include "gpu_pre_draw_vert.h"` for `gpu_pre_draw.vert`
3. Add the new header file to `BUILD.gn`