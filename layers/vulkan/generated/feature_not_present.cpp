// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See feature_not_present.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "chassis/dispatch_object.h"
#include "generated/dispatch_functions.h"
#include "error_message/error_location.h"

namespace vvl {
namespace dispatch {

void Instance::ReportErrorFeatureNotPresent(VkPhysicalDevice gpu, const VkDeviceCreateInfo &create_info) {
    std::stringstream ss;
    ss << "returned VK_ERROR_FEATURE_NOT_PRESENT because the following features were not supported on this physical device:\n";

    // First do 1.0 VkPhysicalDeviceFeatures
    {
        const auto *features2 = vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(create_info.pNext);
        const VkPhysicalDeviceFeatures &enabling =
            create_info.pEnabledFeatures ? *create_info.pEnabledFeatures : features2->features;

        VkPhysicalDeviceFeatures supported = {};
        DispatchGetPhysicalDeviceFeatures(gpu, &supported);
        if (enabling.robustBufferAccess && !supported.robustBufferAccess) {
            ss << "VkPhysicalDeviceFeatures::robustBufferAccess is not supported\n";
        }
        if (enabling.fullDrawIndexUint32 && !supported.fullDrawIndexUint32) {
            ss << "VkPhysicalDeviceFeatures::fullDrawIndexUint32 is not supported\n";
        }
        if (enabling.imageCubeArray && !supported.imageCubeArray) {
            ss << "VkPhysicalDeviceFeatures::imageCubeArray is not supported\n";
        }
        if (enabling.independentBlend && !supported.independentBlend) {
            ss << "VkPhysicalDeviceFeatures::independentBlend is not supported\n";
        }
        if (enabling.geometryShader && !supported.geometryShader) {
            ss << "VkPhysicalDeviceFeatures::geometryShader is not supported\n";
        }
        if (enabling.tessellationShader && !supported.tessellationShader) {
            ss << "VkPhysicalDeviceFeatures::tessellationShader is not supported\n";
        }
        if (enabling.sampleRateShading && !supported.sampleRateShading) {
            ss << "VkPhysicalDeviceFeatures::sampleRateShading is not supported\n";
        }
        if (enabling.dualSrcBlend && !supported.dualSrcBlend) {
            ss << "VkPhysicalDeviceFeatures::dualSrcBlend is not supported\n";
        }
        if (enabling.logicOp && !supported.logicOp) {
            ss << "VkPhysicalDeviceFeatures::logicOp is not supported\n";
        }
        if (enabling.multiDrawIndirect && !supported.multiDrawIndirect) {
            ss << "VkPhysicalDeviceFeatures::multiDrawIndirect is not supported\n";
        }
        if (enabling.drawIndirectFirstInstance && !supported.drawIndirectFirstInstance) {
            ss << "VkPhysicalDeviceFeatures::drawIndirectFirstInstance is not supported\n";
        }
        if (enabling.depthClamp && !supported.depthClamp) {
            ss << "VkPhysicalDeviceFeatures::depthClamp is not supported\n";
        }
        if (enabling.depthBiasClamp && !supported.depthBiasClamp) {
            ss << "VkPhysicalDeviceFeatures::depthBiasClamp is not supported\n";
        }
        if (enabling.fillModeNonSolid && !supported.fillModeNonSolid) {
            ss << "VkPhysicalDeviceFeatures::fillModeNonSolid is not supported\n";
        }
        if (enabling.depthBounds && !supported.depthBounds) {
            ss << "VkPhysicalDeviceFeatures::depthBounds is not supported\n";
        }
        if (enabling.wideLines && !supported.wideLines) {
            ss << "VkPhysicalDeviceFeatures::wideLines is not supported\n";
        }
        if (enabling.largePoints && !supported.largePoints) {
            ss << "VkPhysicalDeviceFeatures::largePoints is not supported\n";
        }
        if (enabling.alphaToOne && !supported.alphaToOne) {
            ss << "VkPhysicalDeviceFeatures::alphaToOne is not supported\n";
        }
        if (enabling.multiViewport && !supported.multiViewport) {
            ss << "VkPhysicalDeviceFeatures::multiViewport is not supported\n";
        }
        if (enabling.samplerAnisotropy && !supported.samplerAnisotropy) {
            ss << "VkPhysicalDeviceFeatures::samplerAnisotropy is not supported\n";
        }
        if (enabling.textureCompressionETC2 && !supported.textureCompressionETC2) {
            ss << "VkPhysicalDeviceFeatures::textureCompressionETC2 is not supported\n";
        }
        if (enabling.textureCompressionASTC_LDR && !supported.textureCompressionASTC_LDR) {
            ss << "VkPhysicalDeviceFeatures::textureCompressionASTC_LDR is not supported\n";
        }
        if (enabling.textureCompressionBC && !supported.textureCompressionBC) {
            ss << "VkPhysicalDeviceFeatures::textureCompressionBC is not supported\n";
        }
        if (enabling.occlusionQueryPrecise && !supported.occlusionQueryPrecise) {
            ss << "VkPhysicalDeviceFeatures::occlusionQueryPrecise is not supported\n";
        }
        if (enabling.pipelineStatisticsQuery && !supported.pipelineStatisticsQuery) {
            ss << "VkPhysicalDeviceFeatures::pipelineStatisticsQuery is not supported\n";
        }
        if (enabling.vertexPipelineStoresAndAtomics && !supported.vertexPipelineStoresAndAtomics) {
            ss << "VkPhysicalDeviceFeatures::vertexPipelineStoresAndAtomics is not supported\n";
        }
        if (enabling.fragmentStoresAndAtomics && !supported.fragmentStoresAndAtomics) {
            ss << "VkPhysicalDeviceFeatures::fragmentStoresAndAtomics is not supported\n";
        }
        if (enabling.shaderTessellationAndGeometryPointSize && !supported.shaderTessellationAndGeometryPointSize) {
            ss << "VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize is not supported\n";
        }
        if (enabling.shaderImageGatherExtended && !supported.shaderImageGatherExtended) {
            ss << "VkPhysicalDeviceFeatures::shaderImageGatherExtended is not supported\n";
        }
        if (enabling.shaderStorageImageExtendedFormats && !supported.shaderStorageImageExtendedFormats) {
            ss << "VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats is not supported\n";
        }
        if (enabling.shaderStorageImageMultisample && !supported.shaderStorageImageMultisample) {
            ss << "VkPhysicalDeviceFeatures::shaderStorageImageMultisample is not supported\n";
        }
        if (enabling.shaderStorageImageReadWithoutFormat && !supported.shaderStorageImageReadWithoutFormat) {
            ss << "VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat is not supported\n";
        }
        if (enabling.shaderStorageImageWriteWithoutFormat && !supported.shaderStorageImageWriteWithoutFormat) {
            ss << "VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat is not supported\n";
        }
        if (enabling.shaderUniformBufferArrayDynamicIndexing && !supported.shaderUniformBufferArrayDynamicIndexing) {
            ss << "VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing is not supported\n";
        }
        if (enabling.shaderSampledImageArrayDynamicIndexing && !supported.shaderSampledImageArrayDynamicIndexing) {
            ss << "VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing is not supported\n";
        }
        if (enabling.shaderStorageBufferArrayDynamicIndexing && !supported.shaderStorageBufferArrayDynamicIndexing) {
            ss << "VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing is not supported\n";
        }
        if (enabling.shaderStorageImageArrayDynamicIndexing && !supported.shaderStorageImageArrayDynamicIndexing) {
            ss << "VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing is not supported\n";
        }
        if (enabling.shaderClipDistance && !supported.shaderClipDistance) {
            ss << "VkPhysicalDeviceFeatures::shaderClipDistance is not supported\n";
        }
        if (enabling.shaderCullDistance && !supported.shaderCullDistance) {
            ss << "VkPhysicalDeviceFeatures::shaderCullDistance is not supported\n";
        }
        if (enabling.shaderFloat64 && !supported.shaderFloat64) {
            ss << "VkPhysicalDeviceFeatures::shaderFloat64 is not supported\n";
        }
        if (enabling.shaderInt64 && !supported.shaderInt64) {
            ss << "VkPhysicalDeviceFeatures::shaderInt64 is not supported\n";
        }
        if (enabling.shaderInt16 && !supported.shaderInt16) {
            ss << "VkPhysicalDeviceFeatures::shaderInt16 is not supported\n";
        }
        if (enabling.shaderResourceResidency && !supported.shaderResourceResidency) {
            ss << "VkPhysicalDeviceFeatures::shaderResourceResidency is not supported\n";
        }
        if (enabling.shaderResourceMinLod && !supported.shaderResourceMinLod) {
            ss << "VkPhysicalDeviceFeatures::shaderResourceMinLod is not supported\n";
        }
        if (enabling.sparseBinding && !supported.sparseBinding) {
            ss << "VkPhysicalDeviceFeatures::sparseBinding is not supported\n";
        }
        if (enabling.sparseResidencyBuffer && !supported.sparseResidencyBuffer) {
            ss << "VkPhysicalDeviceFeatures::sparseResidencyBuffer is not supported\n";
        }
        if (enabling.sparseResidencyImage2D && !supported.sparseResidencyImage2D) {
            ss << "VkPhysicalDeviceFeatures::sparseResidencyImage2D is not supported\n";
        }
        if (enabling.sparseResidencyImage3D && !supported.sparseResidencyImage3D) {
            ss << "VkPhysicalDeviceFeatures::sparseResidencyImage3D is not supported\n";
        }
        if (enabling.sparseResidency2Samples && !supported.sparseResidency2Samples) {
            ss << "VkPhysicalDeviceFeatures::sparseResidency2Samples is not supported\n";
        }
        if (enabling.sparseResidency4Samples && !supported.sparseResidency4Samples) {
            ss << "VkPhysicalDeviceFeatures::sparseResidency4Samples is not supported\n";
        }
        if (enabling.sparseResidency8Samples && !supported.sparseResidency8Samples) {
            ss << "VkPhysicalDeviceFeatures::sparseResidency8Samples is not supported\n";
        }
        if (enabling.sparseResidency16Samples && !supported.sparseResidency16Samples) {
            ss << "VkPhysicalDeviceFeatures::sparseResidency16Samples is not supported\n";
        }
        if (enabling.sparseResidencyAliased && !supported.sparseResidencyAliased) {
            ss << "VkPhysicalDeviceFeatures::sparseResidencyAliased is not supported\n";
        }
        if (enabling.variableMultisampleRate && !supported.variableMultisampleRate) {
            ss << "VkPhysicalDeviceFeatures::variableMultisampleRate is not supported\n";
        }
        if (enabling.inheritedQueries && !supported.inheritedQueries) {
            ss << "VkPhysicalDeviceFeatures::inheritedQueries is not supported\n";
        }
    }
    VkPhysicalDeviceFeatures2 features_2 = vku::InitStructHelper();
    for (const VkBaseInStructure *current = static_cast<const VkBaseInStructure *>(create_info.pNext); current != nullptr;
         current = current->pNext) {
        switch (current->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES: {
                VkPhysicalDevice16BitStorageFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevice16BitStorageFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDevice16BitStorageFeatures *>(current);
                if (enabling->storageBuffer16BitAccess && !supported.storageBuffer16BitAccess) {
                    ss << "VkPhysicalDevice16BitStorageFeatures::storageBuffer16BitAccess is not supported\n";
                }
                if (enabling->uniformAndStorageBuffer16BitAccess && !supported.uniformAndStorageBuffer16BitAccess) {
                    ss << "VkPhysicalDevice16BitStorageFeatures::uniformAndStorageBuffer16BitAccess is not supported\n";
                }
                if (enabling->storagePushConstant16 && !supported.storagePushConstant16) {
                    ss << "VkPhysicalDevice16BitStorageFeatures::storagePushConstant16 is not supported\n";
                }
                if (enabling->storageInputOutput16 && !supported.storageInputOutput16) {
                    ss << "VkPhysicalDevice16BitStorageFeatures::storageInputOutput16 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT: {
                VkPhysicalDevice4444FormatsFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevice4444FormatsFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDevice4444FormatsFeaturesEXT *>(current);
                if (enabling->formatA4R4G4B4 && !supported.formatA4R4G4B4) {
                    ss << "VkPhysicalDevice4444FormatsFeaturesEXT::formatA4R4G4B4 is not supported\n";
                }
                if (enabling->formatA4B4G4R4 && !supported.formatA4B4G4R4) {
                    ss << "VkPhysicalDevice4444FormatsFeaturesEXT::formatA4B4G4R4 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES: {
                VkPhysicalDevice8BitStorageFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevice8BitStorageFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDevice8BitStorageFeatures *>(current);
                if (enabling->storageBuffer8BitAccess && !supported.storageBuffer8BitAccess) {
                    ss << "VkPhysicalDevice8BitStorageFeatures::storageBuffer8BitAccess is not supported\n";
                }
                if (enabling->uniformAndStorageBuffer8BitAccess && !supported.uniformAndStorageBuffer8BitAccess) {
                    ss << "VkPhysicalDevice8BitStorageFeatures::uniformAndStorageBuffer8BitAccess is not supported\n";
                }
                if (enabling->storagePushConstant8 && !supported.storagePushConstant8) {
                    ss << "VkPhysicalDevice8BitStorageFeatures::storagePushConstant8 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT: {
                VkPhysicalDeviceASTCDecodeFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceASTCDecodeFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceASTCDecodeFeaturesEXT *>(current);
                if (enabling->decodeModeSharedExponent && !supported.decodeModeSharedExponent) {
                    ss << "VkPhysicalDeviceASTCDecodeFeaturesEXT::decodeModeSharedExponent is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR: {
                VkPhysicalDeviceAccelerationStructureFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceAccelerationStructureFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(current);
                if (enabling->accelerationStructure && !supported.accelerationStructure) {
                    ss << "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructure is not supported\n";
                }
                if (enabling->accelerationStructureCaptureReplay && !supported.accelerationStructureCaptureReplay) {
                    ss << "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureCaptureReplay is not supported\n";
                }
                if (enabling->accelerationStructureIndirectBuild && !supported.accelerationStructureIndirectBuild) {
                    ss << "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureIndirectBuild is not supported\n";
                }
                if (enabling->accelerationStructureHostCommands && !supported.accelerationStructureHostCommands) {
                    ss << "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands is not supported\n";
                }
                if (enabling->descriptorBindingAccelerationStructureUpdateAfterBind &&
                    !supported.descriptorBindingAccelerationStructureUpdateAfterBind) {
                    ss << "VkPhysicalDeviceAccelerationStructureFeaturesKHR::descriptorBindingAccelerationStructureUpdateAfterBind "
                          "is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT: {
                VkPhysicalDeviceAddressBindingReportFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceAddressBindingReportFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceAddressBindingReportFeaturesEXT *>(current);
                if (enabling->reportAddressBinding && !supported.reportAddressBinding) {
                    ss << "VkPhysicalDeviceAddressBindingReportFeaturesEXT::reportAddressBinding is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC: {
                VkPhysicalDeviceAmigoProfilingFeaturesSEC supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceAmigoProfilingFeaturesSEC *enabling =
                    reinterpret_cast<const VkPhysicalDeviceAmigoProfilingFeaturesSEC *>(current);
                if (enabling->amigoProfiling && !supported.amigoProfiling) {
                    ss << "VkPhysicalDeviceAmigoProfilingFeaturesSEC::amigoProfiling is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD: {
                VkPhysicalDeviceAntiLagFeaturesAMD supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceAntiLagFeaturesAMD *enabling =
                    reinterpret_cast<const VkPhysicalDeviceAntiLagFeaturesAMD *>(current);
                if (enabling->antiLag && !supported.antiLag) {
                    ss << "VkPhysicalDeviceAntiLagFeaturesAMD::antiLag is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT: {
                VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *>(current);
                if (enabling->attachmentFeedbackLoopDynamicState && !supported.attachmentFeedbackLoopDynamicState) {
                    ss << "VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT::attachmentFeedbackLoopDynamicState is "
                          "not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT: {
                VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *>(current);
                if (enabling->attachmentFeedbackLoopLayout && !supported.attachmentFeedbackLoopLayout) {
                    ss << "VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT::attachmentFeedbackLoopLayout is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT: {
                VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(current);
                if (enabling->advancedBlendCoherentOperations && !supported.advancedBlendCoherentOperations) {
                    ss << "VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::advancedBlendCoherentOperations is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT: {
                VkPhysicalDeviceBorderColorSwizzleFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(current);
                if (enabling->borderColorSwizzle && !supported.borderColorSwizzle) {
                    ss << "VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::borderColorSwizzle is not supported\n";
                }
                if (enabling->borderColorSwizzleFromImage && !supported.borderColorSwizzleFromImage) {
                    ss << "VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::borderColorSwizzleFromImage is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES: {
                VkPhysicalDeviceBufferDeviceAddressFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceBufferDeviceAddressFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeatures *>(current);
                if (enabling->bufferDeviceAddress && !supported.bufferDeviceAddress) {
                    ss << "VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddress is not supported\n";
                }
                if (enabling->bufferDeviceAddressCaptureReplay && !supported.bufferDeviceAddressCaptureReplay) {
                    ss << "VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddressCaptureReplay is not supported\n";
                }
                if (enabling->bufferDeviceAddressMultiDevice && !supported.bufferDeviceAddressMultiDevice) {
                    ss << "VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddressMultiDevice is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT: {
                VkPhysicalDeviceBufferDeviceAddressFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *>(current);
                if (enabling->bufferDeviceAddress && !supported.bufferDeviceAddress) {
                    ss << "VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::bufferDeviceAddress is not supported\n";
                }
                if (enabling->bufferDeviceAddressCaptureReplay && !supported.bufferDeviceAddressCaptureReplay) {
                    ss << "VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::bufferDeviceAddressCaptureReplay is not supported\n";
                }
                if (enabling->bufferDeviceAddressMultiDevice && !supported.bufferDeviceAddressMultiDevice) {
                    ss << "VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::bufferDeviceAddressMultiDevice is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV: {
                VkPhysicalDeviceClusterAccelerationStructureFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV *>(current);
                if (enabling->clusterAccelerationStructure && !supported.clusterAccelerationStructure) {
                    ss << "VkPhysicalDeviceClusterAccelerationStructureFeaturesNV::clusterAccelerationStructure is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI: {
                VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *enabling =
                    reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(current);
                if (enabling->clustercullingShader && !supported.clustercullingShader) {
                    ss << "VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::clustercullingShader is not supported\n";
                }
                if (enabling->multiviewClusterCullingShader && !supported.multiviewClusterCullingShader) {
                    ss << "VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::multiviewClusterCullingShader is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD: {
                VkPhysicalDeviceCoherentMemoryFeaturesAMD supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCoherentMemoryFeaturesAMD *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(current);
                if (enabling->deviceCoherentMemory && !supported.deviceCoherentMemory) {
                    ss << "VkPhysicalDeviceCoherentMemoryFeaturesAMD::deviceCoherentMemory is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT: {
                VkPhysicalDeviceColorWriteEnableFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceColorWriteEnableFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(current);
                if (enabling->colorWriteEnable && !supported.colorWriteEnable) {
                    ss << "VkPhysicalDeviceColorWriteEnableFeaturesEXT::colorWriteEnable is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV: {
                VkPhysicalDeviceCommandBufferInheritanceFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV *>(current);
                if (enabling->commandBufferInheritance && !supported.commandBufferInheritance) {
                    ss << "VkPhysicalDeviceCommandBufferInheritanceFeaturesNV::commandBufferInheritance is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR: {
                VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR *>(current);
                if (enabling->computeDerivativeGroupQuads && !supported.computeDerivativeGroupQuads) {
                    ss << "VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::computeDerivativeGroupQuads is not supported\n";
                }
                if (enabling->computeDerivativeGroupLinear && !supported.computeDerivativeGroupLinear) {
                    ss << "VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::computeDerivativeGroupLinear is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT: {
                VkPhysicalDeviceConditionalRenderingFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceConditionalRenderingFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(current);
                if (enabling->conditionalRendering && !supported.conditionalRendering) {
                    ss << "VkPhysicalDeviceConditionalRenderingFeaturesEXT::conditionalRendering is not supported\n";
                }
                if (enabling->inheritedConditionalRendering && !supported.inheritedConditionalRendering) {
                    ss << "VkPhysicalDeviceConditionalRenderingFeaturesEXT::inheritedConditionalRendering is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV: {
                VkPhysicalDeviceCooperativeMatrix2FeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCooperativeMatrix2FeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(current);
                if (enabling->cooperativeMatrixWorkgroupScope && !supported.cooperativeMatrixWorkgroupScope) {
                    ss << "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixWorkgroupScope is not supported\n";
                }
                if (enabling->cooperativeMatrixFlexibleDimensions && !supported.cooperativeMatrixFlexibleDimensions) {
                    ss << "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixFlexibleDimensions is not supported\n";
                }
                if (enabling->cooperativeMatrixReductions && !supported.cooperativeMatrixReductions) {
                    ss << "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixReductions is not supported\n";
                }
                if (enabling->cooperativeMatrixConversions && !supported.cooperativeMatrixConversions) {
                    ss << "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixConversions is not supported\n";
                }
                if (enabling->cooperativeMatrixPerElementOperations && !supported.cooperativeMatrixPerElementOperations) {
                    ss << "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixPerElementOperations is not supported\n";
                }
                if (enabling->cooperativeMatrixTensorAddressing && !supported.cooperativeMatrixTensorAddressing) {
                    ss << "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixTensorAddressing is not supported\n";
                }
                if (enabling->cooperativeMatrixBlockLoads && !supported.cooperativeMatrixBlockLoads) {
                    ss << "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixBlockLoads is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR: {
                VkPhysicalDeviceCooperativeMatrixFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCooperativeMatrixFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesKHR *>(current);
                if (enabling->cooperativeMatrix && !supported.cooperativeMatrix) {
                    ss << "VkPhysicalDeviceCooperativeMatrixFeaturesKHR::cooperativeMatrix is not supported\n";
                }
                if (enabling->cooperativeMatrixRobustBufferAccess && !supported.cooperativeMatrixRobustBufferAccess) {
                    ss << "VkPhysicalDeviceCooperativeMatrixFeaturesKHR::cooperativeMatrixRobustBufferAccess is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV: {
                VkPhysicalDeviceCooperativeMatrixFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCooperativeMatrixFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesNV *>(current);
                if (enabling->cooperativeMatrix && !supported.cooperativeMatrix) {
                    ss << "VkPhysicalDeviceCooperativeMatrixFeaturesNV::cooperativeMatrix is not supported\n";
                }
                if (enabling->cooperativeMatrixRobustBufferAccess && !supported.cooperativeMatrixRobustBufferAccess) {
                    ss << "VkPhysicalDeviceCooperativeMatrixFeaturesNV::cooperativeMatrixRobustBufferAccess is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV: {
                VkPhysicalDeviceCooperativeVectorFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCooperativeVectorFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCooperativeVectorFeaturesNV *>(current);
                if (enabling->cooperativeVector && !supported.cooperativeVector) {
                    ss << "VkPhysicalDeviceCooperativeVectorFeaturesNV::cooperativeVector is not supported\n";
                }
                if (enabling->cooperativeVectorTraining && !supported.cooperativeVectorTraining) {
                    ss << "VkPhysicalDeviceCooperativeVectorFeaturesNV::cooperativeVectorTraining is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_KHR: {
                VkPhysicalDeviceCopyMemoryIndirectFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCopyMemoryIndirectFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesKHR *>(current);
                if (enabling->indirectMemoryCopy && !supported.indirectMemoryCopy) {
                    ss << "VkPhysicalDeviceCopyMemoryIndirectFeaturesKHR::indirectMemoryCopy is not supported\n";
                }
                if (enabling->indirectMemoryToImageCopy && !supported.indirectMemoryToImageCopy) {
                    ss << "VkPhysicalDeviceCopyMemoryIndirectFeaturesKHR::indirectMemoryToImageCopy is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV: {
                VkPhysicalDeviceCopyMemoryIndirectFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *>(current);
                if (enabling->indirectCopy && !supported.indirectCopy) {
                    ss << "VkPhysicalDeviceCopyMemoryIndirectFeaturesNV::indirectCopy is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV: {
                VkPhysicalDeviceCornerSampledImageFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCornerSampledImageFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCornerSampledImageFeaturesNV *>(current);
                if (enabling->cornerSampledImage && !supported.cornerSampledImage) {
                    ss << "VkPhysicalDeviceCornerSampledImageFeaturesNV::cornerSampledImage is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV: {
                VkPhysicalDeviceCoverageReductionModeFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCoverageReductionModeFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(current);
                if (enabling->coverageReductionMode && !supported.coverageReductionMode) {
                    ss << "VkPhysicalDeviceCoverageReductionModeFeaturesNV::coverageReductionMode is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM: {
                VkPhysicalDeviceCubicClampFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCubicClampFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCubicClampFeaturesQCOM *>(current);
                if (enabling->cubicRangeClamp && !supported.cubicRangeClamp) {
                    ss << "VkPhysicalDeviceCubicClampFeaturesQCOM::cubicRangeClamp is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM: {
                VkPhysicalDeviceCubicWeightsFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCubicWeightsFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCubicWeightsFeaturesQCOM *>(current);
                if (enabling->selectableCubicWeights && !supported.selectableCubicWeights) {
                    ss << "VkPhysicalDeviceCubicWeightsFeaturesQCOM::selectableCubicWeights is not supported\n";
                }
                break;
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV: {
                VkPhysicalDeviceCudaKernelLaunchFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCudaKernelLaunchFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCudaKernelLaunchFeaturesNV *>(current);
                if (enabling->cudaKernelLaunchFeatures && !supported.cudaKernelLaunchFeatures) {
                    ss << "VkPhysicalDeviceCudaKernelLaunchFeaturesNV::cudaKernelLaunchFeatures is not supported\n";
                }
                break;
            }
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT: {
                VkPhysicalDeviceCustomBorderColorFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceCustomBorderColorFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(current);
                if (enabling->customBorderColors && !supported.customBorderColors) {
                    ss << "VkPhysicalDeviceCustomBorderColorFeaturesEXT::customBorderColors is not supported\n";
                }
                if (enabling->customBorderColorWithoutFormat && !supported.customBorderColorWithoutFormat) {
                    ss << "VkPhysicalDeviceCustomBorderColorFeaturesEXT::customBorderColorWithoutFormat is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DATA_GRAPH_FEATURES_ARM: {
                VkPhysicalDeviceDataGraphFeaturesARM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDataGraphFeaturesARM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDataGraphFeaturesARM *>(current);
                if (enabling->dataGraph && !supported.dataGraph) {
                    ss << "VkPhysicalDeviceDataGraphFeaturesARM::dataGraph is not supported\n";
                }
                if (enabling->dataGraphUpdateAfterBind && !supported.dataGraphUpdateAfterBind) {
                    ss << "VkPhysicalDeviceDataGraphFeaturesARM::dataGraphUpdateAfterBind is not supported\n";
                }
                if (enabling->dataGraphSpecializationConstants && !supported.dataGraphSpecializationConstants) {
                    ss << "VkPhysicalDeviceDataGraphFeaturesARM::dataGraphSpecializationConstants is not supported\n";
                }
                if (enabling->dataGraphDescriptorBuffer && !supported.dataGraphDescriptorBuffer) {
                    ss << "VkPhysicalDeviceDataGraphFeaturesARM::dataGraphDescriptorBuffer is not supported\n";
                }
                if (enabling->dataGraphShaderModule && !supported.dataGraphShaderModule) {
                    ss << "VkPhysicalDeviceDataGraphFeaturesARM::dataGraphShaderModule is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DATA_GRAPH_MODEL_FEATURES_QCOM: {
                VkPhysicalDeviceDataGraphModelFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDataGraphModelFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDataGraphModelFeaturesQCOM *>(current);
                if (enabling->dataGraphModel && !supported.dataGraphModel) {
                    ss << "VkPhysicalDeviceDataGraphModelFeaturesQCOM::dataGraphModel is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV: {
                VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(current);
                if (enabling->dedicatedAllocationImageAliasing && !supported.dedicatedAllocationImageAliasing) {
                    ss << "VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::dedicatedAllocationImageAliasing is not "
                          "supported\n";
                }
                break;
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DENSE_GEOMETRY_FORMAT_FEATURES_AMDX: {
                VkPhysicalDeviceDenseGeometryFormatFeaturesAMDX supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDenseGeometryFormatFeaturesAMDX *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDenseGeometryFormatFeaturesAMDX *>(current);
                if (enabling->denseGeometryFormat && !supported.denseGeometryFormat) {
                    ss << "VkPhysicalDeviceDenseGeometryFormatFeaturesAMDX::denseGeometryFormat is not supported\n";
                }
                break;
            }
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT: {
                VkPhysicalDeviceDepthBiasControlFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDepthBiasControlFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(current);
                if (enabling->depthBiasControl && !supported.depthBiasControl) {
                    ss << "VkPhysicalDeviceDepthBiasControlFeaturesEXT::depthBiasControl is not supported\n";
                }
                if (enabling->leastRepresentableValueForceUnormRepresentation &&
                    !supported.leastRepresentableValueForceUnormRepresentation) {
                    ss << "VkPhysicalDeviceDepthBiasControlFeaturesEXT::leastRepresentableValueForceUnormRepresentation is not "
                          "supported\n";
                }
                if (enabling->floatRepresentation && !supported.floatRepresentation) {
                    ss << "VkPhysicalDeviceDepthBiasControlFeaturesEXT::floatRepresentation is not supported\n";
                }
                if (enabling->depthBiasExact && !supported.depthBiasExact) {
                    ss << "VkPhysicalDeviceDepthBiasControlFeaturesEXT::depthBiasExact is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT: {
                VkPhysicalDeviceDepthClampControlFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDepthClampControlFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDepthClampControlFeaturesEXT *>(current);
                if (enabling->depthClampControl && !supported.depthClampControl) {
                    ss << "VkPhysicalDeviceDepthClampControlFeaturesEXT::depthClampControl is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR: {
                VkPhysicalDeviceDepthClampZeroOneFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR *>(current);
                if (enabling->depthClampZeroOne && !supported.depthClampZeroOne) {
                    ss << "VkPhysicalDeviceDepthClampZeroOneFeaturesKHR::depthClampZeroOne is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT: {
                VkPhysicalDeviceDepthClipControlFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDepthClipControlFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDepthClipControlFeaturesEXT *>(current);
                if (enabling->depthClipControl && !supported.depthClipControl) {
                    ss << "VkPhysicalDeviceDepthClipControlFeaturesEXT::depthClipControl is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT: {
                VkPhysicalDeviceDepthClipEnableFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDepthClipEnableFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(current);
                if (enabling->depthClipEnable && !supported.depthClipEnable) {
                    ss << "VkPhysicalDeviceDepthClipEnableFeaturesEXT::depthClipEnable is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT: {
                VkPhysicalDeviceDescriptorBufferFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDescriptorBufferFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(current);
                if (enabling->descriptorBuffer && !supported.descriptorBuffer) {
                    ss << "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBuffer is not supported\n";
                }
                if (enabling->descriptorBufferCaptureReplay && !supported.descriptorBufferCaptureReplay) {
                    ss << "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBufferCaptureReplay is not supported\n";
                }
                if (enabling->descriptorBufferImageLayoutIgnored && !supported.descriptorBufferImageLayoutIgnored) {
                    ss << "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBufferImageLayoutIgnored is not supported\n";
                }
                if (enabling->descriptorBufferPushDescriptors && !supported.descriptorBufferPushDescriptors) {
                    ss << "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBufferPushDescriptors is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_TENSOR_FEATURES_ARM: {
                VkPhysicalDeviceDescriptorBufferTensorFeaturesARM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDescriptorBufferTensorFeaturesARM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDescriptorBufferTensorFeaturesARM *>(current);
                if (enabling->descriptorBufferTensorDescriptors && !supported.descriptorBufferTensorDescriptors) {
                    ss << "VkPhysicalDeviceDescriptorBufferTensorFeaturesARM::descriptorBufferTensorDescriptors is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES: {
                VkPhysicalDeviceDescriptorIndexingFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDescriptorIndexingFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeatures *>(current);
                if (enabling->shaderInputAttachmentArrayDynamicIndexing && !supported.shaderInputAttachmentArrayDynamicIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderInputAttachmentArrayDynamicIndexing is not "
                          "supported\n";
                }
                if (enabling->shaderUniformTexelBufferArrayDynamicIndexing &&
                    !supported.shaderUniformTexelBufferArrayDynamicIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformTexelBufferArrayDynamicIndexing is not "
                          "supported\n";
                }
                if (enabling->shaderStorageTexelBufferArrayDynamicIndexing &&
                    !supported.shaderStorageTexelBufferArrayDynamicIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageTexelBufferArrayDynamicIndexing is not "
                          "supported\n";
                }
                if (enabling->shaderUniformBufferArrayNonUniformIndexing && !supported.shaderUniformBufferArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformBufferArrayNonUniformIndexing is not "
                          "supported\n";
                }
                if (enabling->shaderSampledImageArrayNonUniformIndexing && !supported.shaderSampledImageArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderSampledImageArrayNonUniformIndexing is not "
                          "supported\n";
                }
                if (enabling->shaderStorageBufferArrayNonUniformIndexing && !supported.shaderStorageBufferArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageBufferArrayNonUniformIndexing is not "
                          "supported\n";
                }
                if (enabling->shaderStorageImageArrayNonUniformIndexing && !supported.shaderStorageImageArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageImageArrayNonUniformIndexing is not "
                          "supported\n";
                }
                if (enabling->shaderInputAttachmentArrayNonUniformIndexing &&
                    !supported.shaderInputAttachmentArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderInputAttachmentArrayNonUniformIndexing is not "
                          "supported\n";
                }
                if (enabling->shaderUniformTexelBufferArrayNonUniformIndexing &&
                    !supported.shaderUniformTexelBufferArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformTexelBufferArrayNonUniformIndexing is not "
                          "supported\n";
                }
                if (enabling->shaderStorageTexelBufferArrayNonUniformIndexing &&
                    !supported.shaderStorageTexelBufferArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageTexelBufferArrayNonUniformIndexing is not "
                          "supported\n";
                }
                if (enabling->descriptorBindingUniformBufferUpdateAfterBind &&
                    !supported.descriptorBindingUniformBufferUpdateAfterBind) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingUniformBufferUpdateAfterBind is not "
                          "supported\n";
                }
                if (enabling->descriptorBindingSampledImageUpdateAfterBind &&
                    !supported.descriptorBindingSampledImageUpdateAfterBind) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingSampledImageUpdateAfterBind is not "
                          "supported\n";
                }
                if (enabling->descriptorBindingStorageImageUpdateAfterBind &&
                    !supported.descriptorBindingStorageImageUpdateAfterBind) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingStorageImageUpdateAfterBind is not "
                          "supported\n";
                }
                if (enabling->descriptorBindingStorageBufferUpdateAfterBind &&
                    !supported.descriptorBindingStorageBufferUpdateAfterBind) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingStorageBufferUpdateAfterBind is not "
                          "supported\n";
                }
                if (enabling->descriptorBindingUniformTexelBufferUpdateAfterBind &&
                    !supported.descriptorBindingUniformTexelBufferUpdateAfterBind) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingUniformTexelBufferUpdateAfterBind is not "
                          "supported\n";
                }
                if (enabling->descriptorBindingStorageTexelBufferUpdateAfterBind &&
                    !supported.descriptorBindingStorageTexelBufferUpdateAfterBind) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingStorageTexelBufferUpdateAfterBind is not "
                          "supported\n";
                }
                if (enabling->descriptorBindingUpdateUnusedWhilePending && !supported.descriptorBindingUpdateUnusedWhilePending) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingUpdateUnusedWhilePending is not "
                          "supported\n";
                }
                if (enabling->descriptorBindingPartiallyBound && !supported.descriptorBindingPartiallyBound) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingPartiallyBound is not supported\n";
                }
                if (enabling->descriptorBindingVariableDescriptorCount && !supported.descriptorBindingVariableDescriptorCount) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingVariableDescriptorCount is not supported\n";
                }
                if (enabling->runtimeDescriptorArray && !supported.runtimeDescriptorArray) {
                    ss << "VkPhysicalDeviceDescriptorIndexingFeatures::runtimeDescriptorArray is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV: {
                VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *>(current);
                if (enabling->descriptorPoolOverallocation && !supported.descriptorPoolOverallocation) {
                    ss << "VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV::descriptorPoolOverallocation is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE: {
                VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *>(current);
                if (enabling->descriptorSetHostMapping && !supported.descriptorSetHostMapping) {
                    ss << "VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE::descriptorSetHostMapping is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV: {
                VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(current);
                if (enabling->deviceGeneratedCompute && !supported.deviceGeneratedCompute) {
                    ss << "VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::deviceGeneratedCompute is not supported\n";
                }
                if (enabling->deviceGeneratedComputePipelines && !supported.deviceGeneratedComputePipelines) {
                    ss << "VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::deviceGeneratedComputePipelines is not "
                          "supported\n";
                }
                if (enabling->deviceGeneratedComputeCaptureReplay && !supported.deviceGeneratedComputeCaptureReplay) {
                    ss << "VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::deviceGeneratedComputeCaptureReplay is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT: {
                VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT *>(current);
                if (enabling->deviceGeneratedCommands && !supported.deviceGeneratedCommands) {
                    ss << "VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::deviceGeneratedCommands is not supported\n";
                }
                if (enabling->dynamicGeneratedPipelineLayout && !supported.dynamicGeneratedPipelineLayout) {
                    ss << "VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::dynamicGeneratedPipelineLayout is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV: {
                VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *>(current);
                if (enabling->deviceGeneratedCommands && !supported.deviceGeneratedCommands) {
                    ss << "VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV::deviceGeneratedCommands is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT: {
                VkPhysicalDeviceDeviceMemoryReportFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(current);
                if (enabling->deviceMemoryReport && !supported.deviceMemoryReport) {
                    ss << "VkPhysicalDeviceDeviceMemoryReportFeaturesEXT::deviceMemoryReport is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV: {
                VkPhysicalDeviceDiagnosticsConfigFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDiagnosticsConfigFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDiagnosticsConfigFeaturesNV *>(current);
                if (enabling->diagnosticsConfig && !supported.diagnosticsConfig) {
                    ss << "VkPhysicalDeviceDiagnosticsConfigFeaturesNV::diagnosticsConfig is not supported\n";
                }
                break;
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV: {
                VkPhysicalDeviceDisplacementMicromapFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDisplacementMicromapFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapFeaturesNV *>(current);
                if (enabling->displacementMicromap && !supported.displacementMicromap) {
                    ss << "VkPhysicalDeviceDisplacementMicromapFeaturesNV::displacementMicromap is not supported\n";
                }
                break;
            }
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES: {
                VkPhysicalDeviceDynamicRenderingFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDynamicRenderingFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeatures *>(current);
                if (enabling->dynamicRendering && !supported.dynamicRendering) {
                    ss << "VkPhysicalDeviceDynamicRenderingFeatures::dynamicRendering is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES: {
                VkPhysicalDeviceDynamicRenderingLocalReadFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDynamicRenderingLocalReadFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDynamicRenderingLocalReadFeatures *>(current);
                if (enabling->dynamicRenderingLocalRead && !supported.dynamicRenderingLocalRead) {
                    ss << "VkPhysicalDeviceDynamicRenderingLocalReadFeatures::dynamicRenderingLocalRead is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT: {
                VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *>(current);
                if (enabling->dynamicRenderingUnusedAttachments && !supported.dynamicRenderingUnusedAttachments) {
                    ss << "VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT::dynamicRenderingUnusedAttachments is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV: {
                VkPhysicalDeviceExclusiveScissorFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceExclusiveScissorFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceExclusiveScissorFeaturesNV *>(current);
                if (enabling->exclusiveScissor && !supported.exclusiveScissor) {
                    ss << "VkPhysicalDeviceExclusiveScissorFeaturesNV::exclusiveScissor is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT: {
                VkPhysicalDeviceExtendedDynamicState2FeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(current);
                if (enabling->extendedDynamicState2 && !supported.extendedDynamicState2) {
                    ss << "VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::extendedDynamicState2 is not supported\n";
                }
                if (enabling->extendedDynamicState2LogicOp && !supported.extendedDynamicState2LogicOp) {
                    ss << "VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::extendedDynamicState2LogicOp is not supported\n";
                }
                if (enabling->extendedDynamicState2PatchControlPoints && !supported.extendedDynamicState2PatchControlPoints) {
                    ss << "VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::extendedDynamicState2PatchControlPoints is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT: {
                VkPhysicalDeviceExtendedDynamicState3FeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(current);
                if (enabling->extendedDynamicState3TessellationDomainOrigin &&
                    !supported.extendedDynamicState3TessellationDomainOrigin) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3TessellationDomainOrigin is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3DepthClampEnable && !supported.extendedDynamicState3DepthClampEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3DepthClampEnable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3PolygonMode && !supported.extendedDynamicState3PolygonMode) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3PolygonMode is not supported\n";
                }
                if (enabling->extendedDynamicState3RasterizationSamples && !supported.extendedDynamicState3RasterizationSamples) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3RasterizationSamples is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3SampleMask && !supported.extendedDynamicState3SampleMask) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3SampleMask is not supported\n";
                }
                if (enabling->extendedDynamicState3AlphaToCoverageEnable && !supported.extendedDynamicState3AlphaToCoverageEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3AlphaToCoverageEnable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3AlphaToOneEnable && !supported.extendedDynamicState3AlphaToOneEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3AlphaToOneEnable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3LogicOpEnable && !supported.extendedDynamicState3LogicOpEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3LogicOpEnable is not supported\n";
                }
                if (enabling->extendedDynamicState3ColorBlendEnable && !supported.extendedDynamicState3ColorBlendEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ColorBlendEnable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3ColorBlendEquation && !supported.extendedDynamicState3ColorBlendEquation) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ColorBlendEquation is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3ColorWriteMask && !supported.extendedDynamicState3ColorWriteMask) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ColorWriteMask is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3RasterizationStream && !supported.extendedDynamicState3RasterizationStream) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3RasterizationStream is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3ConservativeRasterizationMode &&
                    !supported.extendedDynamicState3ConservativeRasterizationMode) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ConservativeRasterizationMode is "
                          "not supported\n";
                }
                if (enabling->extendedDynamicState3ExtraPrimitiveOverestimationSize &&
                    !supported.extendedDynamicState3ExtraPrimitiveOverestimationSize) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ExtraPrimitiveOverestimationSize "
                          "is not supported\n";
                }
                if (enabling->extendedDynamicState3DepthClipEnable && !supported.extendedDynamicState3DepthClipEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3DepthClipEnable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3SampleLocationsEnable && !supported.extendedDynamicState3SampleLocationsEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3SampleLocationsEnable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3ColorBlendAdvanced && !supported.extendedDynamicState3ColorBlendAdvanced) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ColorBlendAdvanced is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3ProvokingVertexMode && !supported.extendedDynamicState3ProvokingVertexMode) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ProvokingVertexMode is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3LineRasterizationMode && !supported.extendedDynamicState3LineRasterizationMode) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3LineRasterizationMode is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3LineStippleEnable && !supported.extendedDynamicState3LineStippleEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3LineStippleEnable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3DepthClipNegativeOneToOne &&
                    !supported.extendedDynamicState3DepthClipNegativeOneToOne) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3DepthClipNegativeOneToOne is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3ViewportWScalingEnable &&
                    !supported.extendedDynamicState3ViewportWScalingEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ViewportWScalingEnable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3ViewportSwizzle && !supported.extendedDynamicState3ViewportSwizzle) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ViewportSwizzle is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3CoverageToColorEnable && !supported.extendedDynamicState3CoverageToColorEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageToColorEnable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3CoverageToColorLocation &&
                    !supported.extendedDynamicState3CoverageToColorLocation) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageToColorLocation is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3CoverageModulationMode &&
                    !supported.extendedDynamicState3CoverageModulationMode) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageModulationMode is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3CoverageModulationTableEnable &&
                    !supported.extendedDynamicState3CoverageModulationTableEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageModulationTableEnable is "
                          "not supported\n";
                }
                if (enabling->extendedDynamicState3CoverageModulationTable &&
                    !supported.extendedDynamicState3CoverageModulationTable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageModulationTable is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3CoverageReductionMode && !supported.extendedDynamicState3CoverageReductionMode) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageReductionMode is not "
                          "supported\n";
                }
                if (enabling->extendedDynamicState3RepresentativeFragmentTestEnable &&
                    !supported.extendedDynamicState3RepresentativeFragmentTestEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3RepresentativeFragmentTestEnable "
                          "is not supported\n";
                }
                if (enabling->extendedDynamicState3ShadingRateImageEnable &&
                    !supported.extendedDynamicState3ShadingRateImageEnable) {
                    ss << "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ShadingRateImageEnable is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT: {
                VkPhysicalDeviceExtendedDynamicStateFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(current);
                if (enabling->extendedDynamicState && !supported.extendedDynamicState) {
                    ss << "VkPhysicalDeviceExtendedDynamicStateFeaturesEXT::extendedDynamicState is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV: {
                VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *>(current);
                if (enabling->extendedSparseAddressSpace && !supported.extendedSparseAddressSpace) {
                    ss << "VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV::extendedSparseAddressSpace is not supported\n";
                }
                break;
            }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID: {
                VkPhysicalDeviceExternalFormatResolveFeaturesANDROID supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *enabling =
                    reinterpret_cast<const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *>(current);
                if (enabling->externalFormatResolve && !supported.externalFormatResolve) {
                    ss << "VkPhysicalDeviceExternalFormatResolveFeaturesANDROID::externalFormatResolve is not supported\n";
                }
                break;
            }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV: {
                VkPhysicalDeviceExternalMemoryRDMAFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *>(current);
                if (enabling->externalMemoryRDMA && !supported.externalMemoryRDMA) {
                    ss << "VkPhysicalDeviceExternalMemoryRDMAFeaturesNV::externalMemoryRDMA is not supported\n";
                }
                break;
            }
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX: {
                VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *enabling =
                    reinterpret_cast<const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *>(current);
                if (enabling->screenBufferImport && !supported.screenBufferImport) {
                    ss << "VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX::screenBufferImport is not supported\n";
                }
                break;
            }
#endif  // VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT: {
                VkPhysicalDeviceFaultFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFaultFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFaultFeaturesEXT *>(current);
                if (enabling->deviceFault && !supported.deviceFault) {
                    ss << "VkPhysicalDeviceFaultFeaturesEXT::deviceFault is not supported\n";
                }
                if (enabling->deviceFaultVendorBinary && !supported.deviceFaultVendorBinary) {
                    ss << "VkPhysicalDeviceFaultFeaturesEXT::deviceFaultVendorBinary is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FORMAT_PACK_FEATURES_ARM: {
                VkPhysicalDeviceFormatPackFeaturesARM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFormatPackFeaturesARM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFormatPackFeaturesARM *>(current);
                if (enabling->formatPack && !supported.formatPack) {
                    ss << "VkPhysicalDeviceFormatPackFeaturesARM::formatPack is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT: {
                VkPhysicalDeviceFragmentDensityMap2FeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *>(current);
                if (enabling->fragmentDensityMapDeferred && !supported.fragmentDensityMapDeferred) {
                    ss << "VkPhysicalDeviceFragmentDensityMap2FeaturesEXT::fragmentDensityMapDeferred is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT: {
                VkPhysicalDeviceFragmentDensityMapFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(current);
                if (enabling->fragmentDensityMap && !supported.fragmentDensityMap) {
                    ss << "VkPhysicalDeviceFragmentDensityMapFeaturesEXT::fragmentDensityMap is not supported\n";
                }
                if (enabling->fragmentDensityMapDynamic && !supported.fragmentDensityMapDynamic) {
                    ss << "VkPhysicalDeviceFragmentDensityMapFeaturesEXT::fragmentDensityMapDynamic is not supported\n";
                }
                if (enabling->fragmentDensityMapNonSubsampledImages && !supported.fragmentDensityMapNonSubsampledImages) {
                    ss << "VkPhysicalDeviceFragmentDensityMapFeaturesEXT::fragmentDensityMapNonSubsampledImages is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_LAYERED_FEATURES_VALVE: {
                VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE *>(current);
                if (enabling->fragmentDensityMapLayered && !supported.fragmentDensityMapLayered) {
                    ss << "VkPhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE::fragmentDensityMapLayered is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_EXT: {
                VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT *>(current);
                if (enabling->fragmentDensityMapOffset && !supported.fragmentDensityMapOffset) {
                    ss << "VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT::fragmentDensityMapOffset is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR: {
                VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *>(current);
                if (enabling->fragmentShaderBarycentric && !supported.fragmentShaderBarycentric) {
                    ss << "VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::fragmentShaderBarycentric is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT: {
                VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(current);
                if (enabling->fragmentShaderSampleInterlock && !supported.fragmentShaderSampleInterlock) {
                    ss << "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderSampleInterlock is not supported\n";
                }
                if (enabling->fragmentShaderPixelInterlock && !supported.fragmentShaderPixelInterlock) {
                    ss << "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderPixelInterlock is not supported\n";
                }
                if (enabling->fragmentShaderShadingRateInterlock && !supported.fragmentShaderShadingRateInterlock) {
                    ss << "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderShadingRateInterlock is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV: {
                VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(current);
                if (enabling->fragmentShadingRateEnums && !supported.fragmentShadingRateEnums) {
                    ss << "VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::fragmentShadingRateEnums is not supported\n";
                }
                if (enabling->supersampleFragmentShadingRates && !supported.supersampleFragmentShadingRates) {
                    ss << "VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::supersampleFragmentShadingRates is not supported\n";
                }
                if (enabling->noInvocationFragmentShadingRates && !supported.noInvocationFragmentShadingRates) {
                    ss << "VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::noInvocationFragmentShadingRates is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR: {
                VkPhysicalDeviceFragmentShadingRateFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(current);
                if (enabling->pipelineFragmentShadingRate && !supported.pipelineFragmentShadingRate) {
                    ss << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::pipelineFragmentShadingRate is not supported\n";
                }
                if (enabling->primitiveFragmentShadingRate && !supported.primitiveFragmentShadingRate) {
                    ss << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::primitiveFragmentShadingRate is not supported\n";
                }
                if (enabling->attachmentFragmentShadingRate && !supported.attachmentFragmentShadingRate) {
                    ss << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT: {
                VkPhysicalDeviceFrameBoundaryFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceFrameBoundaryFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceFrameBoundaryFeaturesEXT *>(current);
                if (enabling->frameBoundary && !supported.frameBoundary) {
                    ss << "VkPhysicalDeviceFrameBoundaryFeaturesEXT::frameBoundary is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES: {
                VkPhysicalDeviceGlobalPriorityQueryFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceGlobalPriorityQueryFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeatures *>(current);
                if (enabling->globalPriorityQuery && !supported.globalPriorityQuery) {
                    ss << "VkPhysicalDeviceGlobalPriorityQueryFeatures::globalPriorityQuery is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT: {
                VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(current);
                if (enabling->graphicsPipelineLibrary && !supported.graphicsPipelineLibrary) {
                    ss << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI: {
                VkPhysicalDeviceHdrVividFeaturesHUAWEI supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceHdrVividFeaturesHUAWEI *enabling =
                    reinterpret_cast<const VkPhysicalDeviceHdrVividFeaturesHUAWEI *>(current);
                if (enabling->hdrVivid && !supported.hdrVivid) {
                    ss << "VkPhysicalDeviceHdrVividFeaturesHUAWEI::hdrVivid is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES: {
                VkPhysicalDeviceHostImageCopyFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceHostImageCopyFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeatures *>(current);
                if (enabling->hostImageCopy && !supported.hostImageCopy) {
                    ss << "VkPhysicalDeviceHostImageCopyFeatures::hostImageCopy is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES: {
                VkPhysicalDeviceHostQueryResetFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceHostQueryResetFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeatures *>(current);
                if (enabling->hostQueryReset && !supported.hostQueryReset) {
                    ss << "VkPhysicalDeviceHostQueryResetFeatures::hostQueryReset is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT: {
                VkPhysicalDeviceImage2DViewOf3DFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(current);
                if (enabling->image2DViewOf3D && !supported.image2DViewOf3D) {
                    ss << "VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::image2DViewOf3D is not supported\n";
                }
                if (enabling->sampler2DViewOf3D && !supported.sampler2DViewOf3D) {
                    ss << "VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::sampler2DViewOf3D is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA: {
                VkPhysicalDeviceImageAlignmentControlFeaturesMESA supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImageAlignmentControlFeaturesMESA *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImageAlignmentControlFeaturesMESA *>(current);
                if (enabling->imageAlignmentControl && !supported.imageAlignmentControl) {
                    ss << "VkPhysicalDeviceImageAlignmentControlFeaturesMESA::imageAlignmentControl is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT: {
                VkPhysicalDeviceImageCompressionControlFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImageCompressionControlFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImageCompressionControlFeaturesEXT *>(current);
                if (enabling->imageCompressionControl && !supported.imageCompressionControl) {
                    ss << "VkPhysicalDeviceImageCompressionControlFeaturesEXT::imageCompressionControl is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT: {
                VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *>(current);
                if (enabling->imageCompressionControlSwapchain && !supported.imageCompressionControlSwapchain) {
                    ss << "VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT::imageCompressionControlSwapchain is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM: {
                VkPhysicalDeviceImageProcessing2FeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImageProcessing2FeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImageProcessing2FeaturesQCOM *>(current);
                if (enabling->textureBlockMatch2 && !supported.textureBlockMatch2) {
                    ss << "VkPhysicalDeviceImageProcessing2FeaturesQCOM::textureBlockMatch2 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM: {
                VkPhysicalDeviceImageProcessingFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImageProcessingFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImageProcessingFeaturesQCOM *>(current);
                if (enabling->textureSampleWeighted && !supported.textureSampleWeighted) {
                    ss << "VkPhysicalDeviceImageProcessingFeaturesQCOM::textureSampleWeighted is not supported\n";
                }
                if (enabling->textureBoxFilter && !supported.textureBoxFilter) {
                    ss << "VkPhysicalDeviceImageProcessingFeaturesQCOM::textureBoxFilter is not supported\n";
                }
                if (enabling->textureBlockMatch && !supported.textureBlockMatch) {
                    ss << "VkPhysicalDeviceImageProcessingFeaturesQCOM::textureBlockMatch is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES: {
                VkPhysicalDeviceImageRobustnessFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImageRobustnessFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeatures *>(current);
                if (enabling->robustImageAccess && !supported.robustImageAccess) {
                    ss << "VkPhysicalDeviceImageRobustnessFeatures::robustImageAccess is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT: {
                VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *>(current);
                if (enabling->imageSlicedViewOf3D && !supported.imageSlicedViewOf3D) {
                    ss << "VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::imageSlicedViewOf3D is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT: {
                VkPhysicalDeviceImageViewMinLodFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImageViewMinLodFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImageViewMinLodFeaturesEXT *>(current);
                if (enabling->minLod && !supported.minLod) {
                    ss << "VkPhysicalDeviceImageViewMinLodFeaturesEXT::minLod is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES: {
                VkPhysicalDeviceImagelessFramebufferFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceImagelessFramebufferFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeatures *>(current);
                if (enabling->imagelessFramebuffer && !supported.imagelessFramebuffer) {
                    ss << "VkPhysicalDeviceImagelessFramebufferFeatures::imagelessFramebuffer is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES: {
                VkPhysicalDeviceIndexTypeUint8Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceIndexTypeUint8Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8Features *>(current);
                if (enabling->indexTypeUint8 && !supported.indexTypeUint8) {
                    ss << "VkPhysicalDeviceIndexTypeUint8Features::indexTypeUint8 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV: {
                VkPhysicalDeviceInheritedViewportScissorFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceInheritedViewportScissorFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceInheritedViewportScissorFeaturesNV *>(current);
                if (enabling->inheritedViewportScissor2D && !supported.inheritedViewportScissor2D) {
                    ss << "VkPhysicalDeviceInheritedViewportScissorFeaturesNV::inheritedViewportScissor2D is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES: {
                VkPhysicalDeviceInlineUniformBlockFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceInlineUniformBlockFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeatures *>(current);
                if (enabling->inlineUniformBlock && !supported.inlineUniformBlock) {
                    ss << "VkPhysicalDeviceInlineUniformBlockFeatures::inlineUniformBlock is not supported\n";
                }
                if (enabling->descriptorBindingInlineUniformBlockUpdateAfterBind &&
                    !supported.descriptorBindingInlineUniformBlockUpdateAfterBind) {
                    ss << "VkPhysicalDeviceInlineUniformBlockFeatures::descriptorBindingInlineUniformBlockUpdateAfterBind is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI: {
                VkPhysicalDeviceInvocationMaskFeaturesHUAWEI supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *enabling =
                    reinterpret_cast<const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *>(current);
                if (enabling->invocationMask && !supported.invocationMask) {
                    ss << "VkPhysicalDeviceInvocationMaskFeaturesHUAWEI::invocationMask is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT: {
                VkPhysicalDeviceLegacyDitheringFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceLegacyDitheringFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceLegacyDitheringFeaturesEXT *>(current);
                if (enabling->legacyDithering && !supported.legacyDithering) {
                    ss << "VkPhysicalDeviceLegacyDitheringFeaturesEXT::legacyDithering is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT: {
                VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT *>(current);
                if (enabling->legacyVertexAttributes && !supported.legacyVertexAttributes) {
                    ss << "VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT::legacyVertexAttributes is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES: {
                VkPhysicalDeviceLineRasterizationFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceLineRasterizationFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeatures *>(current);
                if (enabling->rectangularLines && !supported.rectangularLines) {
                    ss << "VkPhysicalDeviceLineRasterizationFeatures::rectangularLines is not supported\n";
                }
                if (enabling->bresenhamLines && !supported.bresenhamLines) {
                    ss << "VkPhysicalDeviceLineRasterizationFeatures::bresenhamLines is not supported\n";
                }
                if (enabling->smoothLines && !supported.smoothLines) {
                    ss << "VkPhysicalDeviceLineRasterizationFeatures::smoothLines is not supported\n";
                }
                if (enabling->stippledRectangularLines && !supported.stippledRectangularLines) {
                    ss << "VkPhysicalDeviceLineRasterizationFeatures::stippledRectangularLines is not supported\n";
                }
                if (enabling->stippledBresenhamLines && !supported.stippledBresenhamLines) {
                    ss << "VkPhysicalDeviceLineRasterizationFeatures::stippledBresenhamLines is not supported\n";
                }
                if (enabling->stippledSmoothLines && !supported.stippledSmoothLines) {
                    ss << "VkPhysicalDeviceLineRasterizationFeatures::stippledSmoothLines is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV: {
                VkPhysicalDeviceLinearColorAttachmentFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceLinearColorAttachmentFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceLinearColorAttachmentFeaturesNV *>(current);
                if (enabling->linearColorAttachment && !supported.linearColorAttachment) {
                    ss << "VkPhysicalDeviceLinearColorAttachmentFeaturesNV::linearColorAttachment is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_10_FEATURES_KHR: {
                VkPhysicalDeviceMaintenance10FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMaintenance10FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance10FeaturesKHR *>(current);
                if (enabling->maintenance10 && !supported.maintenance10) {
                    ss << "VkPhysicalDeviceMaintenance10FeaturesKHR::maintenance10 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES: {
                VkPhysicalDeviceMaintenance4Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMaintenance4Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance4Features *>(current);
                if (enabling->maintenance4 && !supported.maintenance4) {
                    ss << "VkPhysicalDeviceMaintenance4Features::maintenance4 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES: {
                VkPhysicalDeviceMaintenance5Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMaintenance5Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance5Features *>(current);
                if (enabling->maintenance5 && !supported.maintenance5) {
                    ss << "VkPhysicalDeviceMaintenance5Features::maintenance5 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES: {
                VkPhysicalDeviceMaintenance6Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMaintenance6Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance6Features *>(current);
                if (enabling->maintenance6 && !supported.maintenance6) {
                    ss << "VkPhysicalDeviceMaintenance6Features::maintenance6 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR: {
                VkPhysicalDeviceMaintenance7FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMaintenance7FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance7FeaturesKHR *>(current);
                if (enabling->maintenance7 && !supported.maintenance7) {
                    ss << "VkPhysicalDeviceMaintenance7FeaturesKHR::maintenance7 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR: {
                VkPhysicalDeviceMaintenance8FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMaintenance8FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance8FeaturesKHR *>(current);
                if (enabling->maintenance8 && !supported.maintenance8) {
                    ss << "VkPhysicalDeviceMaintenance8FeaturesKHR::maintenance8 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_FEATURES_KHR: {
                VkPhysicalDeviceMaintenance9FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMaintenance9FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMaintenance9FeaturesKHR *>(current);
                if (enabling->maintenance9 && !supported.maintenance9) {
                    ss << "VkPhysicalDeviceMaintenance9FeaturesKHR::maintenance9 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT: {
                VkPhysicalDeviceMapMemoryPlacedFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *>(current);
                if (enabling->memoryMapPlaced && !supported.memoryMapPlaced) {
                    ss << "VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::memoryMapPlaced is not supported\n";
                }
                if (enabling->memoryMapRangePlaced && !supported.memoryMapRangePlaced) {
                    ss << "VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::memoryMapRangePlaced is not supported\n";
                }
                if (enabling->memoryUnmapReserve && !supported.memoryUnmapReserve) {
                    ss << "VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::memoryUnmapReserve is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_EXT: {
                VkPhysicalDeviceMemoryDecompressionFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMemoryDecompressionFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionFeaturesEXT *>(current);
                if (enabling->memoryDecompression && !supported.memoryDecompression) {
                    ss << "VkPhysicalDeviceMemoryDecompressionFeaturesEXT::memoryDecompression is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT: {
                VkPhysicalDeviceMemoryPriorityFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMemoryPriorityFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(current);
                if (enabling->memoryPriority && !supported.memoryPriority) {
                    ss << "VkPhysicalDeviceMemoryPriorityFeaturesEXT::memoryPriority is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT: {
                VkPhysicalDeviceMeshShaderFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMeshShaderFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesEXT *>(current);
                if (enabling->taskShader && !supported.taskShader) {
                    ss << "VkPhysicalDeviceMeshShaderFeaturesEXT::taskShader is not supported\n";
                }
                if (enabling->meshShader && !supported.meshShader) {
                    ss << "VkPhysicalDeviceMeshShaderFeaturesEXT::meshShader is not supported\n";
                }
                if (enabling->multiviewMeshShader && !supported.multiviewMeshShader) {
                    ss << "VkPhysicalDeviceMeshShaderFeaturesEXT::multiviewMeshShader is not supported\n";
                }
                if (enabling->primitiveFragmentShadingRateMeshShader && !supported.primitiveFragmentShadingRateMeshShader) {
                    ss << "VkPhysicalDeviceMeshShaderFeaturesEXT::primitiveFragmentShadingRateMeshShader is not supported\n";
                }
                if (enabling->meshShaderQueries && !supported.meshShaderQueries) {
                    ss << "VkPhysicalDeviceMeshShaderFeaturesEXT::meshShaderQueries is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV: {
                VkPhysicalDeviceMeshShaderFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMeshShaderFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesNV *>(current);
                if (enabling->taskShader && !supported.taskShader) {
                    ss << "VkPhysicalDeviceMeshShaderFeaturesNV::taskShader is not supported\n";
                }
                if (enabling->meshShader && !supported.meshShader) {
                    ss << "VkPhysicalDeviceMeshShaderFeaturesNV::meshShader is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT: {
                VkPhysicalDeviceMultiDrawFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMultiDrawFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMultiDrawFeaturesEXT *>(current);
                if (enabling->multiDraw && !supported.multiDraw) {
                    ss << "VkPhysicalDeviceMultiDrawFeaturesEXT::multiDraw is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT: {
                VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *>(current);
                if (enabling->multisampledRenderToSingleSampled && !supported.multisampledRenderToSingleSampled) {
                    ss << "VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT::multisampledRenderToSingleSampled is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES: {
                VkPhysicalDeviceMultiviewFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMultiviewFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMultiviewFeatures *>(current);
                if (enabling->multiview && !supported.multiview) {
                    ss << "VkPhysicalDeviceMultiviewFeatures::multiview is not supported\n";
                }
                if (enabling->multiviewGeometryShader && !supported.multiviewGeometryShader) {
                    ss << "VkPhysicalDeviceMultiviewFeatures::multiviewGeometryShader is not supported\n";
                }
                if (enabling->multiviewTessellationShader && !supported.multiviewTessellationShader) {
                    ss << "VkPhysicalDeviceMultiviewFeatures::multiviewTessellationShader is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM: {
                VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *>(current);
                if (enabling->multiviewPerViewRenderAreas && !supported.multiviewPerViewRenderAreas) {
                    ss << "VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM::multiviewPerViewRenderAreas is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM: {
                VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *>(current);
                if (enabling->multiviewPerViewViewports && !supported.multiviewPerViewViewports) {
                    ss << "VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM::multiviewPerViewViewports is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT: {
                VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *>(current);
                if (enabling->mutableDescriptorType && !supported.mutableDescriptorType) {
                    ss << "VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::mutableDescriptorType is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT: {
                VkPhysicalDeviceNestedCommandBufferFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceNestedCommandBufferFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(current);
                if (enabling->nestedCommandBuffer && !supported.nestedCommandBuffer) {
                    ss << "VkPhysicalDeviceNestedCommandBufferFeaturesEXT::nestedCommandBuffer is not supported\n";
                }
                if (enabling->nestedCommandBufferRendering && !supported.nestedCommandBufferRendering) {
                    ss << "VkPhysicalDeviceNestedCommandBufferFeaturesEXT::nestedCommandBufferRendering is not supported\n";
                }
                if (enabling->nestedCommandBufferSimultaneousUse && !supported.nestedCommandBufferSimultaneousUse) {
                    ss << "VkPhysicalDeviceNestedCommandBufferFeaturesEXT::nestedCommandBufferSimultaneousUse is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT: {
                VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *>(current);
                if (enabling->nonSeamlessCubeMap && !supported.nonSeamlessCubeMap) {
                    ss << "VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT::nonSeamlessCubeMap is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT: {
                VkPhysicalDeviceOpacityMicromapFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceOpacityMicromapFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(current);
                if (enabling->micromap && !supported.micromap) {
                    ss << "VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromap is not supported\n";
                }
                if (enabling->micromapCaptureReplay && !supported.micromapCaptureReplay) {
                    ss << "VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromapCaptureReplay is not supported\n";
                }
                if (enabling->micromapHostCommands && !supported.micromapHostCommands) {
                    ss << "VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromapHostCommands is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV: {
                VkPhysicalDeviceOpticalFlowFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceOpticalFlowFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceOpticalFlowFeaturesNV *>(current);
                if (enabling->opticalFlow && !supported.opticalFlow) {
                    ss << "VkPhysicalDeviceOpticalFlowFeaturesNV::opticalFlow is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT: {
                VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *>(current);
                if (enabling->pageableDeviceLocalMemory && !supported.pageableDeviceLocalMemory) {
                    ss << "VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::pageableDeviceLocalMemory is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV: {
                VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV *>(current);
                if (enabling->partitionedAccelerationStructure && !supported.partitionedAccelerationStructure) {
                    ss << "VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV::partitionedAccelerationStructure is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV: {
                VkPhysicalDevicePerStageDescriptorSetFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePerStageDescriptorSetFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDevicePerStageDescriptorSetFeaturesNV *>(current);
                if (enabling->perStageDescriptorSet && !supported.perStageDescriptorSet) {
                    ss << "VkPhysicalDevicePerStageDescriptorSetFeaturesNV::perStageDescriptorSet is not supported\n";
                }
                if (enabling->dynamicPipelineLayout && !supported.dynamicPipelineLayout) {
                    ss << "VkPhysicalDevicePerStageDescriptorSetFeaturesNV::dynamicPipelineLayout is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_COUNTERS_BY_REGION_FEATURES_ARM: {
                VkPhysicalDevicePerformanceCountersByRegionFeaturesARM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePerformanceCountersByRegionFeaturesARM *enabling =
                    reinterpret_cast<const VkPhysicalDevicePerformanceCountersByRegionFeaturesARM *>(current);
                if (enabling->performanceCountersByRegion && !supported.performanceCountersByRegion) {
                    ss << "VkPhysicalDevicePerformanceCountersByRegionFeaturesARM::performanceCountersByRegion is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR: {
                VkPhysicalDevicePerformanceQueryFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePerformanceQueryFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDevicePerformanceQueryFeaturesKHR *>(current);
                if (enabling->performanceCounterQueryPools && !supported.performanceCounterQueryPools) {
                    ss << "VkPhysicalDevicePerformanceQueryFeaturesKHR::performanceCounterQueryPools is not supported\n";
                }
                if (enabling->performanceCounterMultipleQueryPools && !supported.performanceCounterMultipleQueryPools) {
                    ss << "VkPhysicalDevicePerformanceQueryFeaturesKHR::performanceCounterMultipleQueryPools is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR: {
                VkPhysicalDevicePipelineBinaryFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePipelineBinaryFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDevicePipelineBinaryFeaturesKHR *>(current);
                if (enabling->pipelineBinaries && !supported.pipelineBinaries) {
                    ss << "VkPhysicalDevicePipelineBinaryFeaturesKHR::pipelineBinaries is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CACHE_INCREMENTAL_MODE_FEATURES_SEC: {
                VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC *enabling =
                    reinterpret_cast<const VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC *>(current);
                if (enabling->pipelineCacheIncrementalMode && !supported.pipelineCacheIncrementalMode) {
                    ss << "VkPhysicalDevicePipelineCacheIncrementalModeFeaturesSEC::pipelineCacheIncrementalMode is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES: {
                VkPhysicalDevicePipelineCreationCacheControlFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePipelineCreationCacheControlFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeatures *>(current);
                if (enabling->pipelineCreationCacheControl && !supported.pipelineCreationCacheControl) {
                    ss << "VkPhysicalDevicePipelineCreationCacheControlFeatures::pipelineCreationCacheControl is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR: {
                VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(current);
                if (enabling->pipelineExecutableInfo && !supported.pipelineExecutableInfo) {
                    ss << "VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::pipelineExecutableInfo is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT: {
                VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *>(current);
                if (enabling->pipelineLibraryGroupHandles && !supported.pipelineLibraryGroupHandles) {
                    ss << "VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT::pipelineLibraryGroupHandles is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM: {
                VkPhysicalDevicePipelineOpacityMicromapFeaturesARM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM *enabling =
                    reinterpret_cast<const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM *>(current);
                if (enabling->pipelineOpacityMicromap && !supported.pipelineOpacityMicromap) {
                    ss << "VkPhysicalDevicePipelineOpacityMicromapFeaturesARM::pipelineOpacityMicromap is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT: {
                VkPhysicalDevicePipelinePropertiesFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePipelinePropertiesFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDevicePipelinePropertiesFeaturesEXT *>(current);
                if (enabling->pipelinePropertiesIdentifier && !supported.pipelinePropertiesIdentifier) {
                    ss << "VkPhysicalDevicePipelinePropertiesFeaturesEXT::pipelinePropertiesIdentifier is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES: {
                VkPhysicalDevicePipelineProtectedAccessFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePipelineProtectedAccessFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeatures *>(current);
                if (enabling->pipelineProtectedAccess && !supported.pipelineProtectedAccess) {
                    ss << "VkPhysicalDevicePipelineProtectedAccessFeatures::pipelineProtectedAccess is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES: {
                VkPhysicalDevicePipelineRobustnessFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePipelineRobustnessFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeatures *>(current);
                if (enabling->pipelineRobustness && !supported.pipelineRobustness) {
                    ss << "VkPhysicalDevicePipelineRobustnessFeatures::pipelineRobustness is not supported\n";
                }
                break;
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR: {
                VkPhysicalDevicePortabilitySubsetFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePortabilitySubsetFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(current);
                if (enabling->constantAlphaColorBlendFactors && !supported.constantAlphaColorBlendFactors) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::constantAlphaColorBlendFactors is not supported\n";
                }
                if (enabling->events && !supported.events) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::events is not supported\n";
                }
                if (enabling->imageViewFormatReinterpretation && !supported.imageViewFormatReinterpretation) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::imageViewFormatReinterpretation is not supported\n";
                }
                if (enabling->imageViewFormatSwizzle && !supported.imageViewFormatSwizzle) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::imageViewFormatSwizzle is not supported\n";
                }
                if (enabling->imageView2DOn3DImage && !supported.imageView2DOn3DImage) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::imageView2DOn3DImage is not supported\n";
                }
                if (enabling->multisampleArrayImage && !supported.multisampleArrayImage) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::multisampleArrayImage is not supported\n";
                }
                if (enabling->mutableComparisonSamplers && !supported.mutableComparisonSamplers) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::mutableComparisonSamplers is not supported\n";
                }
                if (enabling->pointPolygons && !supported.pointPolygons) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::pointPolygons is not supported\n";
                }
                if (enabling->samplerMipLodBias && !supported.samplerMipLodBias) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::samplerMipLodBias is not supported\n";
                }
                if (enabling->separateStencilMaskRef && !supported.separateStencilMaskRef) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::separateStencilMaskRef is not supported\n";
                }
                if (enabling->shaderSampleRateInterpolationFunctions && !supported.shaderSampleRateInterpolationFunctions) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::shaderSampleRateInterpolationFunctions is not supported\n";
                }
                if (enabling->tessellationIsolines && !supported.tessellationIsolines) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::tessellationIsolines is not supported\n";
                }
                if (enabling->tessellationPointMode && !supported.tessellationPointMode) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::tessellationPointMode is not supported\n";
                }
                if (enabling->triangleFans && !supported.triangleFans) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::triangleFans is not supported\n";
                }
                if (enabling->vertexAttributeAccessBeyondStride && !supported.vertexAttributeAccessBeyondStride) {
                    ss << "VkPhysicalDevicePortabilitySubsetFeaturesKHR::vertexAttributeAccessBeyondStride is not supported\n";
                }
                break;
            }
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV: {
                VkPhysicalDevicePresentBarrierFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePresentBarrierFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDevicePresentBarrierFeaturesNV *>(current);
                if (enabling->presentBarrier && !supported.presentBarrier) {
                    ss << "VkPhysicalDevicePresentBarrierFeaturesNV::presentBarrier is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_2_FEATURES_KHR: {
                VkPhysicalDevicePresentId2FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePresentId2FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDevicePresentId2FeaturesKHR *>(current);
                if (enabling->presentId2 && !supported.presentId2) {
                    ss << "VkPhysicalDevicePresentId2FeaturesKHR::presentId2 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR: {
                VkPhysicalDevicePresentIdFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePresentIdFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDevicePresentIdFeaturesKHR *>(current);
                if (enabling->presentId && !supported.presentId) {
                    ss << "VkPhysicalDevicePresentIdFeaturesKHR::presentId is not supported\n";
                }
                break;
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_METERING_FEATURES_NV: {
                VkPhysicalDevicePresentMeteringFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePresentMeteringFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDevicePresentMeteringFeaturesNV *>(current);
                if (enabling->presentMetering && !supported.presentMetering) {
                    ss << "VkPhysicalDevicePresentMeteringFeaturesNV::presentMetering is not supported\n";
                }
                break;
            }
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_KHR: {
                VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR *>(current);
                if (enabling->presentModeFifoLatestReady && !supported.presentModeFifoLatestReady) {
                    ss << "VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR::presentModeFifoLatestReady is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_2_FEATURES_KHR: {
                VkPhysicalDevicePresentWait2FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePresentWait2FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDevicePresentWait2FeaturesKHR *>(current);
                if (enabling->presentWait2 && !supported.presentWait2) {
                    ss << "VkPhysicalDevicePresentWait2FeaturesKHR::presentWait2 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR: {
                VkPhysicalDevicePresentWaitFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePresentWaitFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDevicePresentWaitFeaturesKHR *>(current);
                if (enabling->presentWait && !supported.presentWait) {
                    ss << "VkPhysicalDevicePresentWaitFeaturesKHR::presentWait is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT: {
                VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(current);
                if (enabling->primitiveTopologyListRestart && !supported.primitiveTopologyListRestart) {
                    ss << "VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::primitiveTopologyListRestart is not "
                          "supported\n";
                }
                if (enabling->primitiveTopologyPatchListRestart && !supported.primitiveTopologyPatchListRestart) {
                    ss << "VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::primitiveTopologyPatchListRestart is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT: {
                VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(current);
                if (enabling->primitivesGeneratedQuery && !supported.primitivesGeneratedQuery) {
                    ss << "VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::primitivesGeneratedQuery is not supported\n";
                }
                if (enabling->primitivesGeneratedQueryWithRasterizerDiscard &&
                    !supported.primitivesGeneratedQueryWithRasterizerDiscard) {
                    ss << "VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::primitivesGeneratedQueryWithRasterizerDiscard is "
                          "not supported\n";
                }
                if (enabling->primitivesGeneratedQueryWithNonZeroStreams && !supported.primitivesGeneratedQueryWithNonZeroStreams) {
                    ss << "VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::primitivesGeneratedQueryWithNonZeroStreams is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES: {
                VkPhysicalDevicePrivateDataFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDevicePrivateDataFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDevicePrivateDataFeatures *>(current);
                if (enabling->privateData && !supported.privateData) {
                    ss << "VkPhysicalDevicePrivateDataFeatures::privateData is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES: {
                VkPhysicalDeviceProtectedMemoryFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceProtectedMemoryFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceProtectedMemoryFeatures *>(current);
                if (enabling->protectedMemory && !supported.protectedMemory) {
                    ss << "VkPhysicalDeviceProtectedMemoryFeatures::protectedMemory is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT: {
                VkPhysicalDeviceProvokingVertexFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceProvokingVertexFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceProvokingVertexFeaturesEXT *>(current);
                if (enabling->provokingVertexLast && !supported.provokingVertexLast) {
                    ss << "VkPhysicalDeviceProvokingVertexFeaturesEXT::provokingVertexLast is not supported\n";
                }
                if (enabling->transformFeedbackPreservesProvokingVertex && !supported.transformFeedbackPreservesProvokingVertex) {
                    ss << "VkPhysicalDeviceProvokingVertexFeaturesEXT::transformFeedbackPreservesProvokingVertex is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT: {
                VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *>(current);
                if (enabling->formatRgba10x6WithoutYCbCrSampler && !supported.formatRgba10x6WithoutYCbCrSampler) {
                    ss << "VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT::formatRgba10x6WithoutYCbCrSampler is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT: {
                VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(current);
                if (enabling->rasterizationOrderColorAttachmentAccess && !supported.rasterizationOrderColorAttachmentAccess) {
                    ss << "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::rasterizationOrderColorAttachmentAccess "
                          "is not supported\n";
                }
                if (enabling->rasterizationOrderDepthAttachmentAccess && !supported.rasterizationOrderDepthAttachmentAccess) {
                    ss << "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::rasterizationOrderDepthAttachmentAccess "
                          "is not supported\n";
                }
                if (enabling->rasterizationOrderStencilAttachmentAccess && !supported.rasterizationOrderStencilAttachmentAccess) {
                    ss << "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::"
                          "rasterizationOrderStencilAttachmentAccess is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV: {
                VkPhysicalDeviceRawAccessChainsFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRawAccessChainsFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRawAccessChainsFeaturesNV *>(current);
                if (enabling->shaderRawAccessChains && !supported.shaderRawAccessChains) {
                    ss << "VkPhysicalDeviceRawAccessChainsFeaturesNV::shaderRawAccessChains is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR: {
                VkPhysicalDeviceRayQueryFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRayQueryFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRayQueryFeaturesKHR *>(current);
                if (enabling->rayQuery && !supported.rayQuery) {
                    ss << "VkPhysicalDeviceRayQueryFeaturesKHR::rayQuery is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV: {
                VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *>(current);
                if (enabling->rayTracingInvocationReorder && !supported.rayTracingInvocationReorder) {
                    ss << "VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV::rayTracingInvocationReorder is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV: {
                VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV *>(current);
                if (enabling->spheres && !supported.spheres) {
                    ss << "VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::spheres is not supported\n";
                }
                if (enabling->linearSweptSpheres && !supported.linearSweptSpheres) {
                    ss << "VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::linearSweptSpheres is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR: {
                VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(current);
                if (enabling->rayTracingMaintenance1 && !supported.rayTracingMaintenance1) {
                    ss << "VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::rayTracingMaintenance1 is not supported\n";
                }
                if (enabling->rayTracingPipelineTraceRaysIndirect2 && !supported.rayTracingPipelineTraceRaysIndirect2) {
                    ss << "VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::rayTracingPipelineTraceRaysIndirect2 is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV: {
                VkPhysicalDeviceRayTracingMotionBlurFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(current);
                if (enabling->rayTracingMotionBlur && !supported.rayTracingMotionBlur) {
                    ss << "VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::rayTracingMotionBlur is not supported\n";
                }
                if (enabling->rayTracingMotionBlurPipelineTraceRaysIndirect &&
                    !supported.rayTracingMotionBlurPipelineTraceRaysIndirect) {
                    ss << "VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::rayTracingMotionBlurPipelineTraceRaysIndirect is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR: {
                VkPhysicalDeviceRayTracingPipelineFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRayTracingPipelineFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(current);
                if (enabling->rayTracingPipeline && !supported.rayTracingPipeline) {
                    ss << "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipeline is not supported\n";
                }
                if (enabling->rayTracingPipelineShaderGroupHandleCaptureReplay &&
                    !supported.rayTracingPipelineShaderGroupHandleCaptureReplay) {
                    ss << "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineShaderGroupHandleCaptureReplay is not "
                          "supported\n";
                }
                if (enabling->rayTracingPipelineShaderGroupHandleCaptureReplayMixed &&
                    !supported.rayTracingPipelineShaderGroupHandleCaptureReplayMixed) {
                    ss << "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineShaderGroupHandleCaptureReplayMixed is "
                          "not supported\n";
                }
                if (enabling->rayTracingPipelineTraceRaysIndirect && !supported.rayTracingPipelineTraceRaysIndirect) {
                    ss << "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineTraceRaysIndirect is not supported\n";
                }
                if (enabling->rayTraversalPrimitiveCulling && !supported.rayTraversalPrimitiveCulling) {
                    ss << "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTraversalPrimitiveCulling is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR: {
                VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *>(current);
                if (enabling->rayTracingPositionFetch && !supported.rayTracingPositionFetch) {
                    ss << "VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::rayTracingPositionFetch is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV: {
                VkPhysicalDeviceRayTracingValidationFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRayTracingValidationFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRayTracingValidationFeaturesNV *>(current);
                if (enabling->rayTracingValidation && !supported.rayTracingValidation) {
                    ss << "VkPhysicalDeviceRayTracingValidationFeaturesNV::rayTracingValidation is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG: {
                VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG *>(current);
                if (enabling->relaxedLineRasterization && !supported.relaxedLineRasterization) {
                    ss << "VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG::relaxedLineRasterization is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM: {
                VkPhysicalDeviceRenderPassStripedFeaturesARM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRenderPassStripedFeaturesARM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRenderPassStripedFeaturesARM *>(current);
                if (enabling->renderPassStriped && !supported.renderPassStriped) {
                    ss << "VkPhysicalDeviceRenderPassStripedFeaturesARM::renderPassStriped is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV: {
                VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(current);
                if (enabling->representativeFragmentTest && !supported.representativeFragmentTest) {
                    ss << "VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::representativeFragmentTest is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_KHR: {
                VkPhysicalDeviceRobustness2FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceRobustness2FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesKHR *>(current);
                if (enabling->robustBufferAccess2 && !supported.robustBufferAccess2) {
                    ss << "VkPhysicalDeviceRobustness2FeaturesKHR::robustBufferAccess2 is not supported\n";
                }
                if (enabling->robustImageAccess2 && !supported.robustImageAccess2) {
                    ss << "VkPhysicalDeviceRobustness2FeaturesKHR::robustImageAccess2 is not supported\n";
                }
                if (enabling->nullDescriptor && !supported.nullDescriptor) {
                    ss << "VkPhysicalDeviceRobustness2FeaturesKHR::nullDescriptor is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES: {
                VkPhysicalDeviceSamplerYcbcrConversionFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceSamplerYcbcrConversionFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(current);
                if (enabling->samplerYcbcrConversion && !supported.samplerYcbcrConversion) {
                    ss << "VkPhysicalDeviceSamplerYcbcrConversionFeatures::samplerYcbcrConversion is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES: {
                VkPhysicalDeviceScalarBlockLayoutFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceScalarBlockLayoutFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeatures *>(current);
                if (enabling->scalarBlockLayout && !supported.scalarBlockLayout) {
                    ss << "VkPhysicalDeviceScalarBlockLayoutFeatures::scalarBlockLayout is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM: {
                VkPhysicalDeviceSchedulingControlsFeaturesARM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceSchedulingControlsFeaturesARM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceSchedulingControlsFeaturesARM *>(current);
                if (enabling->schedulingControls && !supported.schedulingControls) {
                    ss << "VkPhysicalDeviceSchedulingControlsFeaturesARM::schedulingControls is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES: {
                VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(current);
                if (enabling->separateDepthStencilLayouts && !supported.separateDepthStencilLayouts) {
                    ss << "VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures::separateDepthStencilLayouts is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_64_BIT_INDEXING_FEATURES_EXT: {
                VkPhysicalDeviceShader64BitIndexingFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShader64BitIndexingFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShader64BitIndexingFeaturesEXT *>(current);
                if (enabling->shader64BitIndexing && !supported.shader64BitIndexing) {
                    ss << "VkPhysicalDeviceShader64BitIndexingFeaturesEXT::shader64BitIndexing is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV: {
                VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV *>(current);
                if (enabling->shaderFloat16VectorAtomics && !supported.shaderFloat16VectorAtomics) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV::shaderFloat16VectorAtomics is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT: {
                VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(current);
                if (enabling->shaderBufferFloat16Atomics && !supported.shaderBufferFloat16Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16Atomics is not supported\n";
                }
                if (enabling->shaderBufferFloat16AtomicAdd && !supported.shaderBufferFloat16AtomicAdd) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16AtomicAdd is not supported\n";
                }
                if (enabling->shaderBufferFloat16AtomicMinMax && !supported.shaderBufferFloat16AtomicMinMax) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16AtomicMinMax is not supported\n";
                }
                if (enabling->shaderBufferFloat32AtomicMinMax && !supported.shaderBufferFloat32AtomicMinMax) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat32AtomicMinMax is not supported\n";
                }
                if (enabling->shaderBufferFloat64AtomicMinMax && !supported.shaderBufferFloat64AtomicMinMax) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat64AtomicMinMax is not supported\n";
                }
                if (enabling->shaderSharedFloat16Atomics && !supported.shaderSharedFloat16Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16Atomics is not supported\n";
                }
                if (enabling->shaderSharedFloat16AtomicAdd && !supported.shaderSharedFloat16AtomicAdd) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16AtomicAdd is not supported\n";
                }
                if (enabling->shaderSharedFloat16AtomicMinMax && !supported.shaderSharedFloat16AtomicMinMax) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16AtomicMinMax is not supported\n";
                }
                if (enabling->shaderSharedFloat32AtomicMinMax && !supported.shaderSharedFloat32AtomicMinMax) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat32AtomicMinMax is not supported\n";
                }
                if (enabling->shaderSharedFloat64AtomicMinMax && !supported.shaderSharedFloat64AtomicMinMax) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat64AtomicMinMax is not supported\n";
                }
                if (enabling->shaderImageFloat32AtomicMinMax && !supported.shaderImageFloat32AtomicMinMax) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderImageFloat32AtomicMinMax is not supported\n";
                }
                if (enabling->sparseImageFloat32AtomicMinMax && !supported.sparseImageFloat32AtomicMinMax) {
                    ss << "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::sparseImageFloat32AtomicMinMax is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT: {
                VkPhysicalDeviceShaderAtomicFloatFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(current);
                if (enabling->shaderBufferFloat32Atomics && !supported.shaderBufferFloat32Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat32Atomics is not supported\n";
                }
                if (enabling->shaderBufferFloat32AtomicAdd && !supported.shaderBufferFloat32AtomicAdd) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat32AtomicAdd is not supported\n";
                }
                if (enabling->shaderBufferFloat64Atomics && !supported.shaderBufferFloat64Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat64Atomics is not supported\n";
                }
                if (enabling->shaderBufferFloat64AtomicAdd && !supported.shaderBufferFloat64AtomicAdd) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat64AtomicAdd is not supported\n";
                }
                if (enabling->shaderSharedFloat32Atomics && !supported.shaderSharedFloat32Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat32Atomics is not supported\n";
                }
                if (enabling->shaderSharedFloat32AtomicAdd && !supported.shaderSharedFloat32AtomicAdd) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat32AtomicAdd is not supported\n";
                }
                if (enabling->shaderSharedFloat64Atomics && !supported.shaderSharedFloat64Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat64Atomics is not supported\n";
                }
                if (enabling->shaderSharedFloat64AtomicAdd && !supported.shaderSharedFloat64AtomicAdd) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat64AtomicAdd is not supported\n";
                }
                if (enabling->shaderImageFloat32Atomics && !supported.shaderImageFloat32Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderImageFloat32Atomics is not supported\n";
                }
                if (enabling->shaderImageFloat32AtomicAdd && !supported.shaderImageFloat32AtomicAdd) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderImageFloat32AtomicAdd is not supported\n";
                }
                if (enabling->sparseImageFloat32Atomics && !supported.sparseImageFloat32Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::sparseImageFloat32Atomics is not supported\n";
                }
                if (enabling->sparseImageFloat32AtomicAdd && !supported.sparseImageFloat32AtomicAdd) {
                    ss << "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::sparseImageFloat32AtomicAdd is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES: {
                VkPhysicalDeviceShaderAtomicInt64Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderAtomicInt64Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64Features *>(current);
                if (enabling->shaderBufferInt64Atomics && !supported.shaderBufferInt64Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicInt64Features::shaderBufferInt64Atomics is not supported\n";
                }
                if (enabling->shaderSharedInt64Atomics && !supported.shaderSharedInt64Atomics) {
                    ss << "VkPhysicalDeviceShaderAtomicInt64Features::shaderSharedInt64Atomics is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_BFLOAT16_FEATURES_KHR: {
                VkPhysicalDeviceShaderBfloat16FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderBfloat16FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderBfloat16FeaturesKHR *>(current);
                if (enabling->shaderBFloat16Type && !supported.shaderBFloat16Type) {
                    ss << "VkPhysicalDeviceShaderBfloat16FeaturesKHR::shaderBFloat16Type is not supported\n";
                }
                if (enabling->shaderBFloat16DotProduct && !supported.shaderBFloat16DotProduct) {
                    ss << "VkPhysicalDeviceShaderBfloat16FeaturesKHR::shaderBFloat16DotProduct is not supported\n";
                }
                if (enabling->shaderBFloat16CooperativeMatrix && !supported.shaderBFloat16CooperativeMatrix) {
                    ss << "VkPhysicalDeviceShaderBfloat16FeaturesKHR::shaderBFloat16CooperativeMatrix is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR: {
                VkPhysicalDeviceShaderClockFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderClockFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderClockFeaturesKHR *>(current);
                if (enabling->shaderSubgroupClock && !supported.shaderSubgroupClock) {
                    ss << "VkPhysicalDeviceShaderClockFeaturesKHR::shaderSubgroupClock is not supported\n";
                }
                if (enabling->shaderDeviceClock && !supported.shaderDeviceClock) {
                    ss << "VkPhysicalDeviceShaderClockFeaturesKHR::shaderDeviceClock is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM: {
                VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *>(current);
                if (enabling->shaderCoreBuiltins && !supported.shaderCoreBuiltins) {
                    ss << "VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::shaderCoreBuiltins is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES: {
                VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(current);
                if (enabling->shaderDemoteToHelperInvocation && !supported.shaderDemoteToHelperInvocation) {
                    ss << "VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures::shaderDemoteToHelperInvocation is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES: {
                VkPhysicalDeviceShaderDrawParametersFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderDrawParametersFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderDrawParametersFeatures *>(current);
                if (enabling->shaderDrawParameters && !supported.shaderDrawParameters) {
                    ss << "VkPhysicalDeviceShaderDrawParametersFeatures::shaderDrawParameters is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD: {
                VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *>(current);
                if (enabling->shaderEarlyAndLateFragmentTests && !supported.shaderEarlyAndLateFragmentTests) {
                    ss << "VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD::shaderEarlyAndLateFragmentTests is not "
                          "supported\n";
                }
                break;
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX: {
                VkPhysicalDeviceShaderEnqueueFeaturesAMDX supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderEnqueueFeaturesAMDX *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderEnqueueFeaturesAMDX *>(current);
                if (enabling->shaderEnqueue && !supported.shaderEnqueue) {
                    ss << "VkPhysicalDeviceShaderEnqueueFeaturesAMDX::shaderEnqueue is not supported\n";
                }
                if (enabling->shaderMeshEnqueue && !supported.shaderMeshEnqueue) {
                    ss << "VkPhysicalDeviceShaderEnqueueFeaturesAMDX::shaderMeshEnqueue is not supported\n";
                }
                break;
            }
#endif  // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES: {
                VkPhysicalDeviceShaderExpectAssumeFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderExpectAssumeFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderExpectAssumeFeatures *>(current);
                if (enabling->shaderExpectAssume && !supported.shaderExpectAssume) {
                    ss << "VkPhysicalDeviceShaderExpectAssumeFeatures::shaderExpectAssume is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES: {
                VkPhysicalDeviceShaderFloat16Int8Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderFloat16Int8Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8Features *>(current);
                if (enabling->shaderFloat16 && !supported.shaderFloat16) {
                    ss << "VkPhysicalDeviceShaderFloat16Int8Features::shaderFloat16 is not supported\n";
                }
                if (enabling->shaderInt8 && !supported.shaderInt8) {
                    ss << "VkPhysicalDeviceShaderFloat16Int8Features::shaderInt8 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT8_FEATURES_EXT: {
                VkPhysicalDeviceShaderFloat8FeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderFloat8FeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderFloat8FeaturesEXT *>(current);
                if (enabling->shaderFloat8 && !supported.shaderFloat8) {
                    ss << "VkPhysicalDeviceShaderFloat8FeaturesEXT::shaderFloat8 is not supported\n";
                }
                if (enabling->shaderFloat8CooperativeMatrix && !supported.shaderFloat8CooperativeMatrix) {
                    ss << "VkPhysicalDeviceShaderFloat8FeaturesEXT::shaderFloat8CooperativeMatrix is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES: {
                VkPhysicalDeviceShaderFloatControls2Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderFloatControls2Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderFloatControls2Features *>(current);
                if (enabling->shaderFloatControls2 && !supported.shaderFloatControls2) {
                    ss << "VkPhysicalDeviceShaderFloatControls2Features::shaderFloatControls2 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FMA_FEATURES_KHR: {
                VkPhysicalDeviceShaderFmaFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderFmaFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderFmaFeaturesKHR *>(current);
                if (enabling->shaderFmaFloat16 && !supported.shaderFmaFloat16) {
                    ss << "VkPhysicalDeviceShaderFmaFeaturesKHR::shaderFmaFloat16 is not supported\n";
                }
                if (enabling->shaderFmaFloat32 && !supported.shaderFmaFloat32) {
                    ss << "VkPhysicalDeviceShaderFmaFeaturesKHR::shaderFmaFloat32 is not supported\n";
                }
                if (enabling->shaderFmaFloat64 && !supported.shaderFmaFloat64) {
                    ss << "VkPhysicalDeviceShaderFmaFeaturesKHR::shaderFmaFloat64 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT: {
                VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(current);
                if (enabling->shaderImageInt64Atomics && !supported.shaderImageInt64Atomics) {
                    ss << "VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::shaderImageInt64Atomics is not supported\n";
                }
                if (enabling->sparseImageInt64Atomics && !supported.sparseImageInt64Atomics) {
                    ss << "VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::sparseImageInt64Atomics is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV: {
                VkPhysicalDeviceShaderImageFootprintFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderImageFootprintFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(current);
                if (enabling->imageFootprint && !supported.imageFootprint) {
                    ss << "VkPhysicalDeviceShaderImageFootprintFeaturesNV::imageFootprint is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES: {
                VkPhysicalDeviceShaderIntegerDotProductFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderIntegerDotProductFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeatures *>(current);
                if (enabling->shaderIntegerDotProduct && !supported.shaderIntegerDotProduct) {
                    ss << "VkPhysicalDeviceShaderIntegerDotProductFeatures::shaderIntegerDotProduct is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL: {
                VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(current);
                if (enabling->shaderIntegerFunctions2 && !supported.shaderIntegerFunctions2) {
                    ss << "VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::shaderIntegerFunctions2 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR: {
                VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR *>(current);
                if (enabling->shaderMaximalReconvergence && !supported.shaderMaximalReconvergence) {
                    ss << "VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR::shaderMaximalReconvergence is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT: {
                VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *>(current);
                if (enabling->shaderModuleIdentifier && !supported.shaderModuleIdentifier) {
                    ss << "VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT::shaderModuleIdentifier is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT: {
                VkPhysicalDeviceShaderObjectFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderObjectFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderObjectFeaturesEXT *>(current);
                if (enabling->shaderObject && !supported.shaderObject) {
                    ss << "VkPhysicalDeviceShaderObjectFeaturesEXT::shaderObject is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR: {
                VkPhysicalDeviceShaderQuadControlFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderQuadControlFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderQuadControlFeaturesKHR *>(current);
                if (enabling->shaderQuadControl && !supported.shaderQuadControl) {
                    ss << "VkPhysicalDeviceShaderQuadControlFeaturesKHR::shaderQuadControl is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR: {
                VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR *>(current);
                if (enabling->shaderRelaxedExtendedInstruction && !supported.shaderRelaxedExtendedInstruction) {
                    ss << "VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR::shaderRelaxedExtendedInstruction is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT: {
                VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT *>(current);
                if (enabling->shaderReplicatedComposites && !supported.shaderReplicatedComposites) {
                    ss << "VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT::shaderReplicatedComposites is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV: {
                VkPhysicalDeviceShaderSMBuiltinsFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(current);
                if (enabling->shaderSMBuiltins && !supported.shaderSMBuiltins) {
                    ss << "VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::shaderSMBuiltins is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES: {
                VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(current);
                if (enabling->shaderSubgroupExtendedTypes && !supported.shaderSubgroupExtendedTypes) {
                    ss << "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures::shaderSubgroupExtendedTypes is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES: {
                VkPhysicalDeviceShaderSubgroupRotateFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderSubgroupRotateFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderSubgroupRotateFeatures *>(current);
                if (enabling->shaderSubgroupRotate && !supported.shaderSubgroupRotate) {
                    ss << "VkPhysicalDeviceShaderSubgroupRotateFeatures::shaderSubgroupRotate is not supported\n";
                }
                if (enabling->shaderSubgroupRotateClustered && !supported.shaderSubgroupRotateClustered) {
                    ss << "VkPhysicalDeviceShaderSubgroupRotateFeatures::shaderSubgroupRotateClustered is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR: {
                VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *>(current);
                if (enabling->shaderSubgroupUniformControlFlow && !supported.shaderSubgroupUniformControlFlow) {
                    ss << "VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::shaderSubgroupUniformControlFlow is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES: {
                VkPhysicalDeviceShaderTerminateInvocationFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderTerminateInvocationFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeatures *>(current);
                if (enabling->shaderTerminateInvocation && !supported.shaderTerminateInvocation) {
                    ss << "VkPhysicalDeviceShaderTerminateInvocationFeatures::shaderTerminateInvocation is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT: {
                VkPhysicalDeviceShaderTileImageFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderTileImageFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderTileImageFeaturesEXT *>(current);
                if (enabling->shaderTileImageColorReadAccess && !supported.shaderTileImageColorReadAccess) {
                    ss << "VkPhysicalDeviceShaderTileImageFeaturesEXT::shaderTileImageColorReadAccess is not supported\n";
                }
                if (enabling->shaderTileImageDepthReadAccess && !supported.shaderTileImageDepthReadAccess) {
                    ss << "VkPhysicalDeviceShaderTileImageFeaturesEXT::shaderTileImageDepthReadAccess is not supported\n";
                }
                if (enabling->shaderTileImageStencilReadAccess && !supported.shaderTileImageStencilReadAccess) {
                    ss << "VkPhysicalDeviceShaderTileImageFeaturesEXT::shaderTileImageStencilReadAccess is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNIFORM_BUFFER_UNSIZED_ARRAY_FEATURES_EXT: {
                VkPhysicalDeviceShaderUniformBufferUnsizedArrayFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderUniformBufferUnsizedArrayFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderUniformBufferUnsizedArrayFeaturesEXT *>(current);
                if (enabling->shaderUniformBufferUnsizedArray && !supported.shaderUniformBufferUnsizedArray) {
                    ss << "VkPhysicalDeviceShaderUniformBufferUnsizedArrayFeaturesEXT::shaderUniformBufferUnsizedArray is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR: {
                VkPhysicalDeviceShaderUntypedPointersFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShaderUntypedPointersFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShaderUntypedPointersFeaturesKHR *>(current);
                if (enabling->shaderUntypedPointers && !supported.shaderUntypedPointers) {
                    ss << "VkPhysicalDeviceShaderUntypedPointersFeaturesKHR::shaderUntypedPointers is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV: {
                VkPhysicalDeviceShadingRateImageFeaturesNV supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceShadingRateImageFeaturesNV *enabling =
                    reinterpret_cast<const VkPhysicalDeviceShadingRateImageFeaturesNV *>(current);
                if (enabling->shadingRateImage && !supported.shadingRateImage) {
                    ss << "VkPhysicalDeviceShadingRateImageFeaturesNV::shadingRateImage is not supported\n";
                }
                if (enabling->shadingRateCoarseSampleOrder && !supported.shadingRateCoarseSampleOrder) {
                    ss << "VkPhysicalDeviceShadingRateImageFeaturesNV::shadingRateCoarseSampleOrder is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES: {
                VkPhysicalDeviceSubgroupSizeControlFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceSubgroupSizeControlFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeatures *>(current);
                if (enabling->subgroupSizeControl && !supported.subgroupSizeControl) {
                    ss << "VkPhysicalDeviceSubgroupSizeControlFeatures::subgroupSizeControl is not supported\n";
                }
                if (enabling->computeFullSubgroups && !supported.computeFullSubgroups) {
                    ss << "VkPhysicalDeviceSubgroupSizeControlFeatures::computeFullSubgroups is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT: {
                VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *>(current);
                if (enabling->subpassMergeFeedback && !supported.subpassMergeFeedback) {
                    ss << "VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT::subpassMergeFeedback is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI: {
                VkPhysicalDeviceSubpassShadingFeaturesHUAWEI supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *enabling =
                    reinterpret_cast<const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *>(current);
                if (enabling->subpassShading && !supported.subpassShading) {
                    ss << "VkPhysicalDeviceSubpassShadingFeaturesHUAWEI::subpassShading is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR: {
                VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR *>(current);
                if (enabling->swapchainMaintenance1 && !supported.swapchainMaintenance1) {
                    ss << "VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR::swapchainMaintenance1 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES: {
                VkPhysicalDeviceSynchronization2Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceSynchronization2Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceSynchronization2Features *>(current);
                if (enabling->synchronization2 && !supported.synchronization2) {
                    ss << "VkPhysicalDeviceSynchronization2Features::synchronization2 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_FEATURES_ARM: {
                VkPhysicalDeviceTensorFeaturesARM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceTensorFeaturesARM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceTensorFeaturesARM *>(current);
                if (enabling->tensorNonPacked && !supported.tensorNonPacked) {
                    ss << "VkPhysicalDeviceTensorFeaturesARM::tensorNonPacked is not supported\n";
                }
                if (enabling->shaderTensorAccess && !supported.shaderTensorAccess) {
                    ss << "VkPhysicalDeviceTensorFeaturesARM::shaderTensorAccess is not supported\n";
                }
                if (enabling->shaderStorageTensorArrayDynamicIndexing && !supported.shaderStorageTensorArrayDynamicIndexing) {
                    ss << "VkPhysicalDeviceTensorFeaturesARM::shaderStorageTensorArrayDynamicIndexing is not supported\n";
                }
                if (enabling->shaderStorageTensorArrayNonUniformIndexing && !supported.shaderStorageTensorArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceTensorFeaturesARM::shaderStorageTensorArrayNonUniformIndexing is not supported\n";
                }
                if (enabling->descriptorBindingStorageTensorUpdateAfterBind &&
                    !supported.descriptorBindingStorageTensorUpdateAfterBind) {
                    ss << "VkPhysicalDeviceTensorFeaturesARM::descriptorBindingStorageTensorUpdateAfterBind is not supported\n";
                }
                if (enabling->tensors && !supported.tensors) {
                    ss << "VkPhysicalDeviceTensorFeaturesARM::tensors is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT: {
                VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(current);
                if (enabling->texelBufferAlignment && !supported.texelBufferAlignment) {
                    ss << "VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT::texelBufferAlignment is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES: {
                VkPhysicalDeviceTextureCompressionASTCHDRFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(current);
                if (enabling->textureCompressionASTC_HDR && !supported.textureCompressionASTC_HDR) {
                    ss << "VkPhysicalDeviceTextureCompressionASTCHDRFeatures::textureCompressionASTC_HDR is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_MEMORY_HEAP_FEATURES_QCOM: {
                VkPhysicalDeviceTileMemoryHeapFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceTileMemoryHeapFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceTileMemoryHeapFeaturesQCOM *>(current);
                if (enabling->tileMemoryHeap && !supported.tileMemoryHeap) {
                    ss << "VkPhysicalDeviceTileMemoryHeapFeaturesQCOM::tileMemoryHeap is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM: {
                VkPhysicalDeviceTilePropertiesFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceTilePropertiesFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceTilePropertiesFeaturesQCOM *>(current);
                if (enabling->tileProperties && !supported.tileProperties) {
                    ss << "VkPhysicalDeviceTilePropertiesFeaturesQCOM::tileProperties is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_SHADING_FEATURES_QCOM: {
                VkPhysicalDeviceTileShadingFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceTileShadingFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceTileShadingFeaturesQCOM *>(current);
                if (enabling->tileShading && !supported.tileShading) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShading is not supported\n";
                }
                if (enabling->tileShadingFragmentStage && !supported.tileShadingFragmentStage) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingFragmentStage is not supported\n";
                }
                if (enabling->tileShadingColorAttachments && !supported.tileShadingColorAttachments) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingColorAttachments is not supported\n";
                }
                if (enabling->tileShadingDepthAttachments && !supported.tileShadingDepthAttachments) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingDepthAttachments is not supported\n";
                }
                if (enabling->tileShadingStencilAttachments && !supported.tileShadingStencilAttachments) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingStencilAttachments is not supported\n";
                }
                if (enabling->tileShadingInputAttachments && !supported.tileShadingInputAttachments) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingInputAttachments is not supported\n";
                }
                if (enabling->tileShadingSampledAttachments && !supported.tileShadingSampledAttachments) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingSampledAttachments is not supported\n";
                }
                if (enabling->tileShadingPerTileDraw && !supported.tileShadingPerTileDraw) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingPerTileDraw is not supported\n";
                }
                if (enabling->tileShadingPerTileDispatch && !supported.tileShadingPerTileDispatch) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingPerTileDispatch is not supported\n";
                }
                if (enabling->tileShadingDispatchTile && !supported.tileShadingDispatchTile) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingDispatchTile is not supported\n";
                }
                if (enabling->tileShadingApron && !supported.tileShadingApron) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingApron is not supported\n";
                }
                if (enabling->tileShadingAnisotropicApron && !supported.tileShadingAnisotropicApron) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingAnisotropicApron is not supported\n";
                }
                if (enabling->tileShadingAtomicOps && !supported.tileShadingAtomicOps) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingAtomicOps is not supported\n";
                }
                if (enabling->tileShadingImageProcessing && !supported.tileShadingImageProcessing) {
                    ss << "VkPhysicalDeviceTileShadingFeaturesQCOM::tileShadingImageProcessing is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES: {
                VkPhysicalDeviceTimelineSemaphoreFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceTimelineSemaphoreFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures *>(current);
                if (enabling->timelineSemaphore && !supported.timelineSemaphore) {
                    ss << "VkPhysicalDeviceTimelineSemaphoreFeatures::timelineSemaphore is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT: {
                VkPhysicalDeviceTransformFeedbackFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceTransformFeedbackFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(current);
                if (enabling->transformFeedback && !supported.transformFeedback) {
                    ss << "VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback is not supported\n";
                }
                if (enabling->geometryStreams && !supported.geometryStreams) {
                    ss << "VkPhysicalDeviceTransformFeedbackFeaturesEXT::geometryStreams is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR: {
                VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR *>(current);
                if (enabling->unifiedImageLayouts && !supported.unifiedImageLayouts) {
                    ss << "VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR::unifiedImageLayouts is not supported\n";
                }
                if (enabling->unifiedImageLayoutsVideo && !supported.unifiedImageLayoutsVideo) {
                    ss << "VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR::unifiedImageLayoutsVideo is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES: {
                VkPhysicalDeviceUniformBufferStandardLayoutFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceUniformBufferStandardLayoutFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(current);
                if (enabling->uniformBufferStandardLayout && !supported.uniformBufferStandardLayout) {
                    ss << "VkPhysicalDeviceUniformBufferStandardLayoutFeatures::uniformBufferStandardLayout is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES: {
                VkPhysicalDeviceVariablePointersFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVariablePointersFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVariablePointersFeatures *>(current);
                if (enabling->variablePointersStorageBuffer && !supported.variablePointersStorageBuffer) {
                    ss << "VkPhysicalDeviceVariablePointersFeatures::variablePointersStorageBuffer is not supported\n";
                }
                if (enabling->variablePointers && !supported.variablePointers) {
                    ss << "VkPhysicalDeviceVariablePointersFeatures::variablePointers is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES: {
                VkPhysicalDeviceVertexAttributeDivisorFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVertexAttributeDivisorFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeatures *>(current);
                if (enabling->vertexAttributeInstanceRateDivisor && !supported.vertexAttributeInstanceRateDivisor) {
                    ss << "VkPhysicalDeviceVertexAttributeDivisorFeatures::vertexAttributeInstanceRateDivisor is not supported\n";
                }
                if (enabling->vertexAttributeInstanceRateZeroDivisor && !supported.vertexAttributeInstanceRateZeroDivisor) {
                    ss << "VkPhysicalDeviceVertexAttributeDivisorFeatures::vertexAttributeInstanceRateZeroDivisor is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT: {
                VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT *>(current);
                if (enabling->vertexAttributeRobustness && !supported.vertexAttributeRobustness) {
                    ss << "VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT::vertexAttributeRobustness is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT: {
                VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(current);
                if (enabling->vertexInputDynamicState && !supported.vertexInputDynamicState) {
                    ss << "VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT::vertexInputDynamicState is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_DECODE_VP9_FEATURES_KHR: {
                VkPhysicalDeviceVideoDecodeVP9FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVideoDecodeVP9FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVideoDecodeVP9FeaturesKHR *>(current);
                if (enabling->videoDecodeVP9 && !supported.videoDecodeVP9) {
                    ss << "VkPhysicalDeviceVideoDecodeVP9FeaturesKHR::videoDecodeVP9 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR: {
                VkPhysicalDeviceVideoEncodeAV1FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR *>(current);
                if (enabling->videoEncodeAV1 && !supported.videoEncodeAV1) {
                    ss << "VkPhysicalDeviceVideoEncodeAV1FeaturesKHR::videoEncodeAV1 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_INTRA_REFRESH_FEATURES_KHR: {
                VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR *>(current);
                if (enabling->videoEncodeIntraRefresh && !supported.videoEncodeIntraRefresh) {
                    ss << "VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR::videoEncodeIntraRefresh is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR: {
                VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR *>(current);
                if (enabling->videoEncodeQuantizationMap && !supported.videoEncodeQuantizationMap) {
                    ss << "VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR::videoEncodeQuantizationMap is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_RGB_CONVERSION_FEATURES_VALVE: {
                VkPhysicalDeviceVideoEncodeRgbConversionFeaturesVALVE supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVideoEncodeRgbConversionFeaturesVALVE *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVideoEncodeRgbConversionFeaturesVALVE *>(current);
                if (enabling->videoEncodeRgbConversion && !supported.videoEncodeRgbConversion) {
                    ss << "VkPhysicalDeviceVideoEncodeRgbConversionFeaturesVALVE::videoEncodeRgbConversion is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR: {
                VkPhysicalDeviceVideoMaintenance1FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVideoMaintenance1FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVideoMaintenance1FeaturesKHR *>(current);
                if (enabling->videoMaintenance1 && !supported.videoMaintenance1) {
                    ss << "VkPhysicalDeviceVideoMaintenance1FeaturesKHR::videoMaintenance1 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR: {
                VkPhysicalDeviceVideoMaintenance2FeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVideoMaintenance2FeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVideoMaintenance2FeaturesKHR *>(current);
                if (enabling->videoMaintenance2 && !supported.videoMaintenance2) {
                    ss << "VkPhysicalDeviceVideoMaintenance2FeaturesKHR::videoMaintenance2 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES: {
                VkPhysicalDeviceVulkan11Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVulkan11Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVulkan11Features *>(current);
                if (enabling->storageBuffer16BitAccess && !supported.storageBuffer16BitAccess) {
                    ss << "VkPhysicalDeviceVulkan11Features::storageBuffer16BitAccess is not supported\n";
                }
                if (enabling->uniformAndStorageBuffer16BitAccess && !supported.uniformAndStorageBuffer16BitAccess) {
                    ss << "VkPhysicalDeviceVulkan11Features::uniformAndStorageBuffer16BitAccess is not supported\n";
                }
                if (enabling->storagePushConstant16 && !supported.storagePushConstant16) {
                    ss << "VkPhysicalDeviceVulkan11Features::storagePushConstant16 is not supported\n";
                }
                if (enabling->storageInputOutput16 && !supported.storageInputOutput16) {
                    ss << "VkPhysicalDeviceVulkan11Features::storageInputOutput16 is not supported\n";
                }
                if (enabling->multiview && !supported.multiview) {
                    ss << "VkPhysicalDeviceVulkan11Features::multiview is not supported\n";
                }
                if (enabling->multiviewGeometryShader && !supported.multiviewGeometryShader) {
                    ss << "VkPhysicalDeviceVulkan11Features::multiviewGeometryShader is not supported\n";
                }
                if (enabling->multiviewTessellationShader && !supported.multiviewTessellationShader) {
                    ss << "VkPhysicalDeviceVulkan11Features::multiviewTessellationShader is not supported\n";
                }
                if (enabling->variablePointersStorageBuffer && !supported.variablePointersStorageBuffer) {
                    ss << "VkPhysicalDeviceVulkan11Features::variablePointersStorageBuffer is not supported\n";
                }
                if (enabling->variablePointers && !supported.variablePointers) {
                    ss << "VkPhysicalDeviceVulkan11Features::variablePointers is not supported\n";
                }
                if (enabling->protectedMemory && !supported.protectedMemory) {
                    ss << "VkPhysicalDeviceVulkan11Features::protectedMemory is not supported\n";
                }
                if (enabling->samplerYcbcrConversion && !supported.samplerYcbcrConversion) {
                    ss << "VkPhysicalDeviceVulkan11Features::samplerYcbcrConversion is not supported\n";
                }
                if (enabling->shaderDrawParameters && !supported.shaderDrawParameters) {
                    ss << "VkPhysicalDeviceVulkan11Features::shaderDrawParameters is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES: {
                VkPhysicalDeviceVulkan12Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVulkan12Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVulkan12Features *>(current);
                if (enabling->samplerMirrorClampToEdge && !supported.samplerMirrorClampToEdge) {
                    ss << "VkPhysicalDeviceVulkan12Features::samplerMirrorClampToEdge is not supported\n";
                }
                if (enabling->drawIndirectCount && !supported.drawIndirectCount) {
                    ss << "VkPhysicalDeviceVulkan12Features::drawIndirectCount is not supported\n";
                }
                if (enabling->storageBuffer8BitAccess && !supported.storageBuffer8BitAccess) {
                    ss << "VkPhysicalDeviceVulkan12Features::storageBuffer8BitAccess is not supported\n";
                }
                if (enabling->uniformAndStorageBuffer8BitAccess && !supported.uniformAndStorageBuffer8BitAccess) {
                    ss << "VkPhysicalDeviceVulkan12Features::uniformAndStorageBuffer8BitAccess is not supported\n";
                }
                if (enabling->storagePushConstant8 && !supported.storagePushConstant8) {
                    ss << "VkPhysicalDeviceVulkan12Features::storagePushConstant8 is not supported\n";
                }
                if (enabling->shaderBufferInt64Atomics && !supported.shaderBufferInt64Atomics) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderBufferInt64Atomics is not supported\n";
                }
                if (enabling->shaderSharedInt64Atomics && !supported.shaderSharedInt64Atomics) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderSharedInt64Atomics is not supported\n";
                }
                if (enabling->shaderFloat16 && !supported.shaderFloat16) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderFloat16 is not supported\n";
                }
                if (enabling->shaderInt8 && !supported.shaderInt8) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderInt8 is not supported\n";
                }
                if (enabling->descriptorIndexing && !supported.descriptorIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorIndexing is not supported\n";
                }
                if (enabling->shaderInputAttachmentArrayDynamicIndexing && !supported.shaderInputAttachmentArrayDynamicIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayDynamicIndexing is not supported\n";
                }
                if (enabling->shaderUniformTexelBufferArrayDynamicIndexing &&
                    !supported.shaderUniformTexelBufferArrayDynamicIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayDynamicIndexing is not supported\n";
                }
                if (enabling->shaderStorageTexelBufferArrayDynamicIndexing &&
                    !supported.shaderStorageTexelBufferArrayDynamicIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayDynamicIndexing is not supported\n";
                }
                if (enabling->shaderUniformBufferArrayNonUniformIndexing && !supported.shaderUniformBufferArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderUniformBufferArrayNonUniformIndexing is not supported\n";
                }
                if (enabling->shaderSampledImageArrayNonUniformIndexing && !supported.shaderSampledImageArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderSampledImageArrayNonUniformIndexing is not supported\n";
                }
                if (enabling->shaderStorageBufferArrayNonUniformIndexing && !supported.shaderStorageBufferArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderStorageBufferArrayNonUniformIndexing is not supported\n";
                }
                if (enabling->shaderStorageImageArrayNonUniformIndexing && !supported.shaderStorageImageArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderStorageImageArrayNonUniformIndexing is not supported\n";
                }
                if (enabling->shaderInputAttachmentArrayNonUniformIndexing &&
                    !supported.shaderInputAttachmentArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayNonUniformIndexing is not supported\n";
                }
                if (enabling->shaderUniformTexelBufferArrayNonUniformIndexing &&
                    !supported.shaderUniformTexelBufferArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayNonUniformIndexing is not supported\n";
                }
                if (enabling->shaderStorageTexelBufferArrayNonUniformIndexing &&
                    !supported.shaderStorageTexelBufferArrayNonUniformIndexing) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayNonUniformIndexing is not supported\n";
                }
                if (enabling->descriptorBindingUniformBufferUpdateAfterBind &&
                    !supported.descriptorBindingUniformBufferUpdateAfterBind) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorBindingUniformBufferUpdateAfterBind is not supported\n";
                }
                if (enabling->descriptorBindingSampledImageUpdateAfterBind &&
                    !supported.descriptorBindingSampledImageUpdateAfterBind) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorBindingSampledImageUpdateAfterBind is not supported\n";
                }
                if (enabling->descriptorBindingStorageImageUpdateAfterBind &&
                    !supported.descriptorBindingStorageImageUpdateAfterBind) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorBindingStorageImageUpdateAfterBind is not supported\n";
                }
                if (enabling->descriptorBindingStorageBufferUpdateAfterBind &&
                    !supported.descriptorBindingStorageBufferUpdateAfterBind) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorBindingStorageBufferUpdateAfterBind is not supported\n";
                }
                if (enabling->descriptorBindingUniformTexelBufferUpdateAfterBind &&
                    !supported.descriptorBindingUniformTexelBufferUpdateAfterBind) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorBindingUniformTexelBufferUpdateAfterBind is not supported\n";
                }
                if (enabling->descriptorBindingStorageTexelBufferUpdateAfterBind &&
                    !supported.descriptorBindingStorageTexelBufferUpdateAfterBind) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorBindingStorageTexelBufferUpdateAfterBind is not supported\n";
                }
                if (enabling->descriptorBindingUpdateUnusedWhilePending && !supported.descriptorBindingUpdateUnusedWhilePending) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorBindingUpdateUnusedWhilePending is not supported\n";
                }
                if (enabling->descriptorBindingPartiallyBound && !supported.descriptorBindingPartiallyBound) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorBindingPartiallyBound is not supported\n";
                }
                if (enabling->descriptorBindingVariableDescriptorCount && !supported.descriptorBindingVariableDescriptorCount) {
                    ss << "VkPhysicalDeviceVulkan12Features::descriptorBindingVariableDescriptorCount is not supported\n";
                }
                if (enabling->runtimeDescriptorArray && !supported.runtimeDescriptorArray) {
                    ss << "VkPhysicalDeviceVulkan12Features::runtimeDescriptorArray is not supported\n";
                }
                if (enabling->samplerFilterMinmax && !supported.samplerFilterMinmax) {
                    ss << "VkPhysicalDeviceVulkan12Features::samplerFilterMinmax is not supported\n";
                }
                if (enabling->scalarBlockLayout && !supported.scalarBlockLayout) {
                    ss << "VkPhysicalDeviceVulkan12Features::scalarBlockLayout is not supported\n";
                }
                if (enabling->imagelessFramebuffer && !supported.imagelessFramebuffer) {
                    ss << "VkPhysicalDeviceVulkan12Features::imagelessFramebuffer is not supported\n";
                }
                if (enabling->uniformBufferStandardLayout && !supported.uniformBufferStandardLayout) {
                    ss << "VkPhysicalDeviceVulkan12Features::uniformBufferStandardLayout is not supported\n";
                }
                if (enabling->shaderSubgroupExtendedTypes && !supported.shaderSubgroupExtendedTypes) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderSubgroupExtendedTypes is not supported\n";
                }
                if (enabling->separateDepthStencilLayouts && !supported.separateDepthStencilLayouts) {
                    ss << "VkPhysicalDeviceVulkan12Features::separateDepthStencilLayouts is not supported\n";
                }
                if (enabling->hostQueryReset && !supported.hostQueryReset) {
                    ss << "VkPhysicalDeviceVulkan12Features::hostQueryReset is not supported\n";
                }
                if (enabling->timelineSemaphore && !supported.timelineSemaphore) {
                    ss << "VkPhysicalDeviceVulkan12Features::timelineSemaphore is not supported\n";
                }
                if (enabling->bufferDeviceAddress && !supported.bufferDeviceAddress) {
                    ss << "VkPhysicalDeviceVulkan12Features::bufferDeviceAddress is not supported\n";
                }
                if (enabling->bufferDeviceAddressCaptureReplay && !supported.bufferDeviceAddressCaptureReplay) {
                    ss << "VkPhysicalDeviceVulkan12Features::bufferDeviceAddressCaptureReplay is not supported\n";
                }
                if (enabling->bufferDeviceAddressMultiDevice && !supported.bufferDeviceAddressMultiDevice) {
                    ss << "VkPhysicalDeviceVulkan12Features::bufferDeviceAddressMultiDevice is not supported\n";
                }
                if (enabling->vulkanMemoryModel && !supported.vulkanMemoryModel) {
                    ss << "VkPhysicalDeviceVulkan12Features::vulkanMemoryModel is not supported\n";
                }
                if (enabling->vulkanMemoryModelDeviceScope && !supported.vulkanMemoryModelDeviceScope) {
                    ss << "VkPhysicalDeviceVulkan12Features::vulkanMemoryModelDeviceScope is not supported\n";
                }
                if (enabling->vulkanMemoryModelAvailabilityVisibilityChains &&
                    !supported.vulkanMemoryModelAvailabilityVisibilityChains) {
                    ss << "VkPhysicalDeviceVulkan12Features::vulkanMemoryModelAvailabilityVisibilityChains is not supported\n";
                }
                if (enabling->shaderOutputViewportIndex && !supported.shaderOutputViewportIndex) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderOutputViewportIndex is not supported\n";
                }
                if (enabling->shaderOutputLayer && !supported.shaderOutputLayer) {
                    ss << "VkPhysicalDeviceVulkan12Features::shaderOutputLayer is not supported\n";
                }
                if (enabling->subgroupBroadcastDynamicId && !supported.subgroupBroadcastDynamicId) {
                    ss << "VkPhysicalDeviceVulkan12Features::subgroupBroadcastDynamicId is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES: {
                VkPhysicalDeviceVulkan13Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVulkan13Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVulkan13Features *>(current);
                if (enabling->robustImageAccess && !supported.robustImageAccess) {
                    ss << "VkPhysicalDeviceVulkan13Features::robustImageAccess is not supported\n";
                }
                if (enabling->inlineUniformBlock && !supported.inlineUniformBlock) {
                    ss << "VkPhysicalDeviceVulkan13Features::inlineUniformBlock is not supported\n";
                }
                if (enabling->descriptorBindingInlineUniformBlockUpdateAfterBind &&
                    !supported.descriptorBindingInlineUniformBlockUpdateAfterBind) {
                    ss << "VkPhysicalDeviceVulkan13Features::descriptorBindingInlineUniformBlockUpdateAfterBind is not supported\n";
                }
                if (enabling->pipelineCreationCacheControl && !supported.pipelineCreationCacheControl) {
                    ss << "VkPhysicalDeviceVulkan13Features::pipelineCreationCacheControl is not supported\n";
                }
                if (enabling->privateData && !supported.privateData) {
                    ss << "VkPhysicalDeviceVulkan13Features::privateData is not supported\n";
                }
                if (enabling->shaderDemoteToHelperInvocation && !supported.shaderDemoteToHelperInvocation) {
                    ss << "VkPhysicalDeviceVulkan13Features::shaderDemoteToHelperInvocation is not supported\n";
                }
                if (enabling->shaderTerminateInvocation && !supported.shaderTerminateInvocation) {
                    ss << "VkPhysicalDeviceVulkan13Features::shaderTerminateInvocation is not supported\n";
                }
                if (enabling->subgroupSizeControl && !supported.subgroupSizeControl) {
                    ss << "VkPhysicalDeviceVulkan13Features::subgroupSizeControl is not supported\n";
                }
                if (enabling->computeFullSubgroups && !supported.computeFullSubgroups) {
                    ss << "VkPhysicalDeviceVulkan13Features::computeFullSubgroups is not supported\n";
                }
                if (enabling->synchronization2 && !supported.synchronization2) {
                    ss << "VkPhysicalDeviceVulkan13Features::synchronization2 is not supported\n";
                }
                if (enabling->textureCompressionASTC_HDR && !supported.textureCompressionASTC_HDR) {
                    ss << "VkPhysicalDeviceVulkan13Features::textureCompressionASTC_HDR is not supported\n";
                }
                if (enabling->shaderZeroInitializeWorkgroupMemory && !supported.shaderZeroInitializeWorkgroupMemory) {
                    ss << "VkPhysicalDeviceVulkan13Features::shaderZeroInitializeWorkgroupMemory is not supported\n";
                }
                if (enabling->dynamicRendering && !supported.dynamicRendering) {
                    ss << "VkPhysicalDeviceVulkan13Features::dynamicRendering is not supported\n";
                }
                if (enabling->shaderIntegerDotProduct && !supported.shaderIntegerDotProduct) {
                    ss << "VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct is not supported\n";
                }
                if (enabling->maintenance4 && !supported.maintenance4) {
                    ss << "VkPhysicalDeviceVulkan13Features::maintenance4 is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES: {
                VkPhysicalDeviceVulkan14Features supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVulkan14Features *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVulkan14Features *>(current);
                if (enabling->globalPriorityQuery && !supported.globalPriorityQuery) {
                    ss << "VkPhysicalDeviceVulkan14Features::globalPriorityQuery is not supported\n";
                }
                if (enabling->shaderSubgroupRotate && !supported.shaderSubgroupRotate) {
                    ss << "VkPhysicalDeviceVulkan14Features::shaderSubgroupRotate is not supported\n";
                }
                if (enabling->shaderSubgroupRotateClustered && !supported.shaderSubgroupRotateClustered) {
                    ss << "VkPhysicalDeviceVulkan14Features::shaderSubgroupRotateClustered is not supported\n";
                }
                if (enabling->shaderFloatControls2 && !supported.shaderFloatControls2) {
                    ss << "VkPhysicalDeviceVulkan14Features::shaderFloatControls2 is not supported\n";
                }
                if (enabling->shaderExpectAssume && !supported.shaderExpectAssume) {
                    ss << "VkPhysicalDeviceVulkan14Features::shaderExpectAssume is not supported\n";
                }
                if (enabling->rectangularLines && !supported.rectangularLines) {
                    ss << "VkPhysicalDeviceVulkan14Features::rectangularLines is not supported\n";
                }
                if (enabling->bresenhamLines && !supported.bresenhamLines) {
                    ss << "VkPhysicalDeviceVulkan14Features::bresenhamLines is not supported\n";
                }
                if (enabling->smoothLines && !supported.smoothLines) {
                    ss << "VkPhysicalDeviceVulkan14Features::smoothLines is not supported\n";
                }
                if (enabling->stippledRectangularLines && !supported.stippledRectangularLines) {
                    ss << "VkPhysicalDeviceVulkan14Features::stippledRectangularLines is not supported\n";
                }
                if (enabling->stippledBresenhamLines && !supported.stippledBresenhamLines) {
                    ss << "VkPhysicalDeviceVulkan14Features::stippledBresenhamLines is not supported\n";
                }
                if (enabling->stippledSmoothLines && !supported.stippledSmoothLines) {
                    ss << "VkPhysicalDeviceVulkan14Features::stippledSmoothLines is not supported\n";
                }
                if (enabling->vertexAttributeInstanceRateDivisor && !supported.vertexAttributeInstanceRateDivisor) {
                    ss << "VkPhysicalDeviceVulkan14Features::vertexAttributeInstanceRateDivisor is not supported\n";
                }
                if (enabling->vertexAttributeInstanceRateZeroDivisor && !supported.vertexAttributeInstanceRateZeroDivisor) {
                    ss << "VkPhysicalDeviceVulkan14Features::vertexAttributeInstanceRateZeroDivisor is not supported\n";
                }
                if (enabling->indexTypeUint8 && !supported.indexTypeUint8) {
                    ss << "VkPhysicalDeviceVulkan14Features::indexTypeUint8 is not supported\n";
                }
                if (enabling->dynamicRenderingLocalRead && !supported.dynamicRenderingLocalRead) {
                    ss << "VkPhysicalDeviceVulkan14Features::dynamicRenderingLocalRead is not supported\n";
                }
                if (enabling->maintenance5 && !supported.maintenance5) {
                    ss << "VkPhysicalDeviceVulkan14Features::maintenance5 is not supported\n";
                }
                if (enabling->maintenance6 && !supported.maintenance6) {
                    ss << "VkPhysicalDeviceVulkan14Features::maintenance6 is not supported\n";
                }
                if (enabling->pipelineProtectedAccess && !supported.pipelineProtectedAccess) {
                    ss << "VkPhysicalDeviceVulkan14Features::pipelineProtectedAccess is not supported\n";
                }
                if (enabling->pipelineRobustness && !supported.pipelineRobustness) {
                    ss << "VkPhysicalDeviceVulkan14Features::pipelineRobustness is not supported\n";
                }
                if (enabling->hostImageCopy && !supported.hostImageCopy) {
                    ss << "VkPhysicalDeviceVulkan14Features::hostImageCopy is not supported\n";
                }
                if (enabling->pushDescriptor && !supported.pushDescriptor) {
                    ss << "VkPhysicalDeviceVulkan14Features::pushDescriptor is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES: {
                VkPhysicalDeviceVulkanMemoryModelFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceVulkanMemoryModelFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeatures *>(current);
                if (enabling->vulkanMemoryModel && !supported.vulkanMemoryModel) {
                    ss << "VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModel is not supported\n";
                }
                if (enabling->vulkanMemoryModelDeviceScope && !supported.vulkanMemoryModelDeviceScope) {
                    ss << "VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModelDeviceScope is not supported\n";
                }
                if (enabling->vulkanMemoryModelAvailabilityVisibilityChains &&
                    !supported.vulkanMemoryModelAvailabilityVisibilityChains) {
                    ss << "VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModelAvailabilityVisibilityChains is not "
                          "supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR: {
                VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *enabling =
                    reinterpret_cast<const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(current);
                if (enabling->workgroupMemoryExplicitLayout && !supported.workgroupMemoryExplicitLayout) {
                    ss << "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout is not "
                          "supported\n";
                }
                if (enabling->workgroupMemoryExplicitLayoutScalarBlockLayout &&
                    !supported.workgroupMemoryExplicitLayoutScalarBlockLayout) {
                    ss << "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::"
                          "workgroupMemoryExplicitLayoutScalarBlockLayout is not supported\n";
                }
                if (enabling->workgroupMemoryExplicitLayout8BitAccess && !supported.workgroupMemoryExplicitLayout8BitAccess) {
                    ss << "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout8BitAccess is "
                          "not supported\n";
                }
                if (enabling->workgroupMemoryExplicitLayout16BitAccess && !supported.workgroupMemoryExplicitLayout16BitAccess) {
                    ss << "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout16BitAccess is "
                          "not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT: {
                VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(current);
                if (enabling->ycbcr2plane444Formats && !supported.ycbcr2plane444Formats) {
                    ss << "VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT::ycbcr2plane444Formats is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM: {
                VkPhysicalDeviceYcbcrDegammaFeaturesQCOM supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *enabling =
                    reinterpret_cast<const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *>(current);
                if (enabling->ycbcrDegamma && !supported.ycbcrDegamma) {
                    ss << "VkPhysicalDeviceYcbcrDegammaFeaturesQCOM::ycbcrDegamma is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT: {
                VkPhysicalDeviceYcbcrImageArraysFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(current);
                if (enabling->ycbcrImageArrays && !supported.ycbcrImageArrays) {
                    ss << "VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::ycbcrImageArrays is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_DEVICE_MEMORY_FEATURES_EXT: {
                VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT *enabling =
                    reinterpret_cast<const VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT *>(current);
                if (enabling->zeroInitializeDeviceMemory && !supported.zeroInitializeDeviceMemory) {
                    ss << "VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT::zeroInitializeDeviceMemory is not supported\n";
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES: {
                VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures supported = vku::InitStructHelper();
                features_2.pNext = &supported;
                DispatchGetPhysicalDeviceFeatures2(gpu, &features_2);
                const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *enabling =
                    reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(current);
                if (enabling->shaderZeroInitializeWorkgroupMemory && !supported.shaderZeroInitializeWorkgroupMemory) {
                    ss << "VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures::shaderZeroInitializeWorkgroupMemory is not "
                          "supported\n";
                }
                break;
            }

            default:
                break;
        }
    }

    Location loc(vvl::Func::vkCreateDevice);
    LogWarning("WARNING-vkCreateDevice-FeatureNotPresent", instance, loc, "%s", ss.str().c_str());
}  // ReportErrorFeatureNotPresent
}  // namespace dispatch
}  // namespace vvl
