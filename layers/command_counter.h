/* Copyright (C) 2019 Intel Corporation.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 *
 * Author: Lionel Landwerlin <lionel.g.landwerlin@intel.com>
 * Author: Nadav Geva <nadav.geva@amd.com>
 */

#pragma once
#include "core_validation.h"

class CommandCounter : public ValidationObject {
  public:
    CommandCounter(ValidationStateTracker *trackerObject) : trackerObject(trackerObject) {container_type = LayerObjectTypeCommandCounter;}
    virtual ~CommandCounter() {}

    virtual write_lock_guard_t write_lock() { return trackerObject->write_lock(); }

#include "command_counter_helper.h"

  private:
    ValidationStateTracker *trackerObject;
};
