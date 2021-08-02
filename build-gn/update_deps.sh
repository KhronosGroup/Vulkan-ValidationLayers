#!/bin/sh

# Copyright (c) 2019-2021 LunarG, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Execute at repo root
cd "$(dirname $0)/.."

# Use update_deps.py to update source dependencies from /scripts/known_good.json
scripts/update_deps.py --dir="external" --no-build

# Use gclient to update toolchain dependencies from /build-gn/DEPS (from chromium)
cat << EOF > .gclient
solutions = [
  { "name"        : ".",
    "url"         : "https://github.com/KhronosGroup/Vulkan-ValidationLayers",
    "deps_file"   : "build-gn/DEPS",
    "managed"     : False,
    "custom_deps" : {
    },
    "custom_vars": {},
  },
]
EOF
gclient sync

