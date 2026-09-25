#!/usr/bin/python3 -i
#
# Copyright (c) 2023-2026 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from dataclasses import dataclass
from base_generator import BaseGenerator
from vulkan_object import DynamicState

# Small wrapper around the VulkanObject DynamicState class
@dataclass
class PipelineDynamicState:
    name: str # ex) VK_DYNAMIC_STATE_DEPTH_BIAS
    commands: list[str] # ex) [vkCmdSetDepthBias, vkCmdSetDepthBias2EXT]
    state: DynamicState # The logical state from the XML this VkDynamicState is part of

# How to describe the dynamic state (keyed by the XML logical name) that other dynamic state depends on.
# The tuple is (message if set dynamically, message if set statically in the pipeline)
stateRequiredDescription = {
    'stencilTestEnable': (
        'vkCmdSetStencilTestEnable last set stencilTestEnable to VK_TRUE.',
        'VkPipelineDepthStencilStateCreateInfo::stencilTestEnable was VK_TRUE in the last bound graphics pipeline.'),
    'depthTestEnable': (
        'vkCmdSetDepthTestEnable last set depthTestEnable to VK_TRUE.',
        'VkPipelineDepthStencilStateCreateInfo::depthTestEnable was VK_TRUE in the last bound graphics pipeline.'),
    'depthBoundsTestEnable': (
        'vkCmdSetDepthBoundsTestEnable last set depthBoundsTestEnable to VK_TRUE.',
        'VkPipelineDepthStencilStateCreateInfo::depthBoundsTestEnable was VK_TRUE in the last bound graphics pipeline.'),
    'depthBiasEnable': (
        'vkCmdSetDepthBiasEnable last set depthBiasEnable to VK_TRUE.',
        'VkPipelineRasterizationStateCreateInfo::depthBiasEnable was VK_TRUE in the last bound graphics pipeline.'),
    'depthClampEnable': (
        'vkCmdSetDepthClampEnableEXT last set depthClampEnable to VK_TRUE.',
        'VkPipelineRasterizationStateCreateInfo::depthClampEnable was VK_TRUE in the last bound graphics pipeline.'),
    'logicOpEnable': (
        'vkCmdSetLogicOpEnableEXT last set logicOpEnable to VK_TRUE.',
        'VkPipelineColorBlendStateCreateInfo::logicOpEnable was VK_TRUE in the last bound graphics pipeline.'),
    'lineStippleEnable': (
        'vkCmdSetLineStippleEnableEXT last set stippledLineEnable to VK_TRUE.',
        'VkPipelineRasterizationLineStateCreateInfo::stippledLineEnable was VK_TRUE in the last bound graphics pipeline.'),
    'sampleLocationsEnable': (
        'vkCmdSetSampleLocationsEnableEXT last set sampleLocationsEnable to VK_TRUE.',
        'VkPipelineMultisampleStateCreateInfo::pNext->VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsEnable was VK_TRUE in the last bound graphics pipeline.'),
    'coverageModulationTableEnable': (
        'vkCmdSetCoverageModulationTableEnableNV last set coverageModulationTableEnable to VK_TRUE.',
        'VkPipelineMultisampleStateCreateInfo::pNext->VkPipelineCoverageModulationStateCreateInfoNV::coverageModulationTableEnable was VK_TRUE in the last bound graphics pipeline.'),
    'coverageToColorEnable': (
        'vkCmdSetCoverageToColorEnableNV last set coverageToColorEnable to VK_TRUE.',
        'VkPipelineMultisampleStateCreateInfo::pNext->VkPipelineCoverageToColorStateCreateInfoNV::coverageToColorEnable was VK_TRUE in the last bound graphics pipeline.'),
    'shadingRateImageEnable': (
        'vkCmdSetShadingRateImageEnableNV last set shadingRateImageEnable to VK_TRUE.',
        'VkPipelineViewportStateCreateInfo::pNext->VkPipelineViewportShadingRateImageStateCreateInfoNV::shadingRateImageEnable was VK_TRUE in the last bound graphics pipeline.'),
    'viewportWScalingEnable': (
        'vkCmdSetViewportWScalingEnableNV last set viewportWScalingEnable to VK_TRUE.',
        'VkPipelineViewportStateCreateInfo::pNext->VkPipelineViewportWScalingStateCreateInfoNV::viewportWScalingEnable was VK_TRUE in the last bound graphics pipeline.'),
    'discardRectangleEnable': (
        'vkCmdSetDiscardRectangleEnableEXT last set discardRectangleEnable to VK_TRUE.',
        'VkGraphicsPipelineCreateInfo::pNext->VkPipelineDiscardRectangleStateCreateInfoEXT::discardRectangleCount was greater than zero in the last bound graphics pipeline.'),
}

