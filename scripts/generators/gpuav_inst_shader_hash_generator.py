#!/usr/bin/env python3
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
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

import os
import subprocess
from generators.base_generator import BaseGenerator

class GpuAvInstShaderHashOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        shader_file = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'layers', 'gpu_shaders', 'inst_functions.comp'))
        result = subprocess.run(["git", "hash-object", shader_file], capture_output=True, text=True)
        git_hash = result.stdout.rstrip('\n')

        try:
            str_as_int = int(git_hash, 16)
        except ValueError:
            raise ValueError(f'value for INST_SHADER_GIT_HASH ({git_hash}) must be a SHA1 hash.')
        if len(git_hash) != 40:
            raise ValueError(f'value for INST_SHADER_GIT_HASH ({git_hash}) must be a SHA1 hash.')

        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2023 The Khronos Group Inc.
            * Copyright (c) 2015-2023 Valve Corporation
            * Copyright (c) 2015-2023 LunarG, Inc.
            * Copyright (c) 2015-2023 Google Inc.
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
            ****************************************************************************/

            #pragma once

            ''')

        out.append(f'#define INST_SHADER_GIT_HASH "{git_hash}"')
        self.write("".join(out))
