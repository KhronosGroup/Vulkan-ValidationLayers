/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

 #include "../framework/layer_validation_tests.h"
 #include "../framework/ray_tracing_objects.h"
 #include "../framework/descriptor_helper.h"
 #include "../framework/shader_helper.h"
 #include "../layers/utils/math_utils.h"
 
 class NegativeRayTracingSpheres : public RayTracingTest {};
 
 TEST_F(NegativeRayTracingSpheres, SpheresMisalignedVertexStride) {
     TEST_DESCRIPTION("Validate vertexStride must be a multiple of the size in bytes of the smallest component of vertexFormat");
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
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
     blas.GetGeometries()[0].SetSpheresVertexStride(1);
     // vertexStride must be a multiple of the size in bytes of the smallest component of vertexFormat
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexStride-10431");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, SpheresInvalidStride) {
     TEST_DESCRIPTION("Validate vertexStride and radiusStride must be less than or equal to 2^32-1");
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
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
     blas.GetGeometries()[0].SetSpheresVertexStride(VkDeviceSize(vvl::kU32Max) + 1);
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexStride-10432");
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexStride-10432");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
 
     // Set valid vertex stride but invalid radius stride
     blas.GetGeometries()[0].SetSpheresVertexStride(3 * sizeof(float));
     blas.GetGeometries()[0].SetSpheresRadiusStride(VkDeviceSize(vvl::kU32Max) + 1);
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexStride-10432");
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexStride-10432");
     blas.BuildCmdBuffer(m_command_buffer.handle(), false);
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, SpheresInvalidVertexFormat) {
     TEST_DESCRIPTION(
         "Validate the format features of vertexFormat must contain VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR");
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
 
     // Set an invalid vertex format
     const VkFormat spheres_vertex_format = VK_FORMAT_R32_SFLOAT;
     VkFormatProperties spheres_format_props{};
     vk::GetPhysicalDeviceFormatProperties(m_device->Physical(), spheres_vertex_format, &spheres_format_props);
 
     if (spheres_format_props.bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR) {
         GTEST_SKIP()
             << "Hard coded vertex format has VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR, skipping test.";
     }
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
     blas.GetGeometries()[0].SetSpheresVertexFormat(spheres_vertex_format);
 
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexFormat-10434");
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexFormat-10434");
     blas.BuildCmdBuffer(m_command_buffer.handle(), false);
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, SpheresInvalidRadiusFormat) {
     TEST_DESCRIPTION(
         "Validate The format features of radiusFormat must contain "
         "VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV");
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
 
     // Set an invalid radius format
     const VkFormat spheres_vertex_format = VK_FORMAT_R8G8_UNORM;
     VkFormatProperties spheres_format_props{};
     vk::GetPhysicalDeviceFormatProperties(m_device->Physical(), spheres_vertex_format, &spheres_format_props);
     if (spheres_format_props.bufferFeatures & VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV) {
         GTEST_SKIP()
             << "Hard coded vertex format has VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV, skipping test.";
     }
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
     blas.GetGeometries()[0].SetSpheresRadiusFormat(spheres_vertex_format);
 
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-radiusFormat-10435");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, SpheresInvalidIndexType) {
     TEST_DESCRIPTION("indexType must be VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or VK_INDEX_TYPE_NONE_KHR");
     SetTargetApiVersion(VK_API_VERSION_1_3);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::accelerationStructure);
     AddRequiredFeature(vkt::Feature::rayQuery);
     AddRequiredFeature(vkt::Feature::spheres);
     AddRequiredFeature(vkt::Feature::linearSweptSpheres);
     AddRequiredExtensions(VK_NV_RAY_TRACING_LINEAR_SWEPT_SPHERES_EXTENSION_NAME);
     AddRequiredExtensions(VK_KHR_INDEX_TYPE_UINT8_EXTENSION_NAME);
     AddRequiredFeature(vkt::Feature::indexTypeUint8);
     RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
     RETURN_IF_SKIP(InitState());
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
     // Set invalid index type (UINT8_KHR)
     blas.GetGeometries()[0].SetSpheresIndexType(VK_INDEX_TYPE_UINT8_KHR);
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-indexData-10437");
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-indexData-10437");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, LSSpheresMisalignedVertexStride) {
     TEST_DESCRIPTION("vertexStride must be a multiple of the size in bytes of the smallest component of vertexFormat");
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
 
     m_command_buffer.Begin();
     // Set an invalid vertex stride
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
     blas.GetGeometries()[0].SetLSSpheresVertexStride(1);
 
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexStride-10421");
     blas.BuildCmdBuffer(m_command_buffer.handle(), false);
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, LSSpheresInvalidStride) {
     TEST_DESCRIPTION("vertexStride and radiusStride must be less than or equal to 2^32-1");
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
 
     m_command_buffer.Begin();
     // Set vertex stride larger than 2^32
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
     blas.GetGeometries()[0].SetLSSpheresVertexStride(VkDeviceSize(vvl::kU32Max) + 1);
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
 
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexStride-10422");
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexStride-10422");
     blas.BuildCmdBuffer(m_command_buffer.handle(), false);
     m_errorMonitor->VerifyFound();
     blas.GetGeometries()[0].SetLSSpheresVertexStride(3 * sizeof(float));
     // Set radius stride larger than 2^32
     blas.GetGeometries()[0].SetLSSpheresRadiusStride(VkDeviceSize(vvl::kU32Max) + 1);
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
 
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexStride-10422");
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexStride-10422");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, LSSpheresInvalidVertexFormat) {
     TEST_DESCRIPTION(
         "The format features of vertexFormat must contain VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR");
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
 
     // set invalid vertexFormat
     const VkFormat lsspheres_vertex_format = VK_FORMAT_R32_SFLOAT;
     VkFormatProperties lsspheres_format_props{};
     vk::GetPhysicalDeviceFormatProperties(m_device->Physical(), lsspheres_vertex_format, &lsspheres_format_props);
     if (lsspheres_format_props.bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR) {
         GTEST_SKIP()
             << "Hard coded vertex format has VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR, skipping test.";
     }
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
     blas.GetGeometries()[0].SetLSSpheresVertexFormat(lsspheres_vertex_format);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexFormat-10423");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, LSSpheresInvalidRadiusFormat) {
     TEST_DESCRIPTION(
         "The format features of radiusFormat must contain VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV");
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
 
     // set invalid radiusFormat
     const VkFormat lsspheres_vertex_format = VK_FORMAT_R8G8_UNORM;
     VkFormatProperties lsspheres_format_props{};
     vk::GetPhysicalDeviceFormatProperties(m_device->Physical(), lsspheres_vertex_format, &lsspheres_format_props);
     if (lsspheres_format_props.bufferFeatures & VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV) {
         GTEST_SKIP()
             << "Hard coded vertex format has VK_FORMAT_FEATURE_2_ACCELERATION_STRUCTURE_RADIUS_BUFFER_BIT_NV, skipping test.";
     }
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
     blas.GetGeometries()[0].SetLSSpheresRadiusFormat(lsspheres_vertex_format);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-radiusFormat-10424");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, LSSpheresInvalidIndexType) {
     TEST_DESCRIPTION("indexType must be VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32, or VK_INDEX_TYPE_NONE_KHR");
     SetTargetApiVersion(VK_API_VERSION_1_3);
     AddRequiredExtensions(VK_KHR_INDEX_TYPE_UINT8_EXTENSION_NAME);
     AddRequiredFeature(vkt::Feature::indexTypeUint8);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::accelerationStructure);
     AddRequiredFeature(vkt::Feature::rayQuery);
     AddRequiredFeature(vkt::Feature::spheres);
     AddRequiredFeature(vkt::Feature::linearSweptSpheres);
     AddRequiredExtensions(VK_NV_RAY_TRACING_LINEAR_SWEPT_SPHERES_EXTENSION_NAME);
     RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
     RETURN_IF_SKIP(InitState());
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
     blas.GetGeometries()[0].SetLSSpheresIndexType(VK_INDEX_TYPE_UINT8_EXT);
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-indexData-10428");
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-indexData-10428");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, LSSpheresIndexDataNull) {
     TEST_DESCRIPTION("When indexingMode is VK_RAY_TRACING_LSS_INDEXING_MODE_SUCCESSIVE_NV, indexData must be provided");
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
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
     blas.GetGeometries()[0].SetLSSpheresIndexingMode(VK_RAY_TRACING_LSS_INDEXING_MODE_SUCCESSIVE_NV);
     blas.GetGeometries()[0].SetLSSpheresIndexDataNull();
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-indexingMode-10427");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, SpheresVertexDataNull) {
     TEST_DESCRIPTION("The memory address in vertexData must not be 0 or `NULL'");
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
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
 
     blas.GetGeometries()[0].SetSpheresVertexAddressNull();
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-vertexData-10430");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, SpheresRadiusDataNull) {
     TEST_DESCRIPTION("The memory address in radiusData must not be 0 or `NULL'");
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
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
     blas.GetGeometries()[0].SetSpheresRadiusAddressNull();
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-radiusData-10433");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, LSSpheresVertexDataNull) {
     TEST_DESCRIPTION("The memory address in vertexData must not be 0 or `NULL'");
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
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
 
     blas.GetGeometries()[0].SetLSSpheresVertexAddressNull();
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-vertexData-10420");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, LSSpheresRadiusDataNull) {
     TEST_DESCRIPTION("The memory address in radiusData must not be 0 or `NULL'");
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
 
     m_command_buffer.Begin();
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
     blas.GetGeometries()[0].SetLSSpheresRadiusAddressNull();
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-radiusData-10425");
     blas.BuildCmdBuffer(m_command_buffer.handle());
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, LSSpheresFeatureDisabled) {
     TEST_DESCRIPTION("The linearSweptSpheres feature must be enabled");
     SetTargetApiVersion(VK_API_VERSION_1_3);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::accelerationStructure);
     AddRequiredFeature(vkt::Feature::rayQuery);
     AddRequiredFeature(vkt::Feature::spheres);
 
     AddRequiredExtensions(VK_NV_RAY_TRACING_LINEAR_SWEPT_SPHERES_EXTENSION_NAME);
     RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
     RETURN_IF_SKIP(InitState());
 
     m_command_buffer.Begin();
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometryLinearSweptSpheresDataNV-None-10419");
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::LSSpheres);
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
 
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 
 TEST_F(NegativeRayTracingSpheres, SpheresFeatureDisabled) {
     TEST_DESCRIPTION("The spheres feature must be enabled");
     SetTargetApiVersion(VK_API_VERSION_1_3);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
     AddRequiredFeature(vkt::Feature::accelerationStructure);
     AddRequiredFeature(vkt::Feature::rayQuery);
     AddRequiredFeature(vkt::Feature::linearSweptSpheres);
     AddRequiredExtensions(VK_NV_RAY_TRACING_LINEAR_SWEPT_SPHERES_EXTENSION_NAME);
     RETURN_IF_SKIP(InitFrameworkForRayTracingTest());
     RETURN_IF_SKIP(InitState());
 
     m_command_buffer.Begin();
     m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureGeometrySpheresDataNV-None-10429");
     auto blas = vkt::as::blueprint::BuildGeometryInfoSimpleOnDeviceBottomLevel(*m_device, vkt::as::GeometryKHR::Type::Spheres);
     blas.SetUpdateDstAccelStructSizeBeforeBuild(false);
     m_errorMonitor->VerifyFound();
     m_command_buffer.End();
 }
 