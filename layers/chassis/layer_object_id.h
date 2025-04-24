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
 ****************************************************************************/

#pragma once
// Layer object type identifiers
// While this doesn't enforce the order, we enforce the order in InitValidationObjects() based on this
enum LayerObjectTypeId {
    LayerObjectTypeParameterValidation,  // Instance or device parameter validation layer object
    LayerObjectTypeThreading,            // Instance or device threading layer object
    LayerObjectTypeObjectTracker,        // Instance or device object tracker layer object
    LayerObjectTypeStateTracker,         // Shared state tracker
    LayerObjectTypeCoreValidation,       // Instance or device core validation layer object
    LayerObjectTypeBestPractices,        // Instance or device best practices layer object
    LayerObjectTypeGpuAssisted,          // Instance or device gpu assisted validation layer object
    LayerObjectTypeSyncValidation,       // Instance or device synchronization validation layer object
    LayerObjectTypeMaxEnum,              // Max enum count
};
