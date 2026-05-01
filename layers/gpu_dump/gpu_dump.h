/* Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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
 */
#pragma once
#include <vulkan/vulkan.h>
#include "chassis/layer_object_id.h"
#include "state_tracker/state_tracker.h"

// Wanted 'gpu_dump' but conflicts with EnableFlags::gpu_dump
namespace gpudump {

class Instance : public vvl::InstanceProxy {
  public:
    Instance(vvl::DispatchInstance* dispatch) : vvl::InstanceProxy(dispatch, LayerObjectTypeGpuDump) {}
};

class GpuDump : public vvl::DeviceProxy {
  public:
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

    GpuDump(vvl::DispatchDevice* dev, gpudump::Instance* instance_vo);
    ~GpuDump();

    void Created(vvl::CommandBuffer& cb_state) override;

    std::vector<uint8_t> CopyDataFromMemory(VkDeviceAddress memory_addresss, VkDeviceSize copy_size);
};

}  // namespace gpudump