#!/usr/bin/env python3
# Copyright (c) 2023-2023 The Khronos Group Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
import inspect
import functools
import pathlib
import importlib
import difflib

class Test:
    def __init__(self):
        self.has_error = False

        # Get script directory name
        dirname = os.path.dirname(os.path.abspath(__file__))

        # Change working directory to the directory where the script is located
        os.chdir(dirname)

        registry_dir = os.path.join(dirname, 'build/vvl/external/Vulkan-Headers/registry')
        grammar_dir = os.path.join(dirname, 'build/vvl/external/SPIRV-Headers/include/spirv/unified1')

        registry_file = os.path.join(registry_dir, 'vk.xml')
        grammar_file = os.path.join(grammar_dir, 'spirv.core.grammar.json')

        # Also add the registry and script directories to the python package search path
        sys.path.insert(0, registry_dir)
        sys.path.insert(1, os.path.join(pathlib.Path(dirname).parent.parent.absolute(), 'scripts'))

        from api_specific_generator_interfaces import APISpecificInterfaces

        # Test that the APISpecific generators have the desired interfaces and patch them
        # with the test implementations
        for package_name in APISpecificInterfaces.generators.__dict__.keys():
            if package_name.startswith('__'):
                continue

            package = getattr(APISpecificInterfaces.generators, package_name)

            try:
                module = importlib.import_module(f'generators.{package_name}')
            except:
                self.error(f'Generator package "{package_name}" not found.')
                continue

            try:
                cls = getattr(module, 'APISpecific')
            except:
                self.error(f'No APISpecific class found in generator package "{package_name}".')
                continue

            method_names = [x for x in package.__dict__.keys() if not x.startswith('__') and not x.startswith('verify_')]
            for method_name in method_names:
                try:
                    method = getattr(cls, method_name)
                except:
                    self.error(f'Method "{package_name}.APISpecific.{method_name}" not found.')
                    continue

                test_method = getattr(package, method_name)

                # Test signature
                if inspect.signature(method) != inspect.signature(test_method):
                    self.error(f'Method "{package_name}.APISpecific.{method_name}" signature mismatch.')

                # Test that the interface only returns something when using the 'vulkan' target API name
                for targetApiName in ['vulkan', 'vulkanvariant1',  'vulkanvariant2']:
                    for param_name, param_data in inspect.signature(method).parameters.items():
                        if param_name == 'targetApiName':
                            bound_method = functools.partial(method, targetApiName)
                        else:
                            bound_method = functools.partial(bound_method, param_data.annotation())
                    try:
                        result = bound_method()
                        if targetApiName != 'vulkan':
                            if result is not None:
                                self.error(f'Method "{package_name}.APISpecific.{method_name}" does not return None for non-Vulkan target API.')
                    except:
                        # Some functions have additional requirements on their inputs
                        pass

                setattr(cls, method_name, test_method)
                setattr(package, f'original_{method_name}', method)

        if self.has_error:
            exit(1)

        # Load source generator function
        RunGenerators = getattr(importlib.import_module('generate_source'), 'RunGenerators')

        # Create directory for generated files
        gen_dir = os.path.join(dirname, 'build/generated')
        ref_gen_dir = os.path.join(pathlib.Path(dirname).parent.parent.absolute(), 'layers/vulkan/generated')
        if not pathlib.Path(gen_dir).is_dir():
            os.mkdir(gen_dir)

        # Run generators
        RunGenerators('vulkan', registry_file, grammar_file, gen_dir, None)

        # Verify output
        contents = {}
        ref_contents = {}
        for filename in APISpecificInterfaces.gen_src_diff_rules.keys():
            # Load generated file
            file = open(f'{gen_dir}/{filename}', mode='r')
            contents[filename] = file.read()
            file.close()

            # Load reference generated file
            file = open(f'{ref_gen_dir}/{filename}', mode='r')
            ref_contents[filename] = file.read()
            file.close()

        # Check contains rules
        methods_with_no_effects = {}
        for filename, rules in APISpecificInterfaces.gen_src_diff_rules.items():
            for rule in rules:
                if not rule.contains in contents[filename]:
                    self.error(f'Could not find "{rule.contains}" in "{filename}".')
                    if not rule.method in methods_with_no_effects:
                        methods_with_no_effects[rule.method] = set()
                    methods_with_no_effects[rule.method].add(filename)

        for method_name, filenames in methods_with_no_effects.items():
            self.error(f'Effect of method "{method_name}" is not observable in the generated file(s): {filenames}.')

        # Apply replacement rules
        for filename, rules in APISpecificInterfaces.gen_src_diff_rules.items():
            for rule in rules:
                contents[filename] = contents[filename].replace(rule.contains, rule.matches_when_replaced_with)

        # Compare contents:
        for filename in APISpecificInterfaces.gen_src_diff_rules.keys():
            if contents[filename] != ref_contents[filename]:
                self.error(f'Effect of API-specific test methods are different than expected in the generated file "{filename}":\n' +
                            ''.join(difflib.unified_diff(
                                [x + '\n' for x in ref_contents[filename].split('\n')],
                                [x + '\n' for x in contents[filename].split('\n')])))

        if self.has_error:
            exit(1)


    def error(self, msg: str):
        print(f'ERROR: {msg}')
        self.has_error = True

    def fatal(self, msg: str):
        print(f'ERROR: {msg}')
        exit(1)


if __name__ == '__main__':
    Test()
