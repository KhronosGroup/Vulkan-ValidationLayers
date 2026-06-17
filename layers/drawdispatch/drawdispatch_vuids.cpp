/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
 * Copyright (c) 2025 Arm Limited.
 * Modifications Copyright (C) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
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

#include "drawdispatch_vuids.h"
#include "error_message/logging.h"
#include "generated/error_location_helper.h"

namespace vvl {

using Func = vvl::Func;

// The reason we keep strings of ALL the VUIDs is to make it easier to search and know what is covered or not.
// The reality is the "action" commands (draw/dispatch/traceRays) VUID list has got stupidly large with 30+ ways to invoke work,
// each having over 250 VUs, large enough that we are breaking stack limits from how many strings we have!!
// (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/11779)
//
// ... So this is "THE EXCEPTION, NOT THE NORMAL" here that we just concatenate the VUID from a number/enum
// The scripts/vk_validation_stats.py is adjusted to handle this.
//
std::string CreateActionVuid(Func function, const ActionVUID id) {
    const char* suffix = nullptr;

    // clang-format off
    switch (id) {
        case ActionVUID::UNKNOWN: suffix = "UNKNOWN"; break;

        // TODO - This one is a lie, see ValidateCmdBufImageLayouts()
        // ### VUID-vkCmdDraw-None-09600
        case ActionVUID::IMAGE_LAYOUT_09600: suffix = "None-09600"; break;

        // We use the three # to reference one real example, for the scripts/vk_validation_stats.py to parse
        //
        // ### VUID-vkCmdDraw-None-04007
        case ActionVUID::VERTEX_BINDING_04007: suffix = "None-04007"; break;
        // ### VUID-vkCmdDraw-None-04008
        case ActionVUID::VERTEX_BINDING_04008: suffix = "None-04008"; break;

        // ### VUID-vkCmdDraw-subpass-02685
        case ActionVUID::SUBPASS_INDEX_02685: suffix = "subpass-02685"; break;

        // ### VUID-vkCmdDraw-sampleLocationsEnable-02689
        case ActionVUID::SAMPLE_LOCATION_02689: suffix = "sampleLocationsEnable-02689"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07484
        case ActionVUID::SAMPLE_LOCATION_07484: suffix = "sampleLocationsEnable-07484"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07485
        case ActionVUID::SAMPLE_LOCATION_07485: suffix = "sampleLocationsEnable-07485"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07486
        case ActionVUID::SAMPLE_LOCATION_07486: suffix = "sampleLocationsEnable-07486"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07487
        case ActionVUID::SAMPLE_LOCATION_07487: suffix = "sampleLocationsEnable-07487"; break;

        // ### VUID-vkCmdDraw-None-09211
        case ActionVUID::RASTERIZATION_SAMPLES_09211: suffix = "None-09211"; break;

        // ### VUID-vkCmdDraw-magFilter-04553
        case ActionVUID::LINEAR_FILTER_04553: suffix = "magFilter-04553"; break;
        // ### VUID-vkCmdDraw-magFilter-09598
        case ActionVUID::LINEAR_FILTER_09598: suffix = "magFilter-09598"; break;
        // ### VUID-vkCmdDraw-mipmapMode-04770
        case ActionVUID::LINEAR_MIPMAP_04770: suffix = "mipmapMode-04770"; break;
        // ### VUID-vkCmdDraw-mipmapMode-09599
        case ActionVUID::LINEAR_MIPMAP_09599: suffix = "mipmapMode-09599"; break;

        // ### VUID-vkCmdDraw-colorAttachmentCount-09362
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09362: suffix = "colorAttachmentCount-09362"; break;
        // ### VUID-vkCmdDraw-None-09363
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09363: suffix = "None-09363"; break;
        // ### VUID-vkCmdDraw-None-09364
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09364: suffix = "None-09364"; break;
        // ### VUID-vkCmdDraw-None-09365
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09365: suffix = "None-09365"; break;
        // ### VUID-vkCmdDraw-None-09368
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09368: suffix = "None-09368"; break;
        // ### VUID-vkCmdDraw-None-09369
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09369: suffix = "None-09369"; break;
        // ### VUID-vkCmdDraw-colorAttachmentCount-09372
        case ActionVUID::EXTERNAL_FORMAT_RESOLVE_09372: suffix = "colorAttachmentCount-09372"; break;

        // ### VUID-vkCmdDraw-flags-11521
        case ActionVUID::CUSTOM_RESOLVE_11521: suffix = "flags-11521"; break;
        // ### VUID-vkCmdDraw-None-11522
        case ActionVUID::CUSTOM_RESOLVE_11522: suffix = "None-11522"; break;
        // ### VUID-vkCmdDraw-None-11523
        case ActionVUID::CUSTOM_RESOLVE_11523: suffix = "None-11523"; break;
        // ### VUID-vkCmdDraw-customResolve-11524
        case ActionVUID::CUSTOM_RESOLVE_11524: suffix = "customResolve-11524"; break;
        // ### VUID-vkCmdDraw-customResolve-11525
        case ActionVUID::CUSTOM_RESOLVE_11525: suffix = "customResolve-11525"; break;
        // ### VUID-vkCmdDraw-customResolve-11529
        case ActionVUID::CUSTOM_RESOLVE_11529: suffix = "customResolve-11529"; break;
        // ### VUID-vkCmdDraw-customResolve-11530
        case ActionVUID::CUSTOM_RESOLVE_11530: suffix = "customResolve-11530"; break;
        // ### VUID-vkCmdDraw-pColorAttachments-11539
        case ActionVUID::CUSTOM_RESOLVE_11539: suffix = "pColorAttachments-11539"; break;
        // ### VUID-vkCmdDraw-pDepthAttachment-11540
        case ActionVUID::CUSTOM_RESOLVE_11540: suffix = "pDepthAttachment-11540"; break;
        // ### VUID-vkCmdDraw-pStencilAttachment-11860
        case ActionVUID::CUSTOM_RESOLVE_11860: suffix = "pStencilAttachment-11860"; break;
        // ### VUID-vkCmdDraw-None-11861
        case ActionVUID::CUSTOM_RESOLVE_11861: suffix = "None-11861"; break;
        // ### VUID-vkCmdDraw-None-11862
        case ActionVUID::CUSTOM_RESOLVE_11862: suffix = "None-11862"; break;
        // ### VUID-vkCmdDraw-None-11863
        case ActionVUID::CUSTOM_RESOLVE_11863: suffix = "None-11863"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-11864
        case ActionVUID::CUSTOM_RESOLVE_11864: suffix = "dynamicRenderingUnusedAttachments-11864"; break;
        // ### VUID-vkCmdDraw-None-11865
        case ActionVUID::CUSTOM_RESOLVE_11865: suffix = "None-11865"; break;
        // ### VUID-vkCmdDraw-None-11866
        case ActionVUID::CUSTOM_RESOLVE_11866: suffix = "None-11866"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-11867
        case ActionVUID::CUSTOM_RESOLVE_11867: suffix = "dynamicRenderingUnusedAttachments-11867"; break;
        // ### VUID-vkCmdDraw-None-11868
        case ActionVUID::CUSTOM_RESOLVE_11868: suffix = "None-11868"; break;
        // ### VUID-vkCmdDraw-None-11869
        case ActionVUID::CUSTOM_RESOLVE_11869: suffix = "None-11869"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-11870
        case ActionVUID::CUSTOM_RESOLVE_11870: suffix = "dynamicRenderingUnusedAttachments-11870"; break;

        // ### VUID-vkCmdDraw-stippledLineEnable-07495
        case ActionVUID::STIPPLED_RECTANGULAR_07495: suffix = "stippledLineEnable-07495"; break;
        // ### VUID-vkCmdDraw-stippledLineEnable-07496
        case ActionVUID::STIPPLED_BRESENHAM_07496: suffix = "stippledLineEnable-07496"; break;
        // ### VUID-vkCmdDraw-stippledLineEnable-07497
        case ActionVUID::STIPPLED_SMOOTH_07497: suffix = "stippledLineEnable-07497"; break;
        // ### VUID-vkCmdDraw-stippledLineEnable-07498
        case ActionVUID::STIPPLED_STRICT_07498: suffix = "stippledLineEnable-07498"; break;

        // ### VUID-vkCmdDraw-SampledType-04470
        case ActionVUID::IMAGE_VIEW_ACCESS_04470: suffix = "SampledType-04470"; break;
        // ### VUID-vkCmdDraw-SampledType-04471
        case ActionVUID::IMAGE_VIEW_ACCESS_04471: suffix = "SampledType-04471"; break;
        // ### VUID-vkCmdDraw-sparseImageInt64Atomics-04474
        case ActionVUID::IMAGE_VIEW_SPARSE_04474: suffix = "sparseImageInt64Atomics-04474"; break;
        // ### VUID-vkCmdDraw-SampledType-04472
        case ActionVUID::BUFFER_VIEW_ACCESS_04472: suffix = "SampledType-04472"; break;
        // ### VUID-vkCmdDraw-SampledType-04473
        case ActionVUID::BUFFER_VIEW_ACCESS_04473: suffix = "SampledType-04473"; break;
        // ### VUID-vkCmdDraw-OpTypeImage-07028
        case ActionVUID::STORAGE_IMAGE_FORMAT_07028: suffix = "OpTypeImage-07028"; break;
        // ### VUID-vkCmdDraw-OpTypeImage-07027
        case ActionVUID::STORAGE_IMAGE_FORMAT_07027: suffix = "OpTypeImage-07027"; break;
        // ### VUID-vkCmdDraw-OpTypeImage-07030
        case ActionVUID::STORAGE_TEXEL_FORMAT_07030: suffix = "OpTypeImage-07030"; break;
        // ### VUID-vkCmdDraw-OpTypeImage-07029
        case ActionVUID::STORAGE_TEXEL_FORMAT_07029: suffix = "OpTypeImage-07029"; break;
        // ### VUID-vkCmdDraw-OpImageWrite-08795
        case ActionVUID::STORAGE_IMAGE_TEXEL_08795: suffix = "OpImageWrite-08795"; break;
        // ### VUID-vkCmdDraw-OpImageWrite-08796
        case ActionVUID::STORAGE_IMAGE_TEXEL_08796: suffix = "OpImageWrite-08796"; break;
        // ### VUID-vkCmdDraw-OpImageWrite-04469
        case ActionVUID::STORAGE_TEXEL_04469: suffix = "OpImageWrite-04469"; break;
        // ### VUID-vkCmdDraw-None-02691
        case ActionVUID::IMAGE_VIEW_ATOMIC_02691: suffix = "None-02691"; break;
        // ### VUID-vkCmdDraw-None-07888
        case ActionVUID::BUFFER_VIEW_ATOMIC_07888: suffix = "None-07888"; break;
        // ### VUID-vkCmdDraw-viewType-07752
        case ActionVUID::IMAGE_VIEW_DIM_07752: suffix = "viewType-07752"; break;
        // ### VUID-vkCmdDraw-format-07753
        case ActionVUID::IMAGE_VIEW_NUMERIC_07753: suffix = "format-07753"; break;

        // ### VUID-vkCmdDraw-None-06537
        case ActionVUID::SUBRESOURCE_RP_WRTIE_06537: suffix = "None-06537"; break;
        // ### VUID-vkCmdDraw-None-12338
        case ActionVUID::SUBRESOURCE_SUBPASS_12338: suffix = "None-12338"; break;
        // ### VUID-vkCmdDraw-None-12339
        case ActionVUID::SUBRESOURCE_SUBPASS_12339: suffix = "None-12339"; break;
        // ### VUID-vkCmdDraw-None-12340
        case ActionVUID::SUBRESOURCE_SUBPASS_12340: suffix = "None-12340"; break;

        // ### VUID-vkCmdDraw-None-11308
        case ActionVUID::DESCRIPTOR_HEAP_11308: suffix = "None-11308"; break;
        // ### VUID-vkCmdDraw-pBindInfo-11375
        case ActionVUID::DESCRIPTOR_HEAP_11375: suffix = "pBindInfo-11375"; break;
        // ### VUID-vkCmdDraw-None-11376
        case ActionVUID::DESCRIPTOR_HEAP_11376: suffix = "None-11376"; break;

        // ### VUID-vkCmdDraw-None-08609
        case ActionVUID::SAMPLER_TYPE_08609: suffix = "None-08609"; break;
        // ### VUID-vkCmdDraw-None-08610
        case ActionVUID::SAMPLER_DREF_PROJ_08610: suffix = "None-08610"; break;
        // ### VUID-vkCmdDraw-None-08611
        case ActionVUID::SAMPLER_BIAS_OFFSET_08611: suffix = "None-08611"; break;
        // ### VUID-vkCmdDraw-None-02692
        case ActionVUID::SAMPLER_CUBIC_02692: suffix = "None-02692"; break;
        // ### VUID-vkCmdDraw-flags-02696
        case ActionVUID::SAMPLER_CORNER_02696: suffix = "flags-02696"; break;

        // ### VUID-vkCmdDraw-None-02693
        case ActionVUID::FILTER_CUBIC_02693: suffix = "None-02693"; break;
        // ### VUID-vkCmdDraw-filterCubic-02694
        case ActionVUID::FILTER_CUBIC_02694: suffix = "filterCubic-02694"; break;
        // ### VUID-vkCmdDraw-filterCubicMinmax-02695
        case ActionVUID::FILTER_CUBIC_02695: suffix = "filterCubicMinmax-02695"; break;

        // ### VUID-vkCmdDraw-imageLayout-00344
        case ActionVUID::IMAGE_LAYOUT_00344: suffix = "imageLayout-00344"; break;

        // ### VUID-vkCmdDraw-None-08601
        case ActionVUID::PUSH_CONSTANT_08601: suffix = "None-08601"; break;
        // ### VUID-vkCmdDraw-flags-13361
        case ActionVUID::INDEPENDENT_SETS_13361: suffix = "flags-13361"; break;
        // ### VUID-vkCmdDraw-flags-13362
        case ActionVUID::INDEPENDENT_SETS_13362: suffix = "flags-13362"; break;
        // ### VUID-vkCmdDraw-flags-13363
        case ActionVUID::INDEPENDENT_SETS_13363: suffix = "flags-13363"; break;
        // ### VUID-vkCmdDraw-flags-13364
        case ActionVUID::INDEPENDENT_SETS_13364: suffix = "flags-13364"; break;
        // ### VUID-vkCmdDraw-flags-13365
        case ActionVUID::INDEPENDENT_SETS_13365: suffix = "flags-13365"; break;

        // ### VUID-vkCmdDraw-None-08608
        case ActionVUID::DYNAMIC_STATE_ALL_SET_08608: suffix = "None-08608"; break;

        // ### VUID-vkCmdDraw-None-07845
        case ActionVUID::DYNAMIC_DEPTH_COMPARE_OP_07845: suffix = "None-07845"; break;
        // ### VUID-vkCmdDraw-None-07834
        case ActionVUID::DYNAMIC_DEPTH_BIAS_07834: suffix = "None-07834"; break;
        // ### VUID-vkCmdDraw-None-07836
        case ActionVUID::DYNAMIC_DEPTH_BOUNDS_07836: suffix = "None-07836"; break;
        // ### VUID-vkCmdDraw-None-07840
        case ActionVUID::DYNAMIC_CULL_MODE_07840: suffix = "None-07840"; break;
        // ### VUID-vkCmdDraw-None-07843
        case ActionVUID::DYNAMIC_DEPTH_TEST_ENABLE_07843: suffix = "None-07843"; break;
        // ### VUID-vkCmdDraw-None-07844
        case ActionVUID::DYNAMIC_DEPTH_WRITE_ENABLE_07844: suffix = "None-07844"; break;
        // ### VUID-vkCmdDraw-None-07847
        case ActionVUID::DYNAMIC_STENCIL_TEST_ENABLE_07847: suffix = "None-07847"; break;
        // ### VUID-vkCmdDraw-None-04877
        case ActionVUID::DYNAMIC_DEPTH_BIAS_ENABLE_04877: suffix = "None-04877"; break;
        // ### VUID-vkCmdDraw-None-07846
        case ActionVUID::DYNAMIC_DEPTH_BOUND_TEST_ENABLE_07846: suffix = "None-07846"; break;
        // ### VUID-vkCmdDraw-None-07837
        case ActionVUID::DYNAMIC_STENCIL_COMPARE_MASK_07837: suffix = "None-07837"; break;
        // ### VUID-vkCmdDraw-None-07838
        case ActionVUID::DYNAMIC_STENCIL_WRITE_MASK_07838: suffix = "None-07838"; break;
        // ### VUID-vkCmdDraw-None-07839
        case ActionVUID::DYNAMIC_STENCIL_REFERENCE_07839: suffix = "None-07839"; break;
        // ### VUID-vkCmdDraw-None-07848
        case ActionVUID::DYNAMIC_STENCIL_OP_07848: suffix = "None-07848"; break;
        // ### VUID-vkCmdDraw-None-07842
        case ActionVUID::DYNAMIC_PRIMITIVE_TOPOLOGY_07842: suffix = "None-07842"; break;
        // ### VUID-vkCmdDraw-None-04879
        case ActionVUID::DYNAMIC_PRIMITIVE_RESTART_ENABLE_04879: suffix = "None-04879"; break;
        // ### VUID-vkCmdDraw-None-04914
        case ActionVUID::DYNAMIC_VERTEX_INPUT_04914: suffix = "None-04914"; break;
        // ### VUID-vkCmdDraw-pipelineFragmentShadingRate-09238
        case ActionVUID::DYNAMIC_SET_FRAGMENT_SHADING_RATE_09238: suffix = "pipelineFragmentShadingRate-09238"; break;
        // ### VUID-vkCmdDraw-logicOp-04878
        case ActionVUID::DYNAMIC_LOGIC_OP_04878: suffix = "logicOp-04878"; break;
        // ### VUID-vkCmdDraw-None-07621
        case ActionVUID::DYNAMIC_POLYGON_MODE_07621: suffix = "None-07621"; break;
        // ### VUID-vkCmdDraw-None-07622
        case ActionVUID::DYNAMIC_RASTERIZATION_SAMPLES_07622: suffix = "None-07622"; break;
        // ### VUID-vkCmdDraw-None-07623
        case ActionVUID::DYNAMIC_SAMPLE_MASK_07623: suffix = "None-07623"; break;
        // ### VUID-vkCmdDraw-None-07624
        case ActionVUID::DYNAMIC_ALPHA_TO_COVERAGE_ENABLE_07624: suffix = "None-07624"; break;
        // ### VUID-vkCmdDraw-None-07625
        case ActionVUID::DYNAMIC_ALPHA_TO_ONE_ENABLE_07625: suffix = "None-07625"; break;
        // ### VUID-vkCmdDraw-None-07626
        case ActionVUID::DYNAMIC_LOGIC_OP_ENABLE_07626: suffix = "None-07626"; break;
        // ### VUID-vkCmdDraw-None-04876
        case ActionVUID::DYNAMIC_RASTERIZER_DISCARD_ENABLE_04876: suffix = "None-04876"; break;
        // ### VUID-vkCmdDraw-None-07634
        case ActionVUID::DYNAMIC_SAMPLE_LOCATIONS_ENABLE_07634: suffix = "None-07634"; break;
        // ### VUID-vkCmdDraw-None-06666
        case ActionVUID::DYNAMIC_SAMPLE_LOCATIONS_06666: suffix = "None-06666"; break;
        // ### VUID-vkCmdDraw-None-08877
        case ActionVUID::DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_08877: suffix = "None-08877"; break;
        // ### VUID-vkCmdDraw-None-07749
        case ActionVUID::DYNAMIC_COLOR_WRITE_ENABLE_07749: suffix = "None-07749"; break;
        // ### VUID-vkCmdDraw-None-07627
        case ActionVUID::DYNAMIC_COLOR_BLEND_ENABLE_07627: suffix = "None-07627"; break;
        // ### VUID-vkCmdDraw-None-07629
        case ActionVUID::DYNAMIC_COLOR_WRITE_MASK_07629: suffix = "None-07629"; break;
        // ### VUID-vkCmdDraw-None-07639
        case ActionVUID::DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_07639: suffix = "None-07639"; break;
        // ### VUID-vkCmdDraw-None-09650
        case ActionVUID::DYNAMIC_DEPTH_CLAMP_CONTROL_09650: suffix = "None-09650"; break;
        // ### VUID-vkCmdDraw-None-07633
        case ActionVUID::DYNAMIC_DEPTH_CLIP_ENABLE_07633: suffix = "None-07633"; break;
        // ### VUID-vkCmdDraw-None-07620
        case ActionVUID::DYNAMIC_DEPTH_CLAMP_ENABLE_07620: suffix = "None-07620"; break;
        // ### VUID-vkCmdDraw-None-07878
        case ActionVUID::DYNAMIC_EXCLUSIVE_SCISSOR_ENABLE_07878: suffix = "None-07878"; break;
        // ### VUID-vkCmdDraw-None-07879
        case ActionVUID::DYNAMIC_EXCLUSIVE_SCISSOR_07879: suffix = "None-07879"; break;
        // ### VUID-vkCmdDraw-None-07619
        case ActionVUID::DYNAMIC_TESSELLATION_DOMAIN_ORIGIN_07619: suffix = "None-07619"; break;
        // ### VUID-vkCmdDraw-None-07630
        case ActionVUID::DYNAMIC_RASTERIZATION_STREAM_07630: suffix = "None-07630"; break;
        // ### VUID-vkCmdDraw-None-07631
        case ActionVUID::DYNAMIC_CONSERVATIVE_RASTERIZATION_MODE_07631: suffix = "None-07631"; break;
        // ### VUID-vkCmdDraw-None-07632
        case ActionVUID::DYNAMIC_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_07632: suffix = "None-07632"; break;
        // ### VUID-vkCmdDraw-None-07636
        case ActionVUID::DYNAMIC_PROVOKING_VERTEX_MODE_07636: suffix = "None-07636"; break;
        // ### VUID-vkCmdDraw-None-07640
        case ActionVUID::DYNAMIC_VIEWPORT_W_SCALING_ENABLE_07640: suffix = "None-07640"; break;
        // ### VUID-vkCmdDraw-None-07641
        case ActionVUID::DYNAMIC_VIEWPORT_SWIZZLE_07641: suffix = "None-07641"; break;
        // ### VUID-vkCmdDraw-None-07642
        case ActionVUID::DYNAMIC_COVERAGE_TO_COLOR_ENABLE_07642: suffix = "None-07642"; break;
        // ### VUID-vkCmdDraw-None-07643
        case ActionVUID::DYNAMIC_COVERAGE_TO_COLOR_LOCATION_07643: suffix = "None-07643"; break;
        // ### VUID-vkCmdDraw-None-07644
        case ActionVUID::DYNAMIC_COVERAGE_MODULATION_MODE_07644: suffix = "None-07644"; break;
        // ### VUID-vkCmdDraw-None-07645
        case ActionVUID::DYNAMIC_COVERAGE_MODULATION_TABLE_ENABLE_07645: suffix = "None-07645"; break;
        // ### VUID-vkCmdDraw-None-07646
        case ActionVUID::DYNAMIC_COVERAGE_MODULATION_TABLE_07646: suffix = "None-07646"; break;
        // ### VUID-vkCmdDraw-None-07647
        case ActionVUID::DYNAMIC_SHADING_RATE_IMAGE_ENABLE_07647: suffix = "None-07647"; break;
        // ### VUID-vkCmdDraw-None-07648
        case ActionVUID::DYNAMIC_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_07648: suffix = "None-07648"; break;
        // ### VUID-vkCmdDraw-None-07649
        case ActionVUID::DYNAMIC_COVERAGE_REDUCTION_MODE_07649: suffix = "None-07649"; break;
        // ### VUID-vkCmdDraw-None-07841
        case ActionVUID::DYNAMIC_FRONT_FACE_07841: suffix = "None-07841"; break;
        // ### VUID-vkCmdDraw-viewportCount-03417
        case ActionVUID::DYNAMIC_VIEWPORT_COUNT_03417: suffix = "viewportCount-03417"; break;
        // ### VUID-vkCmdDraw-scissorCount-03418
        case ActionVUID::DYNAMIC_SCISSOR_COUNT_03418: suffix = "scissorCount-03418"; break;
        // ### VUID-vkCmdDraw-shadingRateImage-09233
        case ActionVUID::DYNAMIC_SET_VIEWPORT_COARSE_SAMPLE_ORDER_09233: suffix = "shadingRateImage-09233"; break;
        // ### VUID-vkCmdDraw-shadingRateImage-09234
        case ActionVUID::DYNAMIC_SET_VIEWPORT_SHADING_RATE_PALETTE_09234: suffix = "shadingRateImage-09234"; break;
        // ### VUID-vkCmdDraw-viewportCount-04138
        case ActionVUID::DYNAMIC_SET_CLIP_SPACE_W_SCALING_04138: suffix = "viewportCount-04138"; break;
        // ### VUID-vkCmdDraw-None-04875
        case ActionVUID::DYNAMIC_PATCH_CONTROL_POINTS_04875: suffix = "None-04875"; break;
        // ### VUID-vkCmdDraw-None-07880
        case ActionVUID::DYNAMIC_DISCARD_RECTANGLE_ENABLE_07880: suffix = "None-07880"; break;
        // ### VUID-vkCmdDraw-None-07881
        case ActionVUID::DYNAMIC_DISCARD_RECTANGLE_MODE_07881: suffix = "None-07881"; break;
        // ### VUID-vkCmdDraw-None-07849
        case ActionVUID::DYNAMIC_LINE_STIPPLE_EXT_07849: suffix = "None-07849"; break;
        // ### VUID-vkCmdDraw-None-08617
        case ActionVUID::DYNAMIC_SET_LINE_WIDTH_08617: suffix = "None-08617"; break;
        // ### VUID-vkCmdDraw-pStrides-04913
        case ActionVUID::DYNAMIC_VERTEX_INPUT_BINDING_STRIDE_04913: suffix = "pStrides-04913"; break;
        // ### VUID-vkCmdDraw-None-08666
        case ActionVUID::DYNAMIC_SET_LINE_RASTERIZATION_MODE_08666: suffix = "None-08666"; break;
        // ### VUID-vkCmdDraw-None-08669
        case ActionVUID::DYNAMIC_SET_LINE_STIPPLE_ENABLE_08669: suffix = "None-08669"; break;

        // ### VUID-vkCmdDraw-None-07751
        case ActionVUID::DISCARD_RECTANGLE_07751: suffix = "None-07751"; break;
        // ### VUID-vkCmdDraw-rasterizerDiscardEnable-09236
        case ActionVUID::DISCARD_RECTANGLE_09236: suffix = "rasterizerDiscardEnable-09236"; break;
        // ### VUID-vkCmdDraw-rasterizerDiscardEnable-09420
        case ActionVUID::COVERAGE_TO_COLOR_09420: suffix = "rasterizerDiscardEnable-09420"; break;
        // ### VUID-vkCmdDraw-coverageToColorEnable-07490
        case ActionVUID::COVERAGE_TO_COLOR_07490: suffix = "coverageToColorEnable-07490"; break;
        // ### VUID-vkCmdDraw-None-08644
        case ActionVUID::SET_RASTERIZATION_SAMPLES_08644: suffix = "None-08644"; break;
        // ### VUID-vkCmdDraw-attachmentCount-07750
        case ActionVUID::COLOR_WRITE_ENABLE_COUNT_07750: suffix = "attachmentCount-07750"; break;
        // ### VUID-vkCmdDraw-None-06479
        case ActionVUID::DEPTH_COMPARE_SAMPLE_06479: suffix = "None-06479"; break;
        // ### VUID-vkCmdDraw-None-06886
        case ActionVUID::DEPTH_READ_ONLY_06886: suffix = "None-06886"; break;
        // ### VUID-vkCmdDraw-None-06887
        case ActionVUID::STENCIL_READ_ONLY_06887: suffix = "None-06887"; break;
        // ### VUID-vkCmdDraw-alphaToCoverageEnable-08919
        case ActionVUID::ALPHA_TO_COVERAGE_COMPONENT_08919: suffix = "alphaToCoverageEnable-08919"; break;
        // ### VUID-vkCmdDraw-firstAttachment-07476
        case ActionVUID::COLOR_BLEND_ENABLE_07476: suffix = "firstAttachment-07476"; break;
        // ### VUID-vkCmdDraw-firstAttachment-07478
        case ActionVUID::COLOR_WRITE_MASK_07478: suffix = "firstAttachment-07478"; break;
        // ### VUID-vkCmdDraw-None-10862
        case ActionVUID::COLOR_BLEND_EQUATION_10862: suffix = "None-10862"; break;
        // ### VUID-vkCmdDraw-rasterizerDiscardEnable-10863
        case ActionVUID::COLOR_BLEND_EQUATION_10863: suffix = "rasterizerDiscardEnable-10863"; break;
        // ### VUID-vkCmdDraw-None-10864
        case ActionVUID::COLOR_BLEND_EQUATION_10864: suffix = "None-10864"; break;
        // ### VUID-vkCmdDraw-None-07831
        case ActionVUID::VIEWPORT_07831: suffix = "None-07831"; break;
        // ### VUID-vkCmdDraw-None-07832
        case ActionVUID::SCISSOR_07832: suffix = "None-07832"; break;
        // ### VUID-vkCmdDraw-None-07835
        case ActionVUID::BLEND_CONSTANTS_07835: suffix = "None-07835"; break;
        // ### VUID-vkCmdDraw-pDynamicStates-08715
        case ActionVUID::DEPTH_ENABLE_08715: suffix = "pDynamicStates-08715"; break;
        // ### VUID-vkCmdDraw-pDynamicStates-08716
        case ActionVUID::STENCIL_WRITE_MASK_08716: suffix = "pDynamicStates-08716"; break;
        // ### VUID-vkCmdDraw-None-07850
        case ActionVUID::STATE_INHERITED_07850: suffix = "None-07850"; break;
        // ### VUID-vkCmdDraw-None-09116
        case ActionVUID::COLOR_WRITE_MASK_09116: suffix = "None-09116"; break;
        // ### VUID-vkCmdDraw-None-10608
        case ActionVUID::LINE_RASTERIZATION_10608: suffix = "None-10608"; break;
        // ### VUID-vkCmdDraw-viewportCount-03419
        case ActionVUID::VIEWPORT_AND_SCISSOR_WITH_COUNT_03419: suffix = "viewportCount-03419"; break;
        // ### VUID-vkCmdDraw-None-08636
        case ActionVUID::VIEWPORT_W_SCALING_08636: suffix = "None-08636"; break;
        // ### VUID-vkCmdDraw-None-08637
        case ActionVUID::SHADING_RATE_PALETTE_08637: suffix = "None-08637"; break;
        // ### VUID-vkCmdDraw-viewportCount-09421
        case ActionVUID::SET_VIEWPORT_SWIZZLE_09421: suffix = "viewportCount-09421"; break;
        // ### VUID-vkCmdDraw-viewportCount-07493
        case ActionVUID::SET_VIEWPORT_SWIZZLE_07493: suffix = "viewportCount-07493"; break;
        // ### VUID-vkCmdDraw-alphaToCoverageEnable-08920
        case ActionVUID::ALPHA_COMPONENT_WORD_08920: suffix = "alphaToCoverageEnable-08920"; break;
        // ### VUID-vkCmdDraw-multiviewPerViewViewports-12262
        case ActionVUID::VIEWPORT_MULTIVIEW_12262: suffix = "multiviewPerViewViewports-12262"; break;
        // ### VUID-vkCmdDraw-multiviewPerViewViewports-12263
        case ActionVUID::SCISSOR_MULTIVIEW_12263: suffix = "multiviewPerViewViewports-12263"; break;
        // ### VUID-vkCmdDraw-conservativePointAndLineRasterization-07499
        case ActionVUID::CONVERVATIVE_RASTERIZATION_07499: suffix = "conservativePointAndLineRasterization-07499"; break;
        // ### VUID-vkCmdDraw-blendEnable-04727
        case ActionVUID::BLEND_ENABLE_04727: suffix = "blendEnable-04727"; break;
        // ### VUID-vkCmdDraw-maxFragmentDualSrcAttachments-09239
        case ActionVUID::BLEND_DUAL_SOURCE_09239: suffix = "maxFragmentDualSrcAttachments-09239"; break;
        // ### VUID-vkCmdDraw-advancedBlendMaxColorAttachments-07480
        case ActionVUID::BLEND_ADVANCED_07480: suffix = "advancedBlendMaxColorAttachments-07480"; break;
        // ### VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-07481
        case ActionVUID::PRIMITIVES_GENERATED_QUERY_07481: suffix = "primitivesGeneratedQueryWithNonZeroStreams-07481"; break;
        // ### VUID-vkCmdDraw-sampleLocationsPerPixel-07482
        case ActionVUID::SAMPLE_LOCATIONS_07482: suffix = "sampleLocationsPerPixel-07482"; break;
        // ### VUID-vkCmdDraw-sampleLocationsPerPixel-07483
        case ActionVUID::SAMPLE_LOCATIONS_07483: suffix = "sampleLocationsPerPixel-07483"; break;
        // ### VUID-vkCmdDraw-rasterizationSamples-07471
        case ActionVUID::SAMPLE_LOCATIONS_07471: suffix = "rasterizationSamples-07471"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07936
        case ActionVUID::SAMPLE_LOCATIONS_ENABLE_07936: suffix = "sampleLocationsEnable-07936"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07937
        case ActionVUID::SAMPLE_LOCATIONS_ENABLE_07937: suffix = "sampleLocationsEnable-07937"; break;
        // ### VUID-vkCmdDraw-sampleLocationsEnable-07938
        case ActionVUID::SAMPLE_LOCATIONS_ENABLE_07938: suffix = "sampleLocationsEnable-07938"; break;
        // ### VUID-vkCmdDraw-samples-07472
        case ActionVUID::SAMPLE_MASK_07472: suffix = "samples-07472"; break;
        // ### VUID-vkCmdDraw-samples-07473
        case ActionVUID::SAMPLE_MASK_07473: suffix = "samples-07473"; break;
        // ### VUID-vkCmdDraw-dynamicPrimitiveTopologyUnrestricted-07500
        case ActionVUID::PRIMITIVE_TOPOLOGY_CLASS_07500: suffix = "dynamicPrimitiveTopologyUnrestricted-07500"; break;
        // ### VUID-vkCmdDraw-primitiveTopology-10286
        case ActionVUID::PRIMITIVE_TOPOLOGY_10286: suffix = "primitiveTopology-10286"; break;
        // ### VUID-vkCmdDraw-primitiveTopology-10747
        case ActionVUID::PRIMITIVE_TOPOLOGY_10747: suffix = "primitiveTopology-10747"; break;
        // ### VUID-vkCmdDraw-primitiveTopology-10748
        case ActionVUID::PRIMITIVE_TOPOLOGY_10748: suffix = "primitiveTopology-10748"; break;
        // ### VUID-vkCmdDraw-None-09637
        case ActionVUID::PRIMITIVE_RESTART_09637: suffix = "None-09637"; break;
        // ### VUID-vkCmdDraw-None-10909
        case ActionVUID::PRIMITIVE_RESTART_10909: suffix = "None-10909"; break;
        // ### VUID-vkCmdDraw-Input-08734
        case ActionVUID::VERTEX_INPUT_08734: suffix = "Input-08734"; break;
        // ### VUID-vkCmdDraw-format-08936
        case ActionVUID::VERTEX_INPUT_FORMAT_08936: suffix = "format-08936"; break;
        // ### VUID-vkCmdDraw-format-08937
        case ActionVUID::VERTEX_INPUT_FORMAT_08937: suffix = "format-08937"; break;
        // ### VUID-vkCmdDraw-None-09203
        case ActionVUID::VERTEX_INPUT_FORMAT_09203: suffix = "None-09203"; break;
        // ### VUID-vkCmdDraw-Input-07939
        case ActionVUID::VERTEX_INPUT_FORMAT_07939: suffix = "Input-07939"; break;
        // ### VUID-vkCmdDraw-commandBuffer-04617
        case ActionVUID::RAY_QUERY_04617: suffix = "commandBuffer-04617"; break;
        // ### VUID-vkCmdDraw-maxMultiviewInstanceIndex-02688
        case ActionVUID::MAX_MULTIVIEW_INSTANCE_INDEX_02688: suffix = "maxMultiviewInstanceIndex-02688"; break;
        // ### VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-04552
        case ActionVUID::VIEWPORT_COUNT_PRIMITIVE_SHADING_RATE_04552: suffix = "primitiveFragmentShadingRateWithMultipleViewports-04552"; break;
        // ### VUID-vkCmdDraw-primitivesGeneratedQueryWithRasterizerDiscard-06708
        case ActionVUID::PRIMITIVES_GENERATED_06708: suffix = "primitivesGeneratedQueryWithRasterizerDiscard-06708"; break;
        // ### VUID-vkCmdDraw-primitivesGeneratedQueryWithNonZeroStreams-06709
        case ActionVUID::PRIMITIVES_GENERATED_STREAMS_06709: suffix = "primitivesGeneratedQueryWithNonZeroStreams-06709"; break;
        // ### VUID-vkCmdDraw-stage-06481
        case ActionVUID::INVALID_MESH_SHADER_STAGES_06481: suffix = "stage-06481"; break;
        // ### VUID-vkCmdDraw-None-10772
        case ActionVUID::SHADER_OBJECT_MULTIVIEW_10772: suffix = "None-10772"; break;
        // ### VUID-vkCmdDraw-nextStage-10745
        case ActionVUID::NEXT_STAGE_10745: suffix = "nextStage-10745"; break;
        // ### VUID-vkCmdDraw-None-08684
        case ActionVUID::VERTEX_SHADER_08684: suffix = "None-08684"; break;
        // ### VUID-vkCmdDraw-None-08685
        case ActionVUID::TESSELLATION_CONTROL_SHADER_08685: suffix = "None-08685"; break;
        // ### VUID-vkCmdDraw-None-08686
        case ActionVUID::TESSELLATION_EVALUATION_SHADER_08686: suffix = "None-08686"; break;
        // ### VUID-vkCmdDraw-None-08687
        case ActionVUID::GEOMETRY_SHADER_08687: suffix = "None-08687"; break;
        // ### VUID-vkCmdDraw-None-08688
        case ActionVUID::FRAGMENT_SHADER_08688: suffix = "None-08688"; break;
        // ### VUID-vkCmdDraw-None-08689
        case ActionVUID::TASK_SHADER_08689: suffix = "None-08689"; break;
        // ### VUID-vkCmdDraw-None-08690
        case ActionVUID::MESH_SHADER_08690: suffix = "None-08690"; break;
        // ### VUID-vkCmdDraw-None-08693
        case ActionVUID::VERT_MESH_SHADER_08693: suffix = "None-08693"; break;
        // ### VUID-vkCmdDraw-None-08696
        case ActionVUID::VERT_TASK_MESH_SHADER_08696: suffix = "None-08696"; break;
        // ### VUID-vkCmdDraw-None-08698
        case ActionVUID::LINKED_SHADERS_08698: suffix = "None-08698"; break;
        // ### VUID-vkCmdDraw-None-08699
        case ActionVUID::LINKED_SHADERS_08699: suffix = "None-08699"; break;
        // ### VUID-vkCmdDraw-None-08878
        case ActionVUID::SHADERS_PUSH_CONSTANTS_08878: suffix = "None-08878"; break;
        // ### VUID-vkCmdDraw-None-08879
        case ActionVUID::SHADERS_DESCRIPTOR_LAYOUTS_08879: suffix = "None-08879"; break;
        // ### VUID-vkCmdDraw-None-08885
        case ActionVUID::DRAW_SHADERS_NO_TASK_MESH_08885: suffix = "None-08885"; break;
        // ### VUID-vkCmdDraw-OpExecutionMode-12239
        case ActionVUID::TESSELLATION_SUBDIVISION_12239: suffix = "OpExecutionMode-12239"; break;
        // ### VUID-vkCmdDraw-OpExecutionMode-12240
        case ActionVUID::TESSELLATION_TRIANGLES_12240: suffix = "OpExecutionMode-12240"; break;
        // ### VUID-vkCmdDraw-OpExecutionMode-12241
        case ActionVUID::TESSELLATION_SEGMENT_12241: suffix = "OpExecutionMode-12241"; break;
        // ### VUID-vkCmdDraw-OpExecutionMode-12242
        case ActionVUID::TESSELLATION_PATCH_SIZE_12242: suffix = "OpExecutionMode-12242"; break;
        // ### VUID-vkCmdDraw-primitiveFragmentShadingRateWithMultipleViewports-08642
        case ActionVUID::SET_VIEWPORT_WITH_COUNT_08642: suffix = "primitiveFragmentShadingRateWithMultipleViewports-08642"; break;
        // ### VUID-vkCmdDraw-pNext-07935
        case ActionVUID::RASTERIZATION_SAMPLES_07935: suffix = "pNext-07935"; break;
        // ### VUID-vkCmdDraw-stage-07073
        case ActionVUID::MESH_SHADER_QUERIES_07073: suffix = "stage-07073"; break;
        // ### VUID-vkCmdDraw-layers-10831
        case ActionVUID::FDM_LAYERED_10831: suffix = "layers-10831"; break;
        // ### VUID-vkCmdDraw-pColorAttachments-08963
        case ActionVUID::COLOR_ATTACHMENT_08963: suffix = "pColorAttachments-08963"; break;
        // ### VUID-vkCmdDraw-pDepthAttachment-08964
        case ActionVUID::DEPTH_ATTACHMENT_08964: suffix = "pDepthAttachment-08964"; break;
        // ### VUID-vkCmdDraw-pStencilAttachment-08965
        case ActionVUID::STENCIL_ATTACHMENT_08965: suffix = "pStencilAttachment-08965"; break;
        // ### VUID-vkCmdDraw-pNext-09461
        case ActionVUID::VERTEX_INPUT_09461: suffix = "pNext-09461"; break;
        // ### VUID-vkCmdDraw-None-09462
        case ActionVUID::VERTEX_INPUT_09462: suffix = "None-09462"; break;
        // ### VUID-vkCmdDraw-flags-10582
        case ActionVUID::RENDERING_CONTENTS_10582: suffix = "flags-10582"; break;
        // ### VUID-vkCmdDraw-None-08876
        case ActionVUID::RENDER_PASS_BEGAN_08876: suffix = "None-08876"; break;
        // ### VUID-vkCmdDraw-unnormalizedCoordinates-09635
        case ActionVUID::UNNORMALIZED_COORDINATES_09635: suffix = "unnormalizedCoordinates-09635"; break;
        // ### VUID-vkCmdDraw-OpTypeTensorARM-09906
        case ActionVUID::SPIRV_OPTYPETENSORARM_09906: suffix = "OpTypeTensorARM-09906"; break;
        // ### VUID-vkCmdDraw-commandBuffer-10746
        case ActionVUID::TILE_MEMORY_HEAP_10746: suffix = "commandBuffer-10746"; break;
        // ### VUID-vkCmdDraw-None-13118
        case ActionVUID::BIND_VERTEX_BUFFERS_3_STRIDE_13118: suffix = "None-13118"; break;
        // ### VUID-vkCmdDraw-None-08115
        case ActionVUID::DESCRIPTOR_BUFFER_BIT_08115: suffix = "None-08115"; break;
        // ### VUID-vkCmdDraw-None-08117
        case ActionVUID::DESCRIPTOR_BUFFER_BIT_08117: suffix = "None-08117"; break;
        // ### VUID-vkCmdDraw-None-08600
        case ActionVUID::COMPATIBLE_PIPELINE_08600: suffix = "None-08600"; break;
        // ### VUID-vkCmdDraw-None-08606
        case ActionVUID::PIPELINE_BOUND_08606: suffix = "None-08606"; break;
        // ### VUID-vkCmdDraw-None-02721
        case ActionVUID::VERTEX_BINDING_ATTRIBUTE_02721: suffix = "None-02721"; break;
        // ### VUID-vkCmdDraw-commandBuffer-02707
        case ActionVUID::CB_UNPROTECTED_02707: suffix = "commandBuffer-02707"; break;
        // ### VUID-vkCmdDraw-commandBuffer-02712
        case ActionVUID::CB_PROTECTED_02712: suffix = "commandBuffer-02712"; break;
        // ### VUID-vkCmdDraw-None-08114
        case ActionVUID::DESCRIPTOR_08114: suffix = "None-08114"; break;
        // ### VUID-vkCmdDraw-pDescription-09900
        case ActionVUID::TENSOR_09900: suffix = "pDescription-09900"; break;
        // ### VUID-vkCmdDraw-dimensionCount-09905
        case ActionVUID::TENSOR_09905: suffix = "dimensionCount-09905"; break;
        // ### VUID-vkCmdDraw-None-10677
        case ActionVUID::PER_TILE_DRAW_ENABLED_10677: suffix = "None-10677"; break;
        // ### VUID-vkCmdDraw-None-10678
        case ActionVUID::TILE_SHADING_SHADER_STAGE_10678: suffix = "None-10678"; break;
        // ### VUID-vkCmdDraw-None-10679
        case ActionVUID::PER_TILE_MODEL_FEEDBACK_LOOP_IMAGE_ACCESS_10679: suffix = "None-10679"; break;

        // ### VUID-vkCmdDraw-OpImageSampleWeightedQCOM-06971
        case ActionVUID::IMAGE_SAMPLE_WEIGHTED_QCOM_06971: suffix = "OpImageSampleWeightedQCOM-06971"; break;
        // ### VUID-vkCmdDraw-OpImageSampleWeightedQCOM-06972
        case ActionVUID::IMAGE_SAMPLE_WEIGHTED_QCOM_06972: suffix = "OpImageSampleWeightedQCOM-06972"; break;
        // ### VUID-vkCmdDraw-OpImageBoxFilterQCOM-06973
        case ActionVUID::IMAGE_BOX_FILTER_QCOM_06973: suffix = "OpImageBoxFilterQCOM-06973"; break;
        // ### VUID-vkCmdDraw-OpImageBlockMatchSSDQCOM-06974
        case ActionVUID::IMAGE_BLOCK_MATCH_SSD_QCOM_06974: suffix = "OpImageBlockMatchSSDQCOM-06974"; break;
        // ### VUID-vkCmdDraw-OpImageSampleWeightedQCOM-06977
        case ActionVUID::IMAGE_PROCESSING_SAMPLE_QCOM_06977: suffix = "OpImageSampleWeightedQCOM-06977"; break;
        // ### VUID-vkCmdDraw-OpImageSampleWeightedQCOM-06978
        case ActionVUID::IMAGE_PROCESSING_SAMPLE_QCOM_06978: suffix = "OpImageSampleWeightedQCOM-06978"; break;
        // ### VUID-vkCmdDraw-OpImageBlockMatchWindow-09215
        case ActionVUID::IMAGE_BLOCK_MATCH_WINDOW_QCOM_09215: suffix = "OpImageBlockMatchWindow-09215"; break;
        // ### VUID-vkCmdDraw-OpImageBlockMatchWindow-09216
        case ActionVUID::IMAGE_BLOCK_MATCH_WINDOW_QCOM_09216: suffix = "OpImageBlockMatchWindow-09216"; break;
        // ### VUID-vkCmdDraw-OpImageBlockMatchSADQCOM-12420
        case ActionVUID::IMAGE_BLOCK_MATCH_SAD_QCOM_12420: suffix = "OpImageBlockMatchSADQCOM-12420"; break;

        // ### VUID-vkCmdDraw-viewMask-06178
        case ActionVUID::DYNAMIC_RENDERING_VIEW_MASK_06178: suffix = "viewMask-06178"; break;
        // ### VUID-vkCmdDraw-colorAttachmentCount-06179
        case ActionVUID::DYNAMIC_RENDERING_COLOR_COUNT_06179: suffix = "colorAttachmentCount-06179"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08910
        case ActionVUID::DYNAMIC_RENDERING_COLOR_FORMATS_08910: suffix = "dynamicRenderingUnusedAttachments-08910"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08911
        case ActionVUID::DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_08911: suffix = "dynamicRenderingUnusedAttachments-08911"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08912
        case ActionVUID::DYNAMIC_RENDERING_UNDEFINED_COLOR_FORMATS_08912: suffix = "dynamicRenderingUnusedAttachments-08912"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08913
        case ActionVUID::DYNAMIC_RENDERING_UNDEFINED_DEPTH_FORMAT_08913: suffix = "dynamicRenderingUnusedAttachments-08913"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08916
        case ActionVUID::DYNAMIC_RENDERING_UNDEFINED_STENCIL_FORMAT_08916: suffix = "dynamicRenderingUnusedAttachments-08916"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08914
        case ActionVUID::DYNAMIC_RENDERING_DEPTH_FORMAT_08914: suffix = "dynamicRenderingUnusedAttachments-08914"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08915
        case ActionVUID::DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_08915: suffix = "dynamicRenderingUnusedAttachments-08915"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08917
        case ActionVUID::DYNAMIC_RENDERING_STENCIL_FORMAT_08917: suffix = "dynamicRenderingUnusedAttachments-08917"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08918
        case ActionVUID::DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_08918: suffix = "dynamicRenderingUnusedAttachments-08918"; break;
        // ### VUID-vkCmdDraw-imageView-06183
        case ActionVUID::DYNAMIC_RENDERING_FSR_06183: suffix = "imageView-06183"; break;
        // ### VUID-vkCmdDraw-imageView-06184
        case ActionVUID::DYNAMIC_RENDERING_FDM_06184: suffix = "imageView-06184"; break;
        // ### VUID-vkCmdDraw-colorAttachmentCount-06185
        case ActionVUID::DYNAMIC_RENDERING_COLOR_SAMPLE_06185: suffix = "colorAttachmentCount-06185"; break;
        // ### VUID-vkCmdDraw-pDepthAttachment-06186
        case ActionVUID::DYNAMIC_RENDERING_DEPTH_SAMPLE_06186: suffix = "pDepthAttachment-06186"; break;
        // ### VUID-vkCmdDraw-pStencilAttachment-06187
        case ActionVUID::DYNAMIC_RENDERING_STENCIL_SAMPLE_06187: suffix = "pStencilAttachment-06187"; break;
        // ### VUID-vkCmdDraw-renderPass-06198
        case ActionVUID::DYNAMIC_RENDERING_06198: suffix = "renderPass-06198"; break;
        // ### VUID-vkCmdDraw-multisampledRenderToSingleSampled-07285
        case ActionVUID::DYNAMIC_RENDERING_07285: suffix = "multisampledRenderToSingleSampled-07285"; break;
        // ### VUID-vkCmdDraw-multisampledRenderToSingleSampled-07286
        case ActionVUID::DYNAMIC_RENDERING_07286: suffix = "multisampledRenderToSingleSampled-07286"; break;
        // ### VUID-vkCmdDraw-multisampledRenderToSingleSampled-07287
        case ActionVUID::DYNAMIC_RENDERING_07287: suffix = "multisampledRenderToSingleSampled-07287"; break;
        // ### VUID-vkCmdDraw-None-09548
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_LOCATION_09548: suffix = "None-09548"; break;
        // ### VUID-vkCmdDraw-None-09549
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_INDEX_09549: suffix = "None-09549"; break;
        // ### VUID-vkCmdDraw-None-10927
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_INDEX_10927: suffix = "None-10927"; break;
        // ### VUID-vkCmdDraw-None-10928
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_INDEX_10928: suffix = "None-10928"; break;
        // ### VUID-vkCmdDraw-None-09642
        case ActionVUID::DYNAMIC_RENDERING_DITHERING_09642: suffix = "None-09642"; break;
        // ### VUID-vkCmdDraw-None-09643
        case ActionVUID::DYNAMIC_RENDERING_DITHERING_09643: suffix = "None-09643"; break;
        // ### VUID-vkCmdDraw-dynamicRenderingLocalRead-11797
        case ActionVUID::DYNAMIC_RENDERING_LOCAL_READ_11797: suffix = "dynamicRenderingLocalRead-11797"; break;

        // ### VUID-vkCmdDrawIndexed-None-07312
        case ActionVUID::INDEX_BINDING_07312: suffix = "None-07312"; break;
        // ### VUID-vkCmdDrawIndexed-primitiveRestartIndex-12401
        case ActionVUID::PRIMITIVE_RESTART_INDEX_12401: suffix = "primitiveRestartIndex-12401"; break;

        // ### VUID-vkCmdDispatch-None-10743
        case ActionVUID::COMPUTE_NOT_BOUND_10743: suffix = "None-10743"; break;
        // ### VUID-vkCmdDispatch-None-10674
        case ActionVUID::PER_TILE_DISPATCH_ENABLED_10674: suffix = "None-10674"; break;

        // ### VUID-vkCmdDrawMeshTasksEXT-stage-06480
        case ActionVUID::MESH_SHADER_STAGES_06480: suffix = "stage-06480"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-None-08694
        case ActionVUID::TASK_MESH_SHADER_08694: suffix = "None-08694"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-None-08695
        case ActionVUID::TASK_MESH_SHADER_08695: suffix = "None-08695"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-pStages-10680
        case ActionVUID::BOUND_NON_MESH_10680: suffix = "pStages-10680"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-None-07074
        case ActionVUID::XFB_QUERIES_07074: suffix = "None-07074"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-None-07075
        case ActionVUID::PG_QUERIES_07075: suffix = "None-07075"; break;
        // ### VUID-vkCmdDrawMeshTasksEXT-pipelineStatistics-07076
        case ActionVUID::PIPELINE_STATISTICS_QUERIES_07076: suffix = "pipelineStatistics-07076"; break;

        // ### VUID-vkCmdTraceRaysKHR-None-09458
        case ActionVUID::RTX_STACK_SIZE_09458: suffix = "None-09458"; break;
        // ### VUID-vkCmdTraceRaysKHR-commandBuffer-03635
        case ActionVUID::RAY_QUERY_PROTECT_03635: suffix = "commandBuffer-03635"; break;

        // ### VUID-vkCmdDraw-uniformBuffers-06935
        case ActionVUID::UNIFORM_ACCESS_OOB_06935: suffix = "uniformBuffers-06935"; break;
        // ### VUID-vkCmdDraw-storageBuffers-06936
        case ActionVUID::STORAGE_ACCESS_OOB_06936: suffix = "storageBuffers-06936"; break;
        // ### VUID-vkCmdDraw-None-08612
        case ActionVUID::UNIFORM_ACCESS_OOB_08612: suffix = "None-08612"; break;
        // ### VUID-vkCmdDraw-None-08613
        case ActionVUID::STORAGE_ACCESS_OOB_08613: suffix = "None-08613"; break;
        // ### VUID-vkCmdDraw-None-08114
        case ActionVUID::INVALID_DESCRIPTOR_08114: suffix = "None-08114"; break;
        // ### VUID-vkCmdDraw-None-10068
        case ActionVUID::DESCRIPTOR_INDEX_OOB_10068: suffix = "None-10068"; break;
        // ### VUID-vkCmdDraw-None-11309
        case ActionVUID::DESCRIPTOR_HEAP_OOB_11309: suffix = "None-11309"; break;
        // ### VUID-vkCmdDraw-None-11297
        case ActionVUID::DESCRIPTOR_HEAP_ALIGNMENT_11297: suffix = "None-11297"; break;
        // ### VUID-vkCmdDraw-None-11298
        case ActionVUID::DESCRIPTOR_HEAP_ALIGNMENT_11298: suffix = "None-11298"; break;
        // ### VUID-vkCmdDraw-None-11299
        case ActionVUID::DESCRIPTOR_HEAP_ALIGNMENT_11299: suffix = "None-11299"; break;
        // ### VUID-vkCmdDraw-None-11300
        case ActionVUID::DESCRIPTOR_HEAP_INDIRECT_INDEX_PUSH_11300: suffix = "None-11300"; break;
        // ### VUID-vkCmdDraw-None-11304
        case ActionVUID::DESCRIPTOR_HEAP_INDIRECT_ADDRESS_PUSH_11304: suffix = "None-11304"; break;
        // ### VUID-vkCmdDraw-None-11441
        case ActionVUID::DESCRIPTOR_HEAP_ADDRESS_BUFFER_ALIGNMENT_11441: suffix = "None-11441"; break;
        // ### VUID-vkCmdDraw-None-11442
        case ActionVUID::DESCRIPTOR_HEAP_ADDRESS_BUFFER_ALIGNMENT_11442: suffix = "None-11442"; break;
        // ### VUID-vkCmdDraw-None-11301
        case ActionVUID::DESCRIPTOR_HEAP_INVALID_ADDRESS_11301: suffix = "None-11301"; break;
        // ### VUID-vkCmdDraw-None-11302
        case ActionVUID::DESCRIPTOR_HEAP_INVALID_ADDRESS_11302: suffix = "None-11302"; break;
        // ### VUID-vkCmdDraw-None-11305
        case ActionVUID::DESCRIPTOR_HEAP_INVALID_ADDRESS_11305: suffix = "None-11305"; break;
        // ### VUID-vkCmdDraw-None-11306
        case ActionVUID::DESCRIPTOR_HEAP_INVALID_ADDRESS_11306: suffix = "None-11306"; break;
        // ### VUID-vkCmdDrawIndirectCount-countBuffer-02717
        case ActionVUID::INDIRECT_COUNT_LIMIT: suffix = "countBuffer-02717"; break;
    }
    // clang-format on

    if (!suffix) {
        return kVUIDUndefined;
    }

    // TODO - need a more practical way to do these
    if (function == Func::vkCmdDrawIndexedIndirectCountKHR) {
        function = Func::vkCmdDrawIndexedIndirectCount;
    } else if (function == Func::vkCmdDrawIndirectCountKHR) {
        function = Func::vkCmdDrawIndirectCount;
    } else if (function == Func::vkCmdDispatchBaseKHR) {
        function = Func::vkCmdDispatchBase;
    }

    // When c++20 is added, turn to std::format
    return std::string("VUID-") + String(function) + "-" + suffix;
}

}  // namespace vvl
