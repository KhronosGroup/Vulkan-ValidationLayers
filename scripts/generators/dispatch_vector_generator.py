#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
# Copyright (c) 2023-2024 RasterGrid Kft.
#
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
#
# This script generates the dispatch portion of a factory layer which intercepts
# all Vulkan  functions. The resultant factory layer allows rapid development of
# layers and interceptors.

import os
from base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper
# Normally we don't like to have generator depend on other generator
# But for this, the logic makes sense in 2 seperate file and its only including encapsulated data
from generators.dispatch_object_generator import APISpecific

class DispatchVectorGenerator(BaseGenerator):
    # will skip all 3 functions
    skip_intercept_id_functions = (
        'vkGetDeviceProcAddr',
        'vkDestroyDevice',
        'vkCreateValidationCacheEXT',
        'vkDestroyValidationCacheEXT',
        'vkMergeValidationCachesEXT',
        'vkGetValidationCacheDataEXT',
        # have all 3 calls have dual signatures being used
        'vkCreateShaderModule',
        'vkCreateShadersEXT',
        'vkCreateGraphicsPipelines',
        'vkCreateComputePipelines',
        'vkCreateRayTracingPipelinesNV',
        'vkCreateRayTracingPipelinesKHR',
    )

    # We need to skip any signatures that pass around chassis_modification_state structs
    # and therefore can't easily create the intercept id
    skip_intercept_id_pre_validate = (
        'vkAllocateDescriptorSets'
    )
    skip_intercept_id_pre_record = (
        'vkCreatePipelineLayout',
        'vkCreateBuffer',
        'vkGetShaderBinaryDataEXT',
    )
    skip_intercept_id_post_record = (
        'vkAllocateDescriptorSets'
    )

    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 The Khronos Group Inc.
            * Copyright (c) 2015-2025 Valve Corporation
            * Copyright (c) 2015-2025 LunarG, Inc.
            * Copyright (c) 2015-2024 Google Inc.
            * Copyright (c) 2023-2024 RasterGrid Kft.
            *
            * Licensed under the Apache License, Version 2.0 (the "License");
            * you may not use this file except in compliance with the License.
            * You may obtain a copy of the License at
            *
            *     http://www.apache.org/licenses/LICENSE-2.0
            *
            * Unless required by applicable law or agreed to in writing, software
            * distributed under the License is distributed on an "AS IS" BASIS,
            * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
            * See the License for the specific language governing permissions and
            * limitations under the License.
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'dispatch_vector.h':
            self.generateHeader()
        elif self.filename == 'dispatch_vector.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            // This source code creates dispatch vectors for each chassis api intercept,
            // i.e., PreCallValidateFoo, PreCallRecordFoo, PostCallRecordFoo, etc., ensuring that
            // each vector contains only the validation objects that override that particular base
            // class virtual function. Preventing non-overridden calls from reaching the default
            // functions saved about 5% in multithreaded applications.

            ''')

        out.append('typedef enum InterceptId{\n')
        for command in [x for x in self.vk.commands.values() if not x.instance and x.name not in self.skip_intercept_id_functions]:
            if command.name not in self.skip_intercept_id_pre_validate:
                out.append(f'    InterceptIdPreCallValidate{command.name[2:]},\n')
            if command.name not in self.skip_intercept_id_pre_record:
                out.append(f'    InterceptIdPreCallRecord{command.name[2:]},\n')
            if command.name not in self.skip_intercept_id_post_record:
                out.append(f'    InterceptIdPostCallRecord{command.name[2:]},\n')
        out.append('    InterceptIdCount,\n')
        out.append('} InterceptId;\n')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            // This source code creates dispatch vectors for each chassis api intercept,
            // i.e., PreCallValidateFoo, PreCallRecordFoo, PostCallRecordFoo, etc., ensuring that
            // each vector contains only the validation objects that override that particular base
            // class virtual function. Preventing non-overridden calls from reaching the default
            // functions saved about 5% in multithreaded applications.

            #include "generated/dispatch_vector.h"
            #include "chassis/dispatch_object.h"
            ''')

        layer_list = APISpecific.getValidationLayerList(self.targetApiName)
        for layer in layer_list:
            include = layer['include']
            out.append(f'#include "{include}"\n')

        out.append('''
            namespace vvl {
            namespace dispatch {

            void Device::InitObjectDispatchVectors() {

            #define BUILD_DISPATCH_VECTOR(name) \\
                init_object_dispatch_vector(InterceptId ## name, \\
                                            typeid(&vvl::base::Device::name), \\
        ''')
        params = [f'typeid(&{layer["device"]}::name)' for layer in layer_list]
        out.append(',\\\n'.join(params))
        out.append(', false);')
        out.append('''
            #define BUILD_DESTROY_DISPATCH_VECTOR(name) \\
                init_object_dispatch_vector(InterceptId ## name, \\
                                            typeid(&vvl::base::Device::name), \\
        ''')
        out.append(',\\\n'.join(params))
        out.append(', true);\n')
        out.append('''
            auto init_object_dispatch_vector = [this](InterceptId id, const std::type_info& vo_typeid,
        ''')
        for i in range(len(layer_list)):
            type_name = layer_list[i]['type'].replace('LayerObjectType', '')
            lambda_param = ''.join([c for c in type_name if c.isupper()]).lower() + '_typeid'
            layer_list[i]['lambda_param'] = lambda_param
            out.append(f'const std::type_info& {lambda_param},\n')

        out.append('bool is_destroy) {\n')
        out.append('''
            vvl::base::Device *state_tracker = nullptr;
            auto *intercept_vector = &this->intercept_vectors[id];
            for (auto& vo: this->object_dispatch) {
                auto *item = vo.get();
                switch (item->container_type) {
            ''')
        for layer in layer_list:
            if layer['type'] == 'LayerObjectTypeStateTracker':
               out.append(f'''
                   case {layer['type']}:
                       if ({layer['lambda_param']} != vo_typeid) {{
                           // For destroy/free commands, the state tracker must run last so that
                           // other validation objects can still access the state object which
                           // is being destroyed.
                           if (is_destroy) {{
                               state_tracker = item;
                           }} else {{
                               intercept_vector->push_back(item);
                           }}
                       }}
                       break;
                   ''')
            else:
               out.append(f'''
                   case {layer['type']}:
                       if ({layer['lambda_param']} != vo_typeid) intercept_vector->push_back(item);
                       break;
                   ''')
        out.append('''
            case LayerObjectTypeMaxEnum:
                /* Chassis codegen needs to be updated for unknown validation object type */
                assert(0);
                break;
            }
        }
        if (state_tracker) {
            intercept_vector->push_back(state_tracker);
        }
    };

    intercept_vectors.resize(InterceptIdCount);
    ''')


        guard_helper = PlatformGuardHelper()
        for command in [x for x in self.vk.commands.values() if not x.instance and x.name not in self.skip_intercept_id_functions]:
            out.extend(guard_helper.add_guard(command.protect))
            macro = 'BUILD_DESTROY_DISPATCH_VECTOR' if ('Destroy' in command.name or 'Free' in command.name) else 'BUILD_DISPATCH_VECTOR'
            if command.name not in self.skip_intercept_id_pre_validate:
                out.append(f'    {macro}(PreCallValidate{command.name[2:]});\n')
            if command.name not in self.skip_intercept_id_pre_record:
                out.append(f'    {macro}(PreCallRecord{command.name[2:]});\n')
            if command.name not in self.skip_intercept_id_post_record:
                out.append(f'    {macro}(PostCallRecord{command.name[2:]});\n')
        out.extend(guard_helper.add_guard(None))
        out.append('}\n')
        out.append('} // namespace dispatch\n')
        out.append('} // namespace vvl\n')
        self.write("".join(out))
