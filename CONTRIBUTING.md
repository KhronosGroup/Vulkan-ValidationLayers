# How to Contribute to Vulkan Source Repositories

## **The Repository**

The source code for The Vulkan-ValidationLayer components is sponsored by Khronos and LunarG.
* [Khronos Vulkan-ValidationLayers](https://github.com/KhronosGroup/Vulkan-ValidationLayers)


### **The Vulkan Ecosystem Needs Your Help**

The Vulkan validation layers are one of the larger and more important components in this repository.
While there are often active and organized development efforts underway to improve their coverage,
there are always opportunities for anyone to help by contributing additional validation layer checks
and tests for these validation checks.

There are a couple of methods to identify areas of need:
* Examine the [issues list](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues)
in this repository and look for issues that are of interest
* Alternatively, run the `vk_validation_stats.py` script (in the scripts directory) with the `-todo`
command line argument to see a list of as-yet unimplemented validation checks.
* Having selected a validation check to work on, it is often efficient to implement a block of related checks
at once. Refer to the validation database output from `vk_validation_stats.py` (available in text, html,
or csv format) to identify related checks that may be implemented simultaneously.

Of course, if you have your own work in mind, please open an issue to describe it and assign it to yourself.
Finally, please feel free to contact any of the developers that are actively contributing should you
wish to coordinate further.
Please see the [section about Validation Layers](#special-considerations-for-validation-layers)
later on this page.

Repository Issue labels:

* _Bug_:          These issues refer to invalid or broken functionality and are the highest priority.
* _Incomplete_:   These issues refer to missing validation checks that users have encountered during application
development that would have been directly useful, and are high priority.
* _Enhancement_:  These issues refer to ideas for extending or improving the validation layers.
* _Triaged_:      These issues have been assessed and/or reviewed

It is the maintainers goal for all issues to be assigned or triaged within one business day of their submission. If you choose
to work on an issue that is assigned, simply coordinate with the current assignee.

### **How to Submit Fixes**

* **Ensure that the bug was not already reported or fixed** by searching on GitHub under Issues
  and Pull Requests.
* Use the existing GitHub forking and pull request process.
  This will involve [forking the repository](https://help.github.com/articles/fork-a-repo/),
  creating a branch with your commits, and then [submitting a pull request](https://help.github.com/articles/using-pull-requests/).
* Please read and adhere to the style and process [guidelines ](#coding-conventions-and-formatting) enumerated below.
* Please base your fixes on the master branch.  SDK branches are generally not updated except for critical fixes needed to repair an SDK release.
* The resulting Pull Request will be assigned to a repository maintainer. It is the maintainer's responsibility to ensure the Pull Request
  passes the Google/LunarG internal CI processes. Once the Pull Request has been approved and is passing internal CI, a repository maintainer
  will merge the PR.


#### **Coding Conventions and Formatting**
* Use the **[Google style guide](https://google.github.io/styleguide/cppguide.html)** for source code with the following exceptions:
    * The column limit is 132 (as opposed to the default value 80). The clang-format tool will handle this. See below.
    * The indent is 4 spaces instead of the default 2 spaces. Again, the clang-format tool will handle this.
    * If you can justify a reason for violating a rule in the guidelines, then you are free to do so. Be prepared to defend your
decision during code review. This should be used responsibly. An example of a bad reason is "I don't like that rule." An example of
a good reason is "This violates the style guide, but it improves type safety."

* Run **clang-format** on your changes to maintain consistent formatting
    * There are `.clang-format` files present in the repository to define clang-format settings
      which are found and used automatically by clang-format.
	* **clang-format** binaries are available from the LLVM orginization, here: [LLVM](https://clang.llvm.org/). Our CI system (Travis-CI)
	  currently uses clang-format version 5.0.0 to check that the lines of code you have changed are formatted properly. It is
	  recommended that you use the same version to format your code prior to submission.
    * A sample git workflow may look like:

>        # Make changes to the source.
>        $ git add -u .
>        $ git clang-format --style=file
>        # Check to see if clang-format made any changes and if they are OK.
>        $ git add -u .
>        $ git commit

* **Commit Messages**
    * Limit the subject line to 64 characters -- this allows the information to display correctly in git/GitHub logs
    * Begin subject line with a one-word component description followed by a colon (e.g. build, docs, layers, tests, etc.)
    * Separate subject from body with a blank line
    * Wrap the body at 72 characters
    * Capitalize the subject line
    * Do not end the subject line with a period
    * Use the body to explain what and why vs. how
    * Use the imperative mode in the subject line. This just means to write it as a command (e.g. Fix the sprocket)

Strive for commits that implement a single or related set of functionality, using as many commits as is necessary (more is better).
That said, please ensure that the repository compiles and passes tests without error for each commit in your pull request.  Note
that to be accepted into the repository, the pull request must [pass all tests](#testing your changes) on all supported platforms
-- the automatic Github Travis and AppVeyor continuous integration features will assist in enforcing this requirement.

#### **Testing Your Changes**
* Run the existing tests in the repository before and after each of your commits to check for any regressions.
  There are some tests that appear in all repositories.
  These tests can be found in the following folders inside of your target build directory:

  (These instructions are for Linux)

* In the `tests` directory, run:

>        run_all_tests.sh

* On Windows, a quick sanity check can be run from inside Visual Studio -- just run the `vk_layer_validation_tests` project,
or you can run `run_all_tests.ps1` from a PowerShell window

* Note that some tests may fail with known issues or driver-specific problems.
  The idea here is that your changes should not change the test results, unless that was the intent of your changes.
* Run tests that explicitly exercise your changes.
* Feel free to subject your code changes to other tests as well!

#### **GitHub Cloud CI Testing**
Pull Requests to GitHub are tested in the cloud on Linux and Windows VMs. The Linux VMs use [Travis CI](https://travis-ci.org/KhronosGroup/Vulkan-ValidationLayers) with the sequence of commands driven by the [.travis.yml](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/.travis.yml) file. The Windows VMs use [AppVeyor](https://ci.appveyor.com/project/Khronoswebmaster/vulkan-validationlayers/branch/master) with the sequence of commands driven by the [.appveyor.yml](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/.appveyor.yml) file.

The Linux testing includes iterating on all of the validation layer tests over multiple [different device](https://github.com/KhronosGroup/Vulkan-ValidationLayers/tree/master/tests/device_profiles) profiles using the [devsim layer](https://github.com/LunarG/VulkanTools/tree/master/layersvt) in combination with the [mock icd](https://github.com/KhronosGroup/Vulkan-Tools/tree/master/icd). This is a fast way to simulate testing across different devices. Any new tests must pass across all device profiles.

#### **Special Considerations for Validation Layers**
* **Validation Tests:**  If you are submitting a change that adds a new validation check, you should also construct a "negative" test function.
The negative test function purposely violates the validation rule that the new validation check is looking for.
The test should cause your new validation check to identify the violation and issue a validation error report.
And finally, the test should check that the validation error report is generated and consider the test as "passing"
if the report is received.  Otherwise, the test should indicate "failure".
This new test should be added to the validation layer test program in the `tests` directory and contributed
at the same time as the new validation check itself. There are many existing validation tests in this directory that can be
used as a starting point.
* **Validation Checks:**  Validation checks are carried out by the Khronos Validation layer. The CoreChecks validation object
contains checks that require significant amounts of application state to carry out. In contrast, the stateless validation object contains
checks that require (mostly) no state at all. Please inquire if you are unsure of the location for your contribution. The other
validation objects (thread_safety, object lifetimes) are more special-purpose and are mostly code-generated from the specification.
* **Validation Error/Warning Messages:**  Strive to give specific information describing the particulars of the failure, including
output all of the applicable Vulkan Objects and related values. Also, ensure that when messages can give suggestions about _how_ to
fix the problem, they should do so to better assist the user.
* **Validation Statistics:** The `vk_validation_stats.py` script (in the scripts directory) inspects the layer and test source files
and reports a variety of statistics on validation completeness and correctness. Before submitting a change you should run this
script with the consistency check (`-c`) argument to ensure that your changes have not introduced any inconsistencies in the code.
* **Generated Source Code:** The `layers/generated` directory contains source code that is created by several
generator scripts in the `scripts` directory. All changes to these scripts _must_ be submitted with the
corresponding generated output to keep the repository self-consistent. This requirement is enforced by both
Travis CI and AppVeyor test configurations. Regenerate source files after modifying any of the generator
scripts and before building and testing your changes. More details can be found in
[BUILD.md](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/master/BUILD.md#generated-source-code).

#### Coding Conventions for [CMake](http://cmake.org) files

* When editing configuration files for CMake, follow the style conventions of the surrounding code.
  * The column limit is 132.
  * The indent is 4 spaces.
  * CMake functions are lower-case.
  * Variable and keyword names are upper-case.
* The format is defined by
  [cmake-format](https://github.com/cheshirekow/cmake_format)
  using the `.cmake-format.py` file in the repository to define the settings.
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
