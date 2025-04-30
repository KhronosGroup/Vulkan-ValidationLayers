# How to Contribute to Vulkan Source Repositories

The source code for The Vulkan-ValidationLayer components is sponsored by Khronos and LunarG.

We enjoy external contributions, we often see them in the form of:

- Improving error messages (or fixing typo).
- Adding a missing VU (especially for Vendor extensions).
- Improving areas of code not as actively developed.

## Quick check list for making a change

- Make sure you run `clang-format` on any C++ code.
- If fixing a bug, we would like a [positive test](./tests/README.md#different-categories-of-tests).
- If addinga a new VU, we **need** a [negative test](./tests/README.md#different-categories-of-tests).
- Try to match the code style around the code as best as possible.
- If dealing with generated code, we have docs [here](./docs/generated_code.md) and [here](./docs/python_scripts_code_style.md).
- Run `scripts/check_code_format.py` after you commit and before you make the PR to see if things pass locally.

## Coding Conventions and Formatting

We are reasonable developers, we don't want to bikeshed on code style, but highly encourge to look at the nearby code.
As maintainer, if we find the style is incredibly different, we will ask you kindly to fix it.

our CI will run **clang-format** (version 14) **FOR EVERY COMMIT**, so make sure your change has been ran with it.

```bash
# sample git workflow may look like
git add -u .
git clang-format --style=file

# Check to see if clang-format made any changes and if they are OK.
git add -u .
git commit
```

## Commit Messsage

Some basic rules (enforced by CI)

- Limit the subject line to 64 characters
- Begin subject line with a one-word component description followed by a colon
    - ex: `build:`, `docs:`, `layers:`, `tests:`
- Separate subject from body with a blank line
- Wrap the body at 72 characters
- Capitalize the subject line

## Writing good error messages

We have a `--print-vu` option in the test suite to view how a VU looks like.

We will be strict to enforce any new VUs added have a good, well written error message. It should contain:

1. All values related to the error message
2. Explain the logic that got to that error
3. Don't repeat the VU from the spec (it gets appended to the custom error message)
4. Leave no doubt to the developer that the error message is not a VVL bug

## Testing Your Changes

- [How to setup tests to run](./tests)
- [Overview for creating tests](docs/creating_tests.md).

> Tip - If you make a fork and push to it, it will run CI there before your make a PR!

### **Contributor License Agreement (CLA)**

You will be prompted with a one-time "click-through" CLA dialog as part of submitting your pull request
or other contribution to GitHub.

### **License and Copyrights**

All contributions made to the Vulkan-ValidationLayers repository are Khronos branded and as such,
any new files need to have the Khronos license (Apache 2.0 style) and copyright included.
Please see an existing file in this repository for an example.

All contributions made to the LunarG repositories are to be made under the Apache 2.0 license
and any new files need to include this license and any applicable copyrights.

You can include your individual copyright after any existing copyrights.
