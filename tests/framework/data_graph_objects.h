/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (C) 2025 Arm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
#pragma once

#include "binding.h"
#include "descriptor_helper.h"

/**
 * This file defines the structures needed to create a simple DataGraphPipelineARM.
 * dg::ShaderModule creates a vkt::ShaderModule using SPIRV generated from a simple shader which uses a Tensor
 *
 *  #extension GL_ARM_tensors : require
 *  #extension GL_EXT_shader_explicit_arithmetic_types : require
 *  layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
 *  layout(set=0, binding=0) uniform tensorARM<int32_t, 1> tens;
 *  void main()
 *  {
 *      const uint size_x = tensorSizeARM(tens, 0);
 *  }
 *
 *  dg::PipelineLayout creates a vkt::PipelineLayout which correlates to the shader module provided above
 */

namespace vkt {
namespace dg {

struct HelperParameters {
    bool protected_tensors = false;
    const char *spirv_source = nullptr;
    const char *entrypoint = "main";
};

class DataGraphPipelineHelper {
  public:
    std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings_;
    std::unique_ptr<OneOffDescriptorSet> descriptor_set_;
    VkPipelineLayoutCreateInfo pipeline_layout_ci_ = {};
    vkt::PipelineLayout pipeline_layout_;
    VkDataGraphPipelineCreateInfoARM pipeline_ci_ = {};
    vkt::ShaderModule shader_;
    VkDataGraphPipelineShaderModuleCreateInfoARM shader_module_ci_;
    std::vector<VkDataGraphPipelineResourceInfoARM> resources_;
    vkt::Tensor in_tensor_;
    vkt::Tensor out_tensor_;
    vkt::TensorView in_tensor_view_;
    vkt::TensorView out_tensor_view_;

    VkLayerTest &layer_test_;
    vkt::Device *device_;

    explicit DataGraphPipelineHelper(VkLayerTest &test, const HelperParameters &params = HelperParameters());
    virtual ~DataGraphPipelineHelper();
    void Destroy();

    static std::string GetSpirvBasicDataGraph(const char *inserted_line = "");
    static std::string GetSpirvMultiEntryComputeAndDataGraph();
    static std::string GetSpirvMultiEntryTwoDataGraph();
    static std::string GetSpirvBasicShader();

    void InitPipelineResources(const std::vector<vkt::Tensor *> &tensors = {},
                               VkDescriptorType desc_type = VK_DESCRIPTOR_TYPE_TENSOR_ARM,
                               VkDescriptorSetLayoutCreateFlags layout_flags = 0);
    void CreatePipelineLayout(const std::vector<VkPushConstantRange> &push_constant_ranges = {});
    VkResult CreateDataGraphPipeline();
    const VkPipeline &Handle() const { return pipeline_; }

    // Helper function to create a simple test case
    // info_override can be any callable that takes a DataGraphPipelineHelper, error can be any args accepted by
    // "SetDesiredFailureMsg".
    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, const std::vector<Error> &errors) {
        DataGraphPipelineHelper helper(test);
        info_override(helper);
        // Allow lambda to decide if to skip trying to compile pipeline to prevent crashing
        for (const auto &error : errors) {
            test.Monitor().SetDesiredFailureMsg(flags, error);
        }
        helper.CreateDataGraphPipeline();

        if (!errors.empty()) {
            test.Monitor().VerifyFound();
        }
    }

    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags, Error error) {
        OneshotTest(test, info_override, flags, std::vector<Error>(1, error));
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotTest(Test &test, const OverrideFunc &info_override, const VkFlags flags) {
        OneshotTest(test, info_override, flags, std::vector<std::string>{});
    }

  private:
    void CreateShaderModule(const char *spirv_source, const char *entrypoint = "main");
    void InitTensor(vkt::Tensor &tensor, vkt::TensorView &tensor_view, const std::vector<int64_t> &tensor_dims, bool is_protected);

    VkPipeline pipeline_ = VK_NULL_HANDLE;
};

}  // namespace dg
}  // namespace vkt
