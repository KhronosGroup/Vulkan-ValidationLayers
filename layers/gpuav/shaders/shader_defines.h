// Copyright (c) 2024-2025 The Khronos Group Inc.
// Copyright (c) 2024-2025 Valve Corporation
// Copyright (c) 2024-2025 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SHADER_DEFINES_H
#define SHADER_DEFINES_H

#ifdef __cplusplus
#define BUFFER_ADDR_FWD_DECL(TypeName)
#define BUFFER_ADDR_DECL(TypeName) VkDeviceAddress
#define BUFFER_ADDR_STRUCT(StructName, alignment) struct StructName
#else
#define BUFFER_ADDR_FWD_DECL(TypeName) layout(buffer_reference, scalar) buffer TypeName;
#define BUFFER_ADDR_DECL(TypeName) TypeName
#define BUFFER_ADDR_STRUCT(StructName, alignment) \
    layout(buffer_reference, buffer_reference_align = alignment, scalar) buffer StructName
#endif

#endif
