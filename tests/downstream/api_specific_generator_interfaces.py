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

from generators.vulkan_object import Version

class TestDiffRule:
    def __init__(self, method, contains, matches_when_replaced_with):
        self.method = method
        self.contains = contains
        self.matches_when_replaced_with = matches_when_replaced_with


# API-specific interfaces across the generators that need testing
# and custom test implementations provided for each
class APISpecificInterfaces:

    # Dictionary of source diff rules to verify the effects of the
    # API-specific interfaces
    gen_src_diff_rules: dict[str, list[TestDiffRule]] = {}

    @staticmethod
    def addGenSrcDiffRule(filename: str, rule: TestDiffRule):
        if not filename in APISpecificInterfaces.gen_src_diff_rules:
            APISpecificInterfaces.gen_src_diff_rules[filename] = []
        APISpecificInterfaces.gen_src_diff_rules[filename].append(rule)


    class generators:
        # BaseGenerator
        class base_generator:
            @staticmethod
            def createApiVersion(targetApiName: str, name: str, number: str) -> Version:
                nameApi = name.replace('VK_', 'VK{NameApi}_API_')
                nameString = '"' + name.replace('VK_', 'VK{NameString}_') + '"'

                APISpecificInterfaces.addGenSrcDiffRule('vk_extension_helper.h', TestDiffRule(
                    'base_generator.APISpecific.createApiVersion',
                    '{NameString}', ''))

                APISpecificInterfaces.addGenSrcDiffRule('stateless_validation_helper.cpp', TestDiffRule(
                    'base_generator.APISpecific.createApiVersion',
                    '{NameApi}', ''))

                return Version(name, nameString, nameApi, number)


        # EnumFlagBitsOutputGenerator
        class enum_flag_bits_generator:
            @staticmethod
            def genManualConstants(targetApiName: str) -> list[str]:
                APISpecificInterfaces.addGenSrcDiffRule('enum_flag_bits.h', TestDiffRule(
                    'enum_flag_bits_generator.APISpecific.genManualConstants',
                    '{Foo}{Bar}', ''))

                return ['{Foo}', '{Bar}']


        # ExtensionHelperOutputGenerator
        class extension_helper_generator:
            @staticmethod
            def genAPIVersionSource(targetApiName: str) -> str:
                modified_result = '{FooBarAPIVersionSource}'
                APISpecificInterfaces.addGenSrcDiffRule('vk_extension_helper.h', TestDiffRule(
                    'extension_helper_generator.APISpecific.genAPIVersionSource',
                    modified_result,
                    APISpecificInterfaces.generators.extension_helper_generator.original_genAPIVersionSource(targetApiName)))
                return modified_result

            @staticmethod
            def getVersionFieldNameDict(targetApiName: str) -> dict[str, str]:
                orig_result = APISpecificInterfaces.generators.extension_helper_generator.original_getVersionFieldNameDict(targetApiName)
                modified_result = {}
                for key in orig_result:
                    modified_result[key] = '{FooBar' + orig_result[key] + 'VersionField}'

                    APISpecificInterfaces.addGenSrcDiffRule('vk_extension_helper.h', TestDiffRule(
                        'extension_helper_generator.APISpecific.getVersionFieldNameDict',
                        modified_result[key], orig_result[key]))

                return modified_result

            @staticmethod
            def getPromotedExtensionArrayName(targetApiName: str, scope: str) -> dict[str, str]:
                orig_result = APISpecificInterfaces.generators.extension_helper_generator.original_getPromotedExtensionArrayName(targetApiName, scope)
                modified_result = {}
                for key in orig_result:
                    modified_result[key] = '{FooBar' + orig_result[key] + 'PromotedVar}'

                    APISpecificInterfaces.addGenSrcDiffRule('vk_extension_helper.h', TestDiffRule(
                        'extension_helper_generator.APISpecific.getPromotedExtensionArrayName',
                        modified_result[key], orig_result[key]))

                return modified_result


        # LayerChassisOutputGenerator
        class layer_chassis_generator:
            @staticmethod
            def getValidationLayerList(targetApiName: str) -> list[dict[str, str]]:
                orig_result = APISpecificInterfaces.generators.layer_chassis_generator.original_getValidationLayerList(targetApiName)
                modified_result = []
                for layer in orig_result:
                    modified_result.append({
                        'include': '{FooBar' + layer['include'] + 'IncludePath}',
                        'class': '{FooBar' + layer['class'] + 'LayerClass}' if layer['class'] != 'ThreadSafety' else layer['class'],
                        'enabled': '{FooBar' + layer['enabled'] + 'EnabledCondition}'
                    })

                    for key in ['include', 'class', 'enabled']:
                        APISpecificInterfaces.addGenSrcDiffRule('chassis.cpp', TestDiffRule(
                            'layer_chassis_generator.APISpecific.getValidationLayerList',
                            modified_result[-1][key], layer[key]))

                return modified_result


            @staticmethod
            def getInstanceExtensionList(targetApiName: str) -> list[str]:
                orig_result = APISpecificInterfaces.generators.layer_chassis_generator.original_getInstanceExtensionList(targetApiName)
                modified_result = []
                for ext in orig_result:
                    modified_result.append('{FooBar' + ext + 'InstanceExtension}')

                    APISpecificInterfaces.addGenSrcDiffRule('chassis.cpp', TestDiffRule(
                        'layer_chassis_generator.APISpecific.getInstanceExtensionList',
                        modified_result[-1].upper(), ext.upper()))

                return modified_result


            @staticmethod
            def getDeviceExtensionList(targetApiName: str) -> list[str]:
                orig_result = APISpecificInterfaces.generators.layer_chassis_generator.original_getDeviceExtensionList(targetApiName)
                modified_result = []
                for ext in orig_result:
                    modified_result.append('{FooBar' + ext + 'DeviceExtension}')

                    APISpecificInterfaces.addGenSrcDiffRule('chassis.cpp', TestDiffRule(
                        'layer_chassis_generator.APISpecific.getDeviceExtensionList',
                        modified_result[-1].upper(), ext.upper()))

                return modified_result


            @staticmethod
            def genInitObjectDispatchVectorSource(targetApiName: str) -> str:
                modified_result = '{FooBarInitObjectDispatchVectorSource}'
                APISpecificInterfaces.addGenSrcDiffRule('chassis_dispatch_helper.h', TestDiffRule(
                    'layer_chassis_generator.APISpecific.genInitObjectDispatchVectorSource',
                    modified_result,
                    APISpecificInterfaces.generators.layer_chassis_generator.original_genInitObjectDispatchVectorSource(targetApiName)))

                return modified_result


        # ObjectTrackerOutputGenerator
        class object_tracker_generator:
            @staticmethod
            def getUndestroyedObjectVUID(targetApiName: str, scope: str) -> str:
                modified_result = '{FooBar' + scope + 'UndestroyedObjectVUID}'
                APISpecificInterfaces.addGenSrcDiffRule('object_tracker.cpp', TestDiffRule(
                    'object_tracker_generator.APISpecific.getUndestroyedObjectVUID',
                    modified_result,
                    APISpecificInterfaces.generators.object_tracker_generator.original_getUndestroyedObjectVUID(targetApiName, scope)))
                return modified_result


            @staticmethod
            def IsImplicitlyDestroyed(targetApiName: str, handleType: str) -> bool:
                orig_implicitly_destroyed = {
                    'VkDisplayKHR': 'instance',
                    'VkDisplayModeKHR': 'instance'
                }

                modified_implicitly_destroyed = {
                    'VkSurfaceKHR': 'instance',
                    'VkCommandBuffer': 'device',
                    'VkImage': 'device',
                    'VkBufferView': 'device'
                }

                if handleType in orig_implicitly_destroyed:
                    scope = orig_implicitly_destroyed[handleType]
                elif handleType in modified_implicitly_destroyed:
                    scope = modified_implicitly_destroyed[handleType]
                else:
                    scope = 'invalid'

                base_src_line = f'skip |= ReportLeaked{scope.capitalize()}Objects({scope}, kVulkanObjectType{handleType[2:]}, error_code);'
                comment_prefix = '// No destroy API or implicitly freed/destroyed -- do not report: '

                if handleType in orig_implicitly_destroyed and handleType not in modified_implicitly_destroyed:
                    APISpecificInterfaces.addGenSrcDiffRule('object_tracker.cpp', TestDiffRule(
                        'object_tracker_generator.APISpecific.IsImplicitlyDestroyed',
                        '    ' + base_src_line,
                        '    ' + comment_prefix + base_src_line))
                if handleType not in orig_implicitly_destroyed and handleType in modified_implicitly_destroyed:
                    APISpecificInterfaces.addGenSrcDiffRule('object_tracker.cpp', TestDiffRule(
                        'object_tracker_generator.APISpecific.IsImplicitlyDestroyed',
                        '    ' + comment_prefix + base_src_line,
                        '    ' + base_src_line))

                return handleType in modified_implicitly_destroyed


            @staticmethod
            def AreAllocVUIDsEnabled(targetApiName: str) -> bool:
                # Unfortunately we cannot do anything better currently but to mimic the caller to get the original VUIDs
                import inspect
                caller_frame = inspect.currentframe().f_back
                caller_instance = caller_frame.f_locals['self']
                param = caller_frame.f_locals['param']

                def mimic_caller(self, param, allocType: str) -> str:
                    lookup_string = '%s-%s' %(param.name, allocType)
                    vuid = self.manual_vuids.get(lookup_string, None)
                    if vuid is not None:
                        return vuid
                    lookup_string = '%s-%s-%s' %(param.type, param.name, allocType)
                    vuid = self.manual_vuids.get(lookup_string, None)
                    if vuid is not None:
                        return vuid
                    return "kVUIDUndefined"

                orig_compatalloc_vuid = mimic_caller(caller_instance, param, 'compatalloc')
                orig_nullalloc_vuid = mimic_caller(caller_instance, param, 'nullalloc')

                # The ValidateDestroyObject call for these objects is manually written
                manual_validate_destroy_object_types = {
                    'VkInstance',
                    'VkDevice',
                    'VkDescriptorPool'
                    'VkDescriptorSet',
                    'VkCommandPool',
                    'VkCommandBuffer',
                    'VkSwapchainKHR'
                }

                was_modified = True
                was_modified = was_modified and param.type not in manual_validate_destroy_object_types
                was_modified = was_modified and (orig_compatalloc_vuid != 'kVUIDUndefined' or orig_nullalloc_vuid != 'kVUIDUndefined')

                if was_modified:
                    APISpecificInterfaces.addGenSrcDiffRule('object_tracker.cpp', TestDiffRule(
                        'object_tracker_generator.APISpecific.AreAllocVUIDsEnabled',
                        f'ValidateDestroyObject({param.name}, kVulkanObjectType{param.type[2:]}, pAllocator, kVUIDUndefined, kVUIDUndefined)',
                        f'ValidateDestroyObject({param.name}, kVulkanObjectType{param.type[2:]}, pAllocator, {orig_compatalloc_vuid}, {orig_nullalloc_vuid})'))

                return False


        # StatelessValidationHelperOutputGenerator
        class stateless_validation_helper_generator:
            @staticmethod
            def genCustomValidation(targetApiName: str, funcName: str, member) -> list[str]:
                if member.type == 'uint32_t' and member.name == 'engineVersion':
                    APISpecificInterfaces.addGenSrcDiffRule('stateless_validation_helper.cpp', TestDiffRule(
                        'stateless_validation_helper_generator.APISpecific.genCustomValidation',
                        '{Foo}{Bar}{Custom}{Validation}', ''))
                    lines = ['{Foo}', '{Bar}', '{Custom}', '{Validation}']
                    return lines
                else:
                    return None
