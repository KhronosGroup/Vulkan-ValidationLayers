#!/bin/bash

# Copyright 2015 The Android Open Source Project
# Copyright (C) 2015 Valve Corporation

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

dir=$(cd -P -- "$(dirname -- "$0")" && pwd -P)
cd $dir

rm -rf generated
mkdir -p generated/include generated/common
HEADERS_REGISTRY_PATH=$dir/third_party/Vulkan-Headers/registry
echo HEADERS_REGISTRY_PATH defined as $(HEADERS_REGISTRY_PATH)

( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH vk_safe_struct.h )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH vk_safe_struct.cpp )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH vk_enum_string_helper.h )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH vk_object_types.h )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH vk_dispatch_table_helper.h )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH thread_check.h )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH parameter_validation.cpp )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH unique_objects_wrappers.h )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH vk_layer_dispatch_table.h )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH vk_extension_helper.h )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH object_tracker.cpp )
( cd generated/include; python3 ../../../scripts/lvl_genvk.py -registry $HEADERS_REGISTRY_PATH/vk.xml -scripts $HEADERS_REGISTRY_PATH vk_typemap_helper.h )

SPIRV_TOOLS_PATH=../../third_party/shaderc/third_party/spirv-tools
SPIRV_TOOLS_UUID=spirv_tools_uuid.txt

set -e

( cd generated/include;

  if [[ -d $SPIRV_TOOLS_PATH ]]; then

    echo Found spirv-tools, using git_dir for external_revision_generator.py

    python3 ../../../scripts/external_revision_generator.py \
      --git_dir $SPIRV_TOOLS_PATH \
      -s SPIRV_TOOLS_COMMIT_ID \
      -o spirv_tools_commit_id.h

  else

    echo No spirv-tools git_dir found, generating UUID for external_revision_generator.py

    # Ensure uuidgen is installed, this should error if not found
    uuidgen --v

    uuidgen > $SPIRV_TOOLS_UUID;
    cat $SPIRV_TOOLS_UUID;
    python3 ../../../scripts/external_revision_generator.py \
      --rev_file $SPIRV_TOOLS_UUID \
      -s SPIRV_TOOLS_COMMIT_ID \
      -o spirv_tools_commit_id.h

  fi
)


exit 0
