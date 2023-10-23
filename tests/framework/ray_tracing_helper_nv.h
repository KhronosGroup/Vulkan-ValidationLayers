/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "layer_validation_tests.h"

namespace nv {
namespace rt {

// DEPRECATED: This is part of the legacy ray tracing framework, now only used in the old nvidia ray tracing extension tests.
void GetSimpleGeometryForAccelerationStructureTests(const vkt::Device& device, vkt::Buffer* vbo, vkt::Buffer* ibo,
                                                    VkGeometryNV* geometry, VkDeviceSize offset = 0,
                                                    bool buffer_device_address = false);
}  // namespace rt
}  // namespace nv
