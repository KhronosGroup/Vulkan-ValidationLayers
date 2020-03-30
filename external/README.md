# External dependencies

This directory provides a location where external projects can be cloned that
are used by the validation layers

In order to build tests the Google Test library must be present. It can be
checked out with:

```
git clone https://github.com/google/googletest.git
```

The optional depedencies can either be installed into the system directories or
placed in externals.

```
git clone https://github.com/KhronosGroup/Vulkan-Headers.git
git clone https://github.com/KhronosGroup/GLSLang.git
git clone https://github.com/KhronosGroup/SPIRV-Headers.git
git clone https://github.com/KhronosGroup/SPIRV-Tools.git
```
