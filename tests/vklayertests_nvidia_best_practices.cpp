/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2022 NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Rodrigo Locatti <rlocatti@nvidia.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

// Tests for NVIDIA-specific best practices
const char *kEnableNVIDIAValidation = "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA";