#
# DynamicStateOutputGenerator - Generate helpers for VkDynamicState
class DynamicStateOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        # Flatten the logical states from the XML into VkDynamicState
        # keep in VkDynamicState enum order so the generated code is stable
        states = dict()
        for dynamicState in self.vk.dynamicStates.values():
            for command in dynamicState.commands:
                if command.pipelineEnum not in states:
                    states[command.pipelineEnum] = PipelineDynamicState(command.pipelineEnum, [], dynamicState)
                states[command.pipelineEnum].commands.append(command.name)
        self.pipelineDynamicStates = [states[field.name] for field in self.vk.enums['VkDynamicState'].fields if field.name in states]

        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2023-2026 Valve Corporation
            * Copyright (c) 2023-2026 LunarG, Inc.
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
            ****************************************************************************/\n''')

        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'dynamic_state_helper.h':
            self.generateHeader()
        elif self.filename == 'dynamic_state_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <vulkan/vulkan_core.h>
            #include <bitset>

            namespace vvl {
                class Pipeline;
            }  // namespace vvl


            // Reorders VkDynamicState so it can be a bitset
            typedef enum CBDynamicState {
            ''')
        for index, field in enumerate(self.vk.enums['VkDynamicState'].fields, start=1):
            # VK_DYNAMIC_STATE_LINE_WIDTH -> STATE_LINE_WIDTH
            out.append(f'CB_DYNAMIC_{field.name[11:]} = {index},\n')

        out.append(f'CB_DYNAMIC_STATE_STATUS_NUM = {len(self.vk.enums["VkDynamicState"].fields) + 1}')
        out.append('''
            } CBDynamicState;

            using CBDynamicFlags = std::bitset<CB_DYNAMIC_STATE_STATUS_NUM>;
            VkDynamicState ConvertToDynamicState(CBDynamicState dynamic_state);
            CBDynamicState ConvertToCBDynamicState(VkDynamicState dynamic_state);
            const char* DynamicStateToString(CBDynamicState dynamic_state);
            std::string DynamicStatesToString(CBDynamicFlags const &dynamic_states);
            std::string DynamicStatesCommandsToString(CBDynamicFlags const &dynamic_states);

            std::string DescribeDynamicStateCommand(CBDynamicState dynamic_state);
            std::string DescribeDynamicStateDependency(CBDynamicState dynamic_state, const vvl::Pipeline* pipeline);

            // We build these up at code gen time to quickly use at runtime
            // TODO - tried to make these constexpr, but seems bitset over 64 bits doesn't like that it seems
            //
            ''')

        const_names = {
            "VK_SHADER_STAGE_VERTEX_BIT" : "kVertexDynamicState",
            "VK_SHADER_STAGE_FRAGMENT_BIT" : "kFragmentDynamicState",
            "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT" : "kTessControlDynamicState",
            "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT" : "kTessEvalDynamicState",
            "VK_SHADER_STAGE_GEOMETRY_BIT" : "kGeometryDynamicState",
        }
        stage_map = dict()
        for dynamicState in self.pipelineDynamicStates:
            stage_map.setdefault(dynamicState.state.shaderStage, []).append(dynamicState.name)

        # sort so each OS generates same order here
        for stage, states in sorted(stage_map.items()):
            if stage == 'VK_SHADER_STAGE_ALL':
                continue
            out.append(f'// All state is only tied to {stage}\n')
            out.append(f'const CBDynamicFlags {const_names[stage]} =\n')
            for index, state in enumerate(states):
                seperator = ' |' if (index + 1) != len(states) else ';\n'
                out.append(f'CBDynamicFlags(1) << CB_{state[3:]}{seperator}\n')

        out.append('''
            // Some extra const also used, not sure where to put these otherwise
            //
            // VkPipelineColorBlendStateCreateInfo::attachmentCount
            const CBDynamicFlags kColorBlendStateAttachmentCountDynamic =
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT |
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT |
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT |
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT;

            // VkGraphicsPipelineCreateInfo::pDepthStencilState
            const CBDynamicFlags kDepthStencilStateDynamic =
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE |
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE |
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_DEPTH_COMPARE_OP |
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE |
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE |
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_STENCIL_OP |
                CBDynamicFlags(1) << CB_DYNAMIC_STATE_DEPTH_BOUNDS;

            // VkGraphicsPipelineCreateInfo::pRasterizationState
            const CBDynamicFlags kRasterizationStateDynamic =
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_POLYGON_MODE_EXT |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_CULL_MODE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_FRONT_FACE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_DEPTH_BIAS |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_LINE_WIDTH;

            // VkGraphicsPipelineCreateInfo::pColorBlendState
            const CBDynamicFlags kColorBlendStateDynamic =
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_LOGIC_OP_EXT |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_BLEND_CONSTANTS;

            // VK_EXT_extended_dynamic_state
            const CBDynamicFlags kExtendedDynamicState1 =
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_CULL_MODE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_FRONT_FACE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_DEPTH_COMPARE_OP |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE |
                CBDynamicFlags(1) <<  CB_DYNAMIC_STATE_STENCIL_OP;
            ''')


        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "state_tracker/pipeline_state.h"

            VkDynamicState ConvertToDynamicState(CBDynamicState dynamic_state) {
                switch (dynamic_state) {
            ''')
        for field in self.vk.enums['VkDynamicState'].fields:
            # VK_DYNAMIC_STATE_LINE_WIDTH -> STATE_LINE_WIDTH
            out.append(f'case CB_DYNAMIC_{field.name[11:]}:\n')
            out.append(f'    return {field.name};\n')

        out.append('''
                    default:
                        return VK_DYNAMIC_STATE_MAX_ENUM;
                }
            }
            ''')

        out.append('''
            CBDynamicState ConvertToCBDynamicState(VkDynamicState dynamic_state) {
                switch (dynamic_state) {
            ''')

        for field in self.vk.enums['VkDynamicState'].fields:
            # VK_DYNAMIC_STATE_LINE_WIDTH -> STATE_LINE_WIDTH
            out.append(f'case {field.name}:\n')
            out.append(f'    return CB_DYNAMIC_{field.name[11:]};\n')
        out.append('''
                    default:
                        return CB_DYNAMIC_STATE_STATUS_NUM;
                }
            }
            ''')

        out.append('''
            const char* DynamicStateToString(CBDynamicState dynamic_state) {
                return string_VkDynamicState(ConvertToDynamicState(dynamic_state));
            }

            std::string DynamicStatesToString(CBDynamicFlags const& dynamic_states) {
                std::string ret;
                // enum is not zero based
                for (int index = 1; index < CB_DYNAMIC_STATE_STATUS_NUM; ++index) {
                    CBDynamicState status = static_cast<CBDynamicState>(index);
                    if (dynamic_states[status]) {
                        if (!ret.empty()) ret.append("|");
                        ret.append(string_VkDynamicState(ConvertToDynamicState(status)));
                    }
                }
                if (ret.empty()) ret.append("(Unknown Dynamic State)");
                return ret;
            }

            std::string DynamicStatesCommandsToString(CBDynamicFlags const& dynamic_states) {
                std::string ret;
                // enum is not zero based
                for (int index = 1; index < CB_DYNAMIC_STATE_STATUS_NUM; ++index) {
                    CBDynamicState status = static_cast<CBDynamicState>(index);
                    if (dynamic_states[status]) {
                        if (!ret.empty()) ret.append(", ");
                        ret.append(DescribeDynamicStateCommand(status));
                    }
                }
                if (ret.empty()) ret.append("(Unknown Dynamic State)");
                return ret;
            }
            ''')

        out.append('''
            std::string DescribeDynamicStateCommand(CBDynamicState dynamic_state) {
                std::ostringstream ss;
                switch (dynamic_state) {
        ''')
        for dynamicState in self.pipelineDynamicStates:
            # Some state can be set by multiple commands (ex. vkCmdSetDepthBias and vkCmdSetDepthBias2EXT)
            funcs = ' << " or " << '.join(f'String(vvl::Func::{command})' for command in dynamicState.commands)
            out.append(f'case CB_{dynamicState.name[3:]}:\n')
            out.append(f'    ss << {funcs};\n')
            out.append('    break;')
        out.append('''
                    default:
                        ss << "(Unknown Dynamic State) " << String(vvl::Func::Empty);
                }

                return ss.str();
            }
        ''')

        out.append('''
            // For anything with multiple uses
            static std::string_view rasterizer_discard_enable_dynamic{"vkCmdSetRasterizerDiscardEnable last set rasterizerDiscardEnable to VK_FALSE.\\n"};
            static std::string_view rasterizer_discard_enable_static{"VkPipelineRasterizationStateCreateInfo::rasterizerDiscardEnable was VK_FALSE in the last bound graphics pipeline.\\n"};

            std::string DescribeDynamicStateDependency(CBDynamicState dynamic_state, const vvl::Pipeline* pipeline) {
                std::ostringstream ss;
                switch (dynamic_state) {
        ''')
        for dynamicState in self.pipelineDynamicStates:
            state = dynamicState.state
            if not state.requiresRasterization and state.stateRequired is None:
                continue
            out.append(f'case CB_{dynamicState.name[3:]}:')

            if state.requiresRasterization:
                out.append('''
                if (!pipeline || pipeline->IsDynamic(CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                    ss << rasterizer_discard_enable_dynamic;
                } else {
                    ss << rasterizer_discard_enable_static;
                }''')
            if state.stateRequired in stateRequiredDescription:
                # stateRequired is the logical name of another <dynamicstate>
                requiredEnums = {command.pipelineEnum for command in self.vk.dynamicStates[state.stateRequired].commands}
                assert len(requiredEnums) == 1, f'{state.name} depends on {state.stateRequired} which has multiple VkDynamicState {requiredEnums}'
                dynamicMessage, staticMessage = stateRequiredDescription[state.stateRequired]
                out.append(f'''
                if (!pipeline || pipeline->IsDynamic(CB_{requiredEnums.pop()[3:]})) {{
                    ss << "{dynamicMessage}\\n";
                }} else {{
                    ss << "{staticMessage}\\n";
                }}''')
            elif state.stateRequired is not None:
                assert state.stateRequired in self.vk.dynamicStates, f'{state.name} depends on unknown dynamic state {state.stateRequired}'

            out.append('    break;')
        out.append('''
                    default:
                        break; // not all state will be dependent on other state
                }

                return ss.str();
            }
        ''')

        self.write("".join(out))