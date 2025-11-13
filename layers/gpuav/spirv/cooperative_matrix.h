/* Copyright (c) 2024-2025 LunarG, Inc.
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

#include <stdint.h>

namespace gpuav {
namespace spirv {

struct Type;

// Information about a given coop mat load/store bundled with the type information
struct CooperativeMatrixAccess {
    bool used = false;
    // OpTypeCooperativeMatrixKHR
    const Type* type = nullptr;
    uint32_t component_size = 0;
    uint32_t rows = 0;
    uint32_t columns = 0;
    // OpCooperativeMatrixLoadKHR/OpCooperativeMatrixStoreKHR
    bool is_load;
    uint32_t stride_id = 0;
    uint32_t stride_value = 0;  // may be zero if value is not a OpConstant
    bool is_row_major;

    uint32_t Size() const {
        if (stride_value == 0) {
            // This value is dynamic and can't be calculated
            return 0;
        } else if (is_row_major) {
            return ((rows - 1) * stride_value + columns) * component_size;
        } else {
            return ((columns - 1) * stride_value + rows) * component_size;
        }
    }
};

}  // namespace spirv
}  // namespace gpuav