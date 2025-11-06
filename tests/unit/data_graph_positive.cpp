/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (C) 2025 Arm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/data_graph_objects.h"

class PositiveDataGraph : public DataGraphTest {};

void DataGraphTest::InitBasicDataGraph() {
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_ARM_TENSORS_EXTENSION_NAME);
    AddRequiredExtensions(VK_ARM_DATA_GRAPH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tensors);
    AddRequiredFeature(vkt::Feature::dataGraph);
    AddRequiredFeature(vkt::Feature::shaderTensorAccess);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::shaderInt16);
    AddRequiredFeature(vkt::Feature::shaderInt64);
}

const std::string DataGraphTest::IncorrectSpirvMessage{"test incorrect. Possible causes: incorrect spirv, or inconsistency between spirv and tensor/constant declarations\n"} ;

void DataGraphTest::CheckSessionMemory(const vkt::DataGraphPipelineSession& session) {
    const auto &mem_reqs = session.MemReqs();
    if (mem_reqs.size() == 0) {
        GTEST_FAIL() << "No bind points, " << IncorrectSpirvMessage;
    }
    for (uint32_t i = 0; i < mem_reqs.size(); i++) {
        if (mem_reqs[i].memoryRequirements.size == 0) {
            GTEST_FAIL() << "No memory for binding " << i << ", " << IncorrectSpirvMessage;
        }
    }
}

std::vector<VkBindDataGraphPipelineSessionMemoryInfoARM> DataGraphTest::InitSessionBindInfo(const vkt::DataGraphPipelineSession& session, const std::vector<vkt::DeviceMemory>& device_mem) {
    const auto &bind_point_reqs = session.BindPointReqs();
    std::vector<VkBindDataGraphPipelineSessionMemoryInfoARM> session_bind_infos(session.MemReqs().size());
    uint32_t req_i = 0;
    for (uint32_t i = 0; i < bind_point_reqs.size(); i++) {
        if (bind_point_reqs[i].bindPointType != VK_DATA_GRAPH_PIPELINE_SESSION_BIND_POINT_TYPE_MEMORY_ARM) {
            continue;
        }

        for (uint32_t j = 0; j < bind_point_reqs[i].numObjects; j++) {
            session_bind_infos[req_i] = vku::InitStructHelper();
            session_bind_infos[req_i].session = session.handle();
            session_bind_infos[req_i].memory = device_mem[req_i].handle();
            session_bind_infos[req_i].bindPoint = bind_point_reqs[req_i].bindPoint;
            session_bind_infos[req_i].objectIndex = j;
            req_i++;
        }
    }
    return session_bind_infos;
}


