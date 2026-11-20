# GPU-AV Shader Abort

The `Shader Abort` setting (`gpuav_shader_abort`) uses [VK_KHR_shader_abort](https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_shader_abort.html) to stop the shader at the **first** error found by [shader instrumentation](./gpu_av_shader_instrumentation.md).

Because abort **will** cause a device lost when an error is detected. This option was designed for those who want something like "safe mode", but more agressive to ensure they can have the error message reported back.

## How the shader is instrumented

`gpuav_shader_abort` is separate from `gpuav_safe_mode` and is **off** by default.

Internally, `Shader Abort` reuses the Safe Mode control flow, the only difference is what happens in the "invalid" block. Using descriptor indexing as an example

```glsl
// Unsafe Mode
inst_descriptor_indexing_oob(/*...*/);  // writes error_payload if invalid
value = texture(tex[index], uv);

// Safe Mode
if (inst_descriptor_indexing_oob(/*...*/)) {
    value = texture(tex[index], uv);
} else {
    value = vec4(0.0);  // the result is an OpPhi of the 2 paths
}

// Shader Abort
if (inst_descriptor_indexing_oob(/*...*/)) {
    value = texture(tex[index], uv);
} else {
    abort(inst_abort_message(stage_info));  // OpAbortKHR
}
```

Since the abort block never reaches the merge block, there is no need for a "null" value or `OpPhi`, the valid block is the only path to the merge block.

The SPIR-V looks like

```swift
%valid = OpFunctionCall %bool %inst_descriptor_indexing_oob ...
         OpSelectionMerge %merge None
         OpBranchConditional %valid %valid_block %abort_block

%valid_block = OpLabel
         ; original instruction
         OpBranch %merge

%abort_block = OpLabel
%message = OpFunctionCall %AbortMessage %inst_abort_message %stage_info
         OpAbortKHR %AbortMessageLayout %message

%merge = OpLabel
         ; rest of the original block
```

## The abort message

Nothing written to memory before `OpAbortKHR` is guaranteed to be readable after the device is lost, so the error buffer can't be trusted. Instead, everything needed to report the error goes into the abort message.

The 3 things we need inside the abort message are

1. an "abort magic value" so we can ignore messages from the app's own `OpAbortKHR`
2. a "command buffer id"  because at `vkGetDeviceFaultDebugInfoKHR` time multiple command buffers might be in flight and we need to know which one produced the error.
3. The error record, which is the exact same layout in the buffer as a normal error message
