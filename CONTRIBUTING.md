# How to Contribute to The Vulkan Validation Layer

The source code for The Vulkan-ValidationLayer components is sponsored by Khronos and LunarG.
The Vulkan validation layers make up a significant part of the Vulkan ecosystem.
While there are often active and organized development efforts underway to improve their coverage,
opportunities always exist for anyone to help by contributing additional validation layer checks
and tests.

It is the maintainers goal for all issues to be assigned or triaged within one business day of their submission. If you choose
to work on an issue that is assigned, simply coordinate with the current assignee.

## Incomplete VUIDs

There are some VUID that are incomplete and need to be added. The following can be used to find them
* [Incomplete tagged issues](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues?q=is%3Aopen+is%3Aissue+label%3AIncomplete)
* The 'Coverage - html' page at [the Vulkan SDK documentation page](https://vulkan.lunarg.com/doc/sdk/latest/windows/validation_error_database.html)
  * it lists all published Vulkan VUIDs and their status.
* Run `scripts/vk_validation_stats.py` with `-todo` to see a list of as-yet unimplemented validation checks.
  * ```bash
    # Get summary report
    python3 scripts/vk_validation_stats.py external/Vulkan-Headers/registry/validusage.json -summary
    # Some VUIDs are handled in `spirv-val` and need to pass in the repo to check against
    python3 scripts/vk_validation_stats.py external/Vulkan-Headers/registry/validusage.json -spirvtools ~/path/to/SPIRV-Tools/ -summary
    # Print out all the information to an HTML page (also has text and csv support)
    python3 scripts/vk_validation_stats.py external/Vulkan-Headers/registry/validusage.json -spirvtools ~/path/to/SPIRV-Tools/ -html vuid.html
    # -todo filters out only VUID that are unimplemented
    python3 scripts/vk_validation_stats.py external/Vulkan-Headers/registry/validusage.json -spirvtools ~/path/to/SPIRV-Tools/ -todo -html todo.html
    ```

## How to Submit Fixes

* **Ensure that the bug was not already reported or fixed** by searching on GitHub under Issues and Pull Requests.
* Use the existing GitHub [forking](https://help.github.com/articles/fork-a-repo/) and [pull request](https://help.github.com/articles/using-pull-requests/) process.
* Source code must follow the repo coding style guidelines, including a pass through a clang-format utility
* Test your change with [internal test suite](./tests)
* The resulting Pull Request will be assigned to a repository maintainer. It is the maintainer's responsibility to ensure the Pull Request
  passes the Google/LunarG internal CI processes. Once the Pull Request has been approved and is passing internal CI, a repository maintainer
  will merge the PR.
* Please base your fixes on the master branch. SDK branches are generally not updated except for critical fixes needed to repair an SDK release.

If implementing a new VUID some extra considerations:
* New VUID checks must be accompanied by a [relevant tests](./docs/creating_tests.md)
* Validation source code should be in a separate commit from the tests, unless there are interdependencies. The repo should compile and pass all tests after each commit.
* Strive to give specific information describing the particulars of the failure, including output all of the applicable Vulkan Objects and related values. Also, ensure that when messages can give suggestions about _how_ to fix the problem, they should do so to better assist the user.

## Repo Coding Conventions

### Coding styling

Use the **[Google style guide](https://google.github.io/styleguide/cppguide.html)** for source code with the following exceptions:
* The column limit is 132 (as opposed to the default value 80). The clang-format tool will handle this. See below.
* The indent is 4 spaces instead of the default 2 spaces. Access modifier (e.g. `public:`) is indented 2 spaces instead of the
    default 1 space. Again, the clang-format tool will handle this.
* The C++ file extension is `*.cpp` instead of the default `*.cc`.
* If you can justify a reason for violating a rule in the guidelines, then you are free to do so. Be prepared to defend your
decision during code review. This should be used responsibly. An example of a bad reason is "I don't like that rule." An example of
a good reason is "This violates the style guide, but it improves type safety."

> New code should target the above Google style guide, avoid copying/pasting incorrectly formatted code.

### Formatting with clang format

Run **clang-format** on your changes to maintain consistent formatting
* There are `.clang-format` files present in the repository to define clang-format settings
    which are found and used automatically by clang-format.
* **clang-format** binaries are available from the LLVM orginization, here: [LLVM](https://clang.llvm.org/). Our CI system
    currently uses clang-format version 7.0.0 to check that the lines of code you have changed are formatted properly. It is
    recommended that you use the same version to format your code prior to submission.
* A sample git workflow may look like:

>        # Make changes to the source.
>        $ git add -u .
>        $ git clang-format --style=file
>        # Check to see if clang-format made any changes and if they are OK.
>        $ git add -u .
>        $ git commit

### Commit Messages

* Limit the subject line to 64 characters -- this allows the information to display correctly in git/GitHub logs
* Begin subject line with a one-word component description followed by a colon (e.g. build, docs, layers, tests, etc.)
* Separate subject from body with a blank line
* Wrap the body at 72 characters
* Capitalize the subject line
* Do not end the subject line with a period
* Use the body to explain what and why vs. how
* Use the imperative mode in the subject line. This just means to write it as a command (e.g. Fix the sprocket)

Strive for commits that implement a single or related set of functionality, using as many commits as is necessary (more is better).
That said, please ensure that the repository compiles and passes tests without error for each commit in your pull request.

### Coding Conventions for [CMake](http://cmake.org) files

* When editing configuration files for CMake, follow the style conventions of the surrounding code.
  * The column limit is 132.
  * The indent is 4 spaces.
  * CMake functions are lower-case.
  * Variable and keyword names are upper-case.
* The format is defined by
  [cmake-format](https://github.com/cheshirekow/cmake_format)
  using the `cmake-format.py` file in the repository to define the settings.
  See the cmake-format page for information about its simple markup for comments.
* Disable reformatting of a block of comment lines by inserting
  a `# ~~~` comment line before and after that block.
* Disable any formatting of a block of lines by surrounding that block with
  `# cmake-format: off` and `# cmake-format: on` comment lines.
* To install: `sudo pip install cmake_format`
* To run: `cmake-format --in-place $FILENAME`
* **IMPORTANT (June 2018)** cmake-format v0.3.6 has a
  [bug]( https://github.com/cheshirekow/cmake_format/issues/50)
  that can corrupt the formatting of comment lines in CMake files.
  A workaround is to use the following command _before_ running cmake-format:
  `sed --in-place='' 's/^  *#/#/' $FILENAME`

## GitHub Cloud CI Testing
Pull Requests to GitHub are tested in the cloud on Linux and Windows VMs. The Linux VMs use [Github Actions](https://github.com/KhronosGroup/Vulkan-ValidationLayers/actions) with the sequence of commands driven by the [ci_build.yml](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/.github/workflows/ci_build.yml) file. The Windows VMs use [AppVeyor](https://ci.appveyor.com/project/Khronoswebmaster/vulkan-validationlayers/branch/master) with the sequence of commands driven by the [.appveyor.yml](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/.appveyor.yml) file.

The Linux testing includes iterating on all of the validation layer tests over multiple [different device](https://github.com/KhronosGroup/Vulkan-ValidationLayers/tree/master/tests/device_profiles) profiles using the [devsim layer](https://github.com/LunarG/VulkanTools/tree/master/layersvt) in combination with the [mock icd](https://github.com/KhronosGroup/Vulkan-Tools/tree/master/icd). This is a fast way to simulate testing across different devices. Any new tests must pass across all device profiles.

## Contributor License Agreement (CLA)

You will be prompted with a one-time "click-through" CLA dialog as part of submitting your pull request
or other contribution to GitHub.

## License and Copyrights

All contributions made to the Vulkan-ValidationLayers repository are Khronos branded and as such,
any new files need to have the Khronos license (Apache 2.0 style) and copyright included.
Please see an existing file in this repository for an example.

All contributions made to the LunarG repositories are to be made under the Apache 2.0 license
and any new files need to include this license and any applicable copyrights.

You can include your individual copyright after any existing copyrights.