TEST_F(PositiveDataGraph, ExecuteDataGraph) {
    TEST_DESCRIPTION("Create and execute a datagraph");
    InitBasicDataGraph();
    RETURN_IF_SKIP(Init());

    vkt::dg::DataGraphPipelineHelper pipeline(*this);
    pipeline.CreateDataGraphPipeline();

    VkDataGraphPipelineSessionCreateInfoARM session_ci = vku::InitStructHelper();
    session_ci.dataGraphPipeline = pipeline.Handle();
    vkt::DataGraphPipelineSession session(*m_device, session_ci);
    session.GetMemoryReqs();
    CheckSessionMemory(session);

    auto &bind_point_reqs = session.BindPointReqs();
    std::vector<vkt::DeviceMemory> device_mem(bind_point_reqs.size());
    session.AllocSessionMem(device_mem);
    auto session_bind_infos = InitSessionBindInfo(session, device_mem);
    vk::BindDataGraphPipelineSessionMemoryARM(*m_device, session_bind_infos.size(), session_bind_infos.data());

    pipeline.descriptor_set_->WriteDescriptorTensorInfo(0, &pipeline.in_tensor_view_.handle(), 0);
    pipeline.descriptor_set_->WriteDescriptorTensorInfo(1, &pipeline.out_tensor_view_.handle(), 0);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_.get()->set_, 0, nullptr);
    vk::CmdDispatchDataGraphARM(m_command_buffer, session.handle(), nullptr);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDataGraph, DISABLED_ProtectedMemoryDataGraph) {
    TEST_DESCRIPTION("Execute a datagraph with protected memory");
    InitBasicDataGraph();
    AddRequiredFeature(vkt::Feature::protectedMemory);
    AddRequiredFeature(vkt::Feature::pipelineProtectedAccess);
    RETURN_IF_SKIP(InitFramework());
    RETURN_IF_SKIP(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_PROTECTED_BIT));

    vkt::dg::HelperParameters params;
    params.protected_tensors = true;
    vkt::dg::DataGraphPipelineHelper pipeline(*this, params);

    pipeline.pipeline_ci_.flags = VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT_EXT;
    pipeline.CreateDataGraphPipeline();

    VkDataGraphPipelineSessionCreateInfoARM session_ci = vku::InitStructHelper();
    session_ci.dataGraphPipeline = pipeline.Handle();
    session_ci.flags = VK_DATA_GRAPH_PIPELINE_SESSION_CREATE_PROTECTED_BIT_ARM;

    vkt::DataGraphPipelineSession session(*m_device, session_ci);
    session.GetMemoryReqs();
    CheckSessionMemory(session);

    std::vector<vkt::DeviceMemory> device_mem(session.BindPointsCount());
    session.AllocSessionMem(device_mem, true);

    auto session_bind_infos = InitSessionBindInfo(session, device_mem);
    vk::BindDataGraphPipelineSessionMemoryARM(*m_device, session_bind_infos.size(), session_bind_infos.data());

    pipeline.descriptor_set_->WriteDescriptorTensorInfo(0, &pipeline.in_tensor_view_.handle(), 0);
    pipeline.descriptor_set_->WriteDescriptorTensorInfo(1, &pipeline.out_tensor_view_.handle(), 0);
    pipeline.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM, pipeline.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM, pipeline.pipeline_layout_.handle(), 0, 1,
                              &pipeline.descriptor_set_.get()->set_, 0, nullptr);
    vk::CmdDispatchDataGraphARM(m_command_buffer, session.handle(), nullptr);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDataGraph, ShaderModuleInPNext) {
    TEST_DESCRIPTION(
        "Pass a VkShaderModuleCreateInfo in the pNext chain of pipeline info, not as "
        "VkDataGraphPipelineShaderModuleCreateInfoARM::module.");
    InitBasicDataGraph();
    RETURN_IF_SKIP(Init());

    // create a ShaderModule to add in the pNext chain
    spvtools::SpirvTools tools{SPV_ENV_UNIVERSAL_1_6};
    const std::string& spirv_source = vkt::dg::DataGraphPipelineHelper::GetSpirvBasicDataGraph();
    std::vector<uint32_t> spirv_binary;
    if (!tools.Assemble(spirv_source, &spirv_binary)) {
        Monitor().SetError("Failed to compile SPIRV shader module");
        return;
    }
    VkShaderModuleCreateInfo shader_module_create_info = vku::InitStructHelper();
    shader_module_create_info.codeSize = spirv_binary.size() * sizeof(uint32_t);
    shader_module_create_info.pCode = spirv_binary.data();

    // 2 variants, adding the shader in different places in the pipeline's pNext chain, both must work

    {
        vkt::dg::DataGraphPipelineHelper pipeline(*this);
        // the helper constructor adds the shader module as VkDataGraphPipelineShaderModuleCreateInfoARM::module, get rid of it
        pipeline.shader_module_ci_.module = VK_NULL_HANDLE;

        // add the shader info in VkDataGraphPipelineShaderModuleCreateInfoARM::pNext
        pipeline.shader_module_ci_.pNext = &shader_module_create_info;
        pipeline.CreateDataGraphPipeline();
    }

    {
        vkt::dg::DataGraphPipelineHelper pipeline(*this);
        // the helper constructor adds the shader module as VkDataGraphPipelineShaderModuleCreateInfoARM::module, get rid of it
        pipeline.shader_module_ci_.module = VK_NULL_HANDLE;

        // add the shader info in VkDataGraphPipelineCreateInfoARM::pNext
        pipeline.pipeline_ci_.pNext = &shader_module_create_info;
        shader_module_create_info.pNext = &pipeline.shader_module_ci_;
        pipeline.shader_module_ci_.pNext = nullptr;
        pipeline.CreateDataGraphPipeline();
    }
}

TEST_F(PositiveDataGraph, DataGraphMultipleEntrypoints) {
    TEST_DESCRIPTION("Execute 2 different entrypoints in the datagraph's spirv.");
    InitBasicDataGraph();
    RETURN_IF_SKIP(Init());

    // get spirv with 2 entrypoints
    const std::string two_entrypoint_spirv = vkt::dg::DataGraphPipelineHelper::GetSpirvMultiEntryTwoDataGraph();

    // create graph at entrypoint 1
    {
        vkt::dg::HelperParameters params;
        params.spirv_source = two_entrypoint_spirv.c_str();
        params.entrypoint = "entrypoint_1";
        vkt::dg::DataGraphPipelineHelper pipeline(*this, params);
        pipeline.CreateDataGraphPipeline();
    }

    // create graph at entrypoint 2
    {
        vkt::dg::HelperParameters params;
        params.spirv_source = two_entrypoint_spirv.c_str();
        params.entrypoint = "entrypoint_2";
        vkt::dg::DataGraphPipelineHelper pipeline(*this, params);
        pipeline.CreateDataGraphPipeline();
    }
}
