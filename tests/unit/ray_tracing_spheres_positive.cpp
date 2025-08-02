/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

 #include "../framework/layer_validation_tests.h"
 #include "../framework/ray_tracing_objects.h"
 #include "../framework/feature_requirements.h"
 #include "../framework/descriptor_helper.h"
 #include "../framework/pipeline_helper.h"
 #include "utils/math_utils.h"
 #include <algorithm>
 
 class PositiveRayTracingSpheres : public RayTracingTest {};
 
 TEST_F(PositiveRayTracingSpheres, AccelerationStructureGeometrySpheresData) {
     TEST_DESCRIPTION("Test AccelerationStructureGeometrySpheresData structure");
     SetTargetApiVersion(VK_API_VERSION_1_3);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::accelerationStructure);
     AddRequiredFeature(vkt::Feature::rayQuery);
     AddRequiredFeature(vkt::Feature::spheres);
     AddRequiredFeature(vkt::Feature::linearSweptSpheres);
     AddRequiredExtensions(VK_NV_RAY_TRACING_LINEAR_SWEPT_SPHERES_EXTENSION_NAME);
     RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
     RETURN_IF_SKIP(InitState());
 
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 
     // Host build
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 
     // Device build, with index type VK_INDEX_TYPE_UINT16
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
         blas.GetGeometries()[0].SetSpheresIndexType(VK_INDEX_TYPE_UINT16);
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 
     // Device build, with index type VK_INDEX_TYPE_UINT32
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
         blas.GetGeometries()[0].SetSpheresIndexType(VK_INDEX_TYPE_UINT32);
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 
     // Device build, with index type VK_INDEX_TYPE_NONE_KHR
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
         blas.GetGeometries()[0].SetSpheresIndexType(VK_INDEX_TYPE_NONE_KHR);
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 
     // Device build, SetSpheresVertexStride with stride = sizeof(float)
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
         blas.GetGeometries()[0].SetSpheresVertexStride(sizeof(float));
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 }
 
 TEST_F(PositiveRayTracingSpheres, AccelerationStructureGeometryLinearSweptSpheresData) {
     TEST_DESCRIPTION("Test AccelerationStructureGeometryLinearSweptSpheresData structure");
     SetTargetApiVersion(VK_API_VERSION_1_3);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::accelerationStructure);
     AddRequiredFeature(vkt::Feature::rayQuery);
     AddRequiredFeature(vkt::Feature::spheres);
     AddRequiredFeature(vkt::Feature::linearSweptSpheres);
     AddRequiredExtensions(VK_NV_RAY_TRACING_LINEAR_SWEPT_SPHERES_EXTENSION_NAME);
     RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
     RETURN_IF_SKIP(InitState());
 
     // Host build
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnHostBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 
     // Device build, with index type VK_INDEX_TYPE_UINT16
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
         blas.GetGeometries()[0].SetLSSpheresIndexType(VK_INDEX_TYPE_UINT16);
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 
     // Device build, with index type VK_INDEX_TYPE_UINT32
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
         blas.GetGeometries()[0].SetLSSpheresIndexType(VK_INDEX_TYPE_UINT32);
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 
     // Device build, with index type VK_INDEX_TYPE_NONE_KHR
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
         blas.GetGeometries()[0].SetLSSpheresIndexType(VK_INDEX_TYPE_NONE_KHR);
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 
     // Device build, SetLSSpheresVertexStride with stride = sizeof(float)
     {
         m_command_buffer.Begin();
         auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
         blas.GetGeometries()[0].SetLSSpheresVertexStride(sizeof(float));
         blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
         blas.BuildCmdBuffer(m_command_buffer.handle());
         m_command_buffer.End();
     }
 }