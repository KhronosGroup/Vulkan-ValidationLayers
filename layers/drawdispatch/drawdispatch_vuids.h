/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
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

#pragma once

#include "error_message/error_location.h"
#include "generated/error_location_helper.h"

namespace vvl {

// Set of VUID that need to go between drawdispatch_validation.cpp and rest of CoreChecks
struct DrawDispatchVuid {
    // Save the action command here for reverse lookup so don't need to pass around both items
    const Func function;
    DrawDispatchVuid(Func func) : function(func){};
    Location loc() const { return Location(function); }

    const char* pipeline_bound_08606 = kVUIDUndefined;
    const char* index_binding_07312 = kVUIDUndefined;
    const char* compatible_pipeline_08600 = kVUIDUndefined;
    const char* render_pass_compatible_02684 = kVUIDUndefined;
    const char* render_pass_began_08876 = kVUIDUndefined;
    const char* unnormalized_coordinates_09635 = kVUIDUndefined;
    const char* indirect_protected_cb_02711 = kVUIDUndefined;
    const char* indirect_contiguous_memory_02708 = kVUIDUndefined;
    const char* indirect_count_contiguous_memory_02714 = kVUIDUndefined;
    const char* indirect_buffer_bit_02290 = kVUIDUndefined;
    const char* indirect_count_buffer_bit_02715 = kVUIDUndefined;
    const char* indirect_count_offset_04129 = kVUIDUndefined;
    const char* primitive_topology_patch_list_10286 = kVUIDUndefined;
    const char* vertex_binding_attribute_02721 = kVUIDUndefined;
    const char* unprotected_command_buffer_02707 = kVUIDUndefined;
    const char* protected_command_buffer_02712 = kVUIDUndefined;
    const char* compute_not_bound_10743 = kVUIDUndefined;
    const char* compute_inside_rp_10672 = kVUIDUndefined;
    const char* ray_query_04617 = kVUIDUndefined;
    // TODO: Some instance values are in VkBuffer. The validation in those Cmds is skipped.
    const char* max_multiview_instance_index_02688 = kVUIDUndefined;
    const char* viewport_count_primitive_shading_rate_04552 = kVUIDUndefined;
    const char* primitives_generated_06708 = kVUIDUndefined;
    const char* primitives_generated_streams_06709 = kVUIDUndefined;
    const char* mesh_shader_stages_06480 = kVUIDUndefined;
    const char* invalid_mesh_shader_stages_06481 = kVUIDUndefined;
    const char* missing_mesh_shader_stages_07091 = kVUIDUndefined;
    const char* descriptor_buffer_bit_set_08114 = kVUIDUndefined;
    const char* descriptor_buffer_bit_not_set_08115 = kVUIDUndefined;
    const char* descriptor_buffer_set_offset_missing_08117 = kVUIDUndefined;
    const char* shader_object_multiview_10772 = kVUIDUndefined;
    const char* next_stage_10745 = kVUIDUndefined;
    const char* vertex_shader_08684 = kVUIDUndefined;
    const char* tessellation_control_shader_08685 = kVUIDUndefined;
    const char* tessellation_evaluation_shader_08686 = kVUIDUndefined;
    const char* geometry_shader_08687 = kVUIDUndefined;
    const char* fragment_shader_08688 = kVUIDUndefined;
    const char* task_shader_08689 = kVUIDUndefined;
    const char* mesh_shader_08690 = kVUIDUndefined;
    const char* vert_mesh_shader_08693 = kVUIDUndefined;
    const char* task_mesh_shader_08694 = kVUIDUndefined;
    const char* task_mesh_shader_08695 = kVUIDUndefined;
    const char* vert_task_mesh_shader_08696 = kVUIDUndefined;
    const char* bound_non_mesh_10680 = kVUIDUndefined;
    const char* linked_shaders_08698 = kVUIDUndefined;
    const char* linked_shaders_08699 = kVUIDUndefined;
    const char* shaders_push_constants_08878 = kVUIDUndefined;
    const char* shaders_descriptor_layouts_08879 = kVUIDUndefined;
    const char* draw_shaders_no_task_mesh_08885 = kVUIDUndefined;
    const char* tessellation_subdivision_12239 = kVUIDUndefined;
    const char* tessellation_triangles_12240 = kVUIDUndefined;
    const char* tessellation_segment_12241 = kVUIDUndefined;
    const char* tessellation_patch_size_12242 = kVUIDUndefined;
    const char* set_viewport_with_count_08642 = kVUIDUndefined;
    const char* rasterization_samples_07935 = kVUIDUndefined;
    const char* mesh_shader_queries_07073 = kVUIDUndefined;
    const char* fdm_layered_10831 = kVUIDUndefined;
    const char* color_attachment_08963 = kVUIDUndefined;
    const char* depth_attachment_08964 = kVUIDUndefined;
    const char* stencil_attachment_08965 = kVUIDUndefined;
    const char* xfb_queries_07074 = kVUIDUndefined;
    const char* pg_queries_07075 = kVUIDUndefined;
    const char* vertex_input_09461 = kVUIDUndefined;
    const char* vertex_input_09462 = kVUIDUndefined;
    const char* image_layout_09600 = kVUIDUndefined;
    const char* rendering_contents_10582 = kVUIDUndefined;
    const char* tile_memory_heap_10746 = kVUIDUndefined;
    // TensorARM
    const char* tensorARM_pDescription_09900 = kVUIDUndefined;
    const char* tensorARM_dimensionCount_09905 = kVUIDUndefined;
    const char* spirv_OpTypeTensorARM_09906 = kVUIDUndefined;
};

const DrawDispatchVuid& GetDrawDispatchVuid(vvl::Func function);

enum class ActionVUID {
    UNKNOWN,
    VERTEX_BINDING_04007,
    VERTEX_BINDING_04008,
    SUBPASS_INDEX_02685,
    SAMPLE_LOCATION_02689,
    SAMPLE_LOCATION_07484,
    SAMPLE_LOCATION_07485,
    SAMPLE_LOCATION_07486,
    SAMPLE_LOCATION_07487,
    RASTERIZATION_SAMPLES_09211,
    LINEAR_FILTER_04553,
    LINEAR_FILTER_09598,
    LINEAR_MIPMAP_04770,
    LINEAR_MIPMAP_09599,
    EXTERNAL_FORMAT_RESOLVE_09362,
    EXTERNAL_FORMAT_RESOLVE_09363,
    EXTERNAL_FORMAT_RESOLVE_09364,
    EXTERNAL_FORMAT_RESOLVE_09365,
    EXTERNAL_FORMAT_RESOLVE_09368,
    EXTERNAL_FORMAT_RESOLVE_09369,
    EXTERNAL_FORMAT_RESOLVE_09372,
    CUSTOM_RESOLVE_11521,
    CUSTOM_RESOLVE_11522,
    CUSTOM_RESOLVE_11523,
    CUSTOM_RESOLVE_11524,
    CUSTOM_RESOLVE_11525,
    CUSTOM_RESOLVE_11529,
    CUSTOM_RESOLVE_11530,
    CUSTOM_RESOLVE_11539,
    CUSTOM_RESOLVE_11540,
    CUSTOM_RESOLVE_11860,
    CUSTOM_RESOLVE_11861,
    CUSTOM_RESOLVE_11862,
    CUSTOM_RESOLVE_11863,
    CUSTOM_RESOLVE_11864,
    CUSTOM_RESOLVE_11865,
    CUSTOM_RESOLVE_11866,
    CUSTOM_RESOLVE_11867,
    CUSTOM_RESOLVE_11868,
    CUSTOM_RESOLVE_11869,
    CUSTOM_RESOLVE_11870,
    STIPPLED_RECTANGULAR_07495,
    STIPPLED_BRESENHAM_07496,
    STIPPLED_SMOOTH_07497,
    STIPPLED_STRICT_07498,
    IMAGE_VIEW_ACCESS_04470,
    IMAGE_VIEW_ACCESS_04471,
    IMAGE_VIEW_SPARSE_04474,
    BUFFER_VIEW_ACCESS_04472,
    BUFFER_VIEW_ACCESS_04473,
    STORAGE_IMAGE_FORMAT_07028,
    STORAGE_IMAGE_FORMAT_07027,
    STORAGE_TEXEL_FORMAT_07030,
    STORAGE_TEXEL_FORMAT_07029,
    STORAGE_IMAGE_TEXEL_08795,
    STORAGE_IMAGE_TEXEL_08796,
    STORAGE_TEXEL_04469,
    IMAGE_VIEW_ATOMIC_02691,
    BUFFER_VIEW_ATOMIC_07888,
    IMAGE_VIEW_DIM_07752,
    IMAGE_VIEW_NUMERIC_07753,
    SUBRESOURCE_RP_WRTIE_06537,
    SUBRESOURCE_SUBPASS_12338,
    SUBRESOURCE_SUBPASS_12339,
    SUBRESOURCE_SUBPASS_12340,
    DESCRIPTOR_HEAP_11308,
    DESCRIPTOR_HEAP_11375,
    DESCRIPTOR_HEAP_11376,
    SAMPLER_TYPE_08609,
    SAMPLER_DREF_PROJ_08610,
    SAMPLER_BIAS_OFFSET_08611,
    SAMPLER_CUBIC_02692,
    SAMPLER_CORNER_02696,
    FILTER_CUBIC_02693,
    FILTER_CUBIC_02694,
    FILTER_CUBIC_02695,
    IMAGE_LAYOUT_00344,
    PUSH_CONSTANT_08602,

    DYNAMIC_STATE_ALL_SET_08608,
    DYNAMIC_DEPTH_COMPARE_OP_07845,
    DYNAMIC_DEPTH_BIAS_07834,
    DYNAMIC_DEPTH_BOUNDS_07836,
    DYNAMIC_CULL_MODE_07840,
    DYNAMIC_DEPTH_TEST_ENABLE_07843,
    DYNAMIC_DEPTH_WRITE_ENABLE_07844,
    DYNAMIC_STENCIL_TEST_ENABLE_07847,
    DYNAMIC_DEPTH_BIAS_ENABLE_04877,
    DYNAMIC_DEPTH_BOUND_TEST_ENABLE_07846,
    DYNAMIC_STENCIL_COMPARE_MASK_07837,
    DYNAMIC_STENCIL_WRITE_MASK_07838,
    DYNAMIC_STENCIL_REFERENCE_07839,
    DYNAMIC_STENCIL_OP_07848,
    DYNAMIC_PRIMITIVE_TOPOLOGY_07842,
    DYNAMIC_PRIMITIVE_RESTART_ENABLE_04879,
    DYNAMIC_VERTEX_INPUT_04914,
    DYNAMIC_SET_FRAGMENT_SHADING_RATE_09238,
    DYNAMIC_LOGIC_OP_04878,
    DYNAMIC_POLYGON_MODE_07621,
    DYNAMIC_RASTERIZATION_SAMPLES_07622,
    DYNAMIC_SAMPLE_MASK_07623,
    DYNAMIC_ALPHA_TO_COVERAGE_ENABLE_07624,
    DYNAMIC_ALPHA_TO_ONE_ENABLE_07625,
    DYNAMIC_LOGIC_OP_ENABLE_07626,
    DYNAMIC_RASTERIZER_DISCARD_ENABLE_04876,
    DYNAMIC_SAMPLE_LOCATIONS_ENABLE_07634,
    DYNAMIC_SAMPLE_LOCATIONS_06666,
    DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_08877,
    DYNAMIC_COLOR_WRITE_ENABLE_07749,
    DYNAMIC_COLOR_BLEND_ENABLE_07627,
    DYNAMIC_COLOR_WRITE_MASK_07629,
    DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_07639,
    DYNAMIC_DEPTH_CLAMP_CONTROL_09650,
    DYNAMIC_DEPTH_CLIP_ENABLE_07633,
    DYNAMIC_DEPTH_CLAMP_ENABLE_07620,
    DYNAMIC_EXCLUSIVE_SCISSOR_ENABLE_07878,
    DYNAMIC_EXCLUSIVE_SCISSOR_07879,
    DYNAMIC_TESSELLATION_DOMAIN_ORIGIN_07619,
    DYNAMIC_RASTERIZATION_STREAM_07630,
    DYNAMIC_CONSERVATIVE_RASTERIZATION_MODE_07631,
    DYNAMIC_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_07632,
    DYNAMIC_PROVOKING_VERTEX_MODE_07636,
    DYNAMIC_VIEWPORT_W_SCALING_ENABLE_07640,
    DYNAMIC_VIEWPORT_SWIZZLE_07641,
    DYNAMIC_COVERAGE_TO_COLOR_ENABLE_07642,
    DYNAMIC_COVERAGE_TO_COLOR_LOCATION_07643,
    DYNAMIC_COVERAGE_MODULATION_MODE_07644,
    DYNAMIC_COVERAGE_MODULATION_TABLE_ENABLE_07645,
    DYNAMIC_COVERAGE_MODULATION_TABLE_07646,
    DYNAMIC_SHADING_RATE_IMAGE_ENABLE_07647,
    DYNAMIC_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_07648,
    DYNAMIC_COVERAGE_REDUCTION_MODE_07649,
    DYNAMIC_FRONT_FACE_07841,
    DYNAMIC_VIEWPORT_COUNT_03417,
    DYNAMIC_SCISSOR_COUNT_03418,
    DYNAMIC_SET_VIEWPORT_COARSE_SAMPLE_ORDER_09233,
    DYNAMIC_SET_VIEWPORT_SHADING_RATE_PALETTE_09234,
    DYNAMIC_SET_CLIP_SPACE_W_SCALING_04138,
    DYNAMIC_PATCH_CONTROL_POINTS_04875,
    DYNAMIC_DISCARD_RECTANGLE_ENABLE_07880,
    DYNAMIC_DISCARD_RECTANGLE_MODE_07881,
    DYNAMIC_LINE_STIPPLE_EXT_07849,
    DYNAMIC_SET_LINE_WIDTH_08617,
    DYNAMIC_VERTEX_INPUT_BINDING_STRIDE_04913,
    DYNAMIC_SET_LINE_RASTERIZATION_MODE_08666,
    DYNAMIC_SET_LINE_STIPPLE_ENABLE_08669,

