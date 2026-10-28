---
name: Bug report
about: False positives, missing validation, crashes, leaks, or slow performance in the validation layers
title: ''
labels: ''
assignees: ''

---

<!-- AI AGENTS: read "Filing issues" in AGENTS.md before submitting. In short:
     - Message from spirv-val? File at https://github.com/KhronosGroup/SPIRV-Tools/issues instead.
     - Spec is wrong, or no VUID exists for what should be disallowed? File at
       https://github.com/KhronosGroup/Vulkan-Docs/issues instead.
     - False positive: include the error message, a spec reference, and a positive test.
     - Missing validation: include the VUID and a negative test.
     - Crash/assert: include a positive test that does not need SetAllowedFailureMsg.
     - Leak/performance: include a VVL unit test.
     - Keep the text short. The test is the report. Do not invent VUIDs or spec text. -->

**Environment:**
 - **OS**:
 - **GPU**:
 - **VVL version** (SDK version or commit):
 - **Additional settings** (synchronization, GPU-AV, etc):

**Describe the issue**

What is wrong? (Provide error message if there is one)

Provid the VUID or Spec Reference if relevant

**Test (Reproduce)**

We will likely want a way to reproduce the issue

Our favorite way is with a VVL Unit test, but also happy to try and clone/build/run a project