    DISCARD_RECTANGLE_07751,
    DISCARD_RECTANGLE_09236,
    COVERAGE_TO_COLOR_09420,
    COVERAGE_TO_COLOR_07490,
    SET_RASTERIZATION_SAMPLES_08644,
    COLOR_WRITE_ENABLE_COUNT_07750,
    DEPTH_COMPARE_SAMPLE_06479,
    DEPTH_READ_ONLY_06886,
    STENCIL_READ_ONLY_06887,
    ALPHA_TO_COVERAGE_COMPONENT_08919,
    COLOR_BLEND_ENABLE_07476,
    COLOR_WRITE_MASK_07478,
    COLOR_BLEND_EQUATION_10862,
    COLOR_BLEND_EQUATION_10863,
    COLOR_BLEND_EQUATION_10864,
    VIEWPORT_07831,
    SCISSOR_07832,
    BLEND_CONSTANTS_07835,
    DEPTH_ENABLE_08715,
    STENCIL_WRITE_MASK_08716,
    STATE_INHERITED_07850,
    COLOR_WRITE_MASK_09116,
    LINE_RASTERIZATION_10608,
    VIEWPORT_AND_SCISSOR_WITH_COUNT_03419,
    VIEWPORT_W_SCALING_08636,
    SHADING_RATE_PALETTE_08637,
    SET_VIEWPORT_SWIZZLE_09421,
    SET_VIEWPORT_SWIZZLE_07493,
    ALPHA_COMPONENT_WORD_08920,
    VIEWPORT_MULTIVIEW_12262,
    SCISSOR_MULTIVIEW_12263,
    CONVERVATIVE_RASTERIZATION_07499,
    BLEND_ENABLE_04727,
    BLEND_DUAL_SOURCE_09239,
    BLEND_ADVANCED_07480,
    PRIMITIVES_GENERATED_QUERY_07481,
    PRIMITIVE_TOPOLOGY_CLASS_07500,
    SAMPLE_LOCATIONS_07482,
    SAMPLE_LOCATIONS_07483,
    SAMPLE_LOCATIONS_07471,
    SAMPLE_LOCATIONS_ENABLE_07936,
    SAMPLE_LOCATIONS_ENABLE_07937,
    SAMPLE_LOCATIONS_ENABLE_07938,
    SAMPLE_MASK_07472,
    SAMPLE_MASK_07473,
    PRIMITIVE_TOPOLOGY_10286,
    PRIMITIVE_TOPOLOGY_10747,
    PRIMITIVE_TOPOLOGY_10748,
    PRIMITIVE_RESTART_09637,
    PRIMITIVE_RESTART_10909,
    VERTEX_INPUT_08734,
    VERTEX_INPUT_FORMAT_08936,
    VERTEX_INPUT_FORMAT_08937,
    VERTEX_INPUT_FORMAT_09203,
    VERTEX_INPUT_FORMAT_07939,

    DYNAMIC_RENDERING_VIEW_MASK_06178,
    DYNAMIC_RENDERING_COLOR_COUNT_06179,
    DYNAMIC_RENDERING_COLOR_FORMATS_08910,
    DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_08911,
    DYNAMIC_RENDERING_UNDEFINED_COLOR_FORMATS_08912,
    DYNAMIC_RENDERING_UNDEFINED_DEPTH_FORMAT_08913,
    DYNAMIC_RENDERING_UNDEFINED_STENCIL_FORMAT_08916,
    DYNAMIC_RENDERING_DEPTH_FORMAT_08914,
    DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_08915,
    DYNAMIC_RENDERING_STENCIL_FORMAT_08917,
    DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_08918,
    DYNAMIC_RENDERING_FSR_06183,
    DYNAMIC_RENDERING_FDM_06184,
    DYNAMIC_RENDERING_COLOR_SAMPLE_06185,
    DYNAMIC_RENDERING_DEPTH_SAMPLE_06186,
    DYNAMIC_RENDERING_STENCIL_SAMPLE_06187,
    DYNAMIC_RENDERING_06198,
    DYNAMIC_RENDERING_07285,
    DYNAMIC_RENDERING_07286,
    DYNAMIC_RENDERING_07287,
    DYNAMIC_RENDERING_LOCAL_LOCATION_09548,
    DYNAMIC_RENDERING_LOCAL_INDEX_09549,
    DYNAMIC_RENDERING_LOCAL_INDEX_10927,
    DYNAMIC_RENDERING_LOCAL_INDEX_10928,
    DYNAMIC_RENDERING_DITHERING_09642,
    DYNAMIC_RENDERING_DITHERING_09643,
    DYNAMIC_RENDERING_LOCAL_READ_11797,

    RTX_STACK_SIZE_09458,
    RAY_QUERY_PROTECT_03635,
};

std::string CreateActionVuid(vvl::Func function, const ActionVUID id);
std::string CreateActionVuid(const DrawDispatchVuid& vuid, const ActionVUID id);

}  // namespace vvl
