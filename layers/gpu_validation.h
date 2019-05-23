/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 *
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Karl Schultz <karl@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 */

#pragma once
#include "vk_layer_logging.h"
#include "vulkan/vk_layer.h"
#include "vk_layer_data.h"
#include <SPIRV/spirv.hpp>
#include "hash_vk_types.h"
#include "vk_mem_alloc.h"
#include "layer_chassis_dispatch.h"
#include <map>

////////////////////////////////////////////////////////////////////#include "core_validation_types.h"
// Fwd declarations -- including descriptor_set.h creates an ugly include loop
namespace cvdescriptorset {
class DescriptorSetLayoutDef;
class DescriptorSetLayout;
class DescriptorSet;
}  // namespace cvdescriptorset

struct CMD_BUFFER_STATE;
class GpuVal;

class BASE_NODE {
   public:
    // Track command buffers that this object is bound to
    //  binding initialized when cmd referencing object is bound to command buffer
    //  binding removed when command buffer is reset or destroyed
    // When an object is destroyed, any bound cbs are set to INVALID
    std::unordered_set<CMD_BUFFER_STATE*> cb_bindings;

    BASE_NODE(){};
};

// Track command pools and their command buffers
struct COMMAND_POOL_STATE : public BASE_NODE {
    uint32_t queueFamilyIndex;
    std::unordered_set<VkCommandBuffer> commandBuffers;
};

// Generic wrapper for vulkan objects
struct VK_OBJECT {
    uint64_t handle;
    VulkanObjectType type;
};

// Flags describing requirements imposed by the pipeline on a descriptor
enum descriptor_req {
    DESCRIPTOR_REQ_VIEW_TYPE_1D = 1 << VK_IMAGE_VIEW_TYPE_1D,
    DESCRIPTOR_REQ_VIEW_TYPE_1D_ARRAY = 1 << VK_IMAGE_VIEW_TYPE_1D_ARRAY,
    DESCRIPTOR_REQ_VIEW_TYPE_2D = 1 << VK_IMAGE_VIEW_TYPE_2D,
    DESCRIPTOR_REQ_VIEW_TYPE_2D_ARRAY = 1 << VK_IMAGE_VIEW_TYPE_2D_ARRAY,
    DESCRIPTOR_REQ_VIEW_TYPE_3D = 1 << VK_IMAGE_VIEW_TYPE_3D,
    DESCRIPTOR_REQ_VIEW_TYPE_CUBE = 1 << VK_IMAGE_VIEW_TYPE_CUBE,
    DESCRIPTOR_REQ_VIEW_TYPE_CUBE_ARRAY = 1 << VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
    DESCRIPTOR_REQ_ALL_VIEW_TYPE_BITS = (1 << (VK_IMAGE_VIEW_TYPE_END_RANGE + 1)) - 1,
    DESCRIPTOR_REQ_SINGLE_SAMPLE = 2 << VK_IMAGE_VIEW_TYPE_END_RANGE,
    DESCRIPTOR_REQ_MULTI_SAMPLE = DESCRIPTOR_REQ_SINGLE_SAMPLE << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT = DESCRIPTOR_REQ_MULTI_SAMPLE << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_SINT = DESCRIPTOR_REQ_COMPONENT_TYPE_FLOAT << 1,
    DESCRIPTOR_REQ_COMPONENT_TYPE_UINT = DESCRIPTOR_REQ_COMPONENT_TYPE_SINT << 1,
};

struct DESCRIPTOR_POOL_STATE : BASE_NODE {
    VkDescriptorPool pool;
    std::unordered_set<cvdescriptorset::DescriptorSet*> sets;
    DESCRIPTOR_POOL_STATE(const VkDescriptorPool pool) : pool(pool) {}
};

struct RENDER_PASS_STATE : public BASE_NODE {
    VkRenderPass renderPass;

    // ColorAttachment Index for each color attachment for each subpass
    std::vector<std::vector<uint32_t>> subpasses;

    RENDER_PASS_STATE(VkRenderPassCreateInfo2KHR const* pCreateInfo) {
        subpasses.resize(pCreateInfo->subpassCount);
        for (uint32_t subpass = 0; subpass < subpasses.size(); subpass++) {
            subpasses[subpass].resize(pCreateInfo->pSubpasses[subpass].colorAttachmentCount);
            for (uint32_t attachment = 0; attachment < subpasses[subpass].size(); attachment++) {
                subpasses[subpass][attachment] = pCreateInfo->pSubpasses[subpass].pColorAttachments[attachment].attachment;
            }
        }
    }

    RENDER_PASS_STATE(VkRenderPassCreateInfo const* pCreateInfo) {
        subpasses.resize(pCreateInfo->subpassCount);
        for (uint32_t subpass = 0; subpass < subpasses.size(); subpass++) {
            subpasses[subpass].resize(pCreateInfo->pSubpasses[subpass].colorAttachmentCount);
            for (uint32_t attachment = 0; attachment < subpasses[subpass].size(); attachment++) {
                subpasses[subpass][attachment] = pCreateInfo->pSubpasses[subpass].pColorAttachments[attachment].attachment;
            }
        }
    }
};

// Canonical dictionary for PushConstantRanges
using PushConstantRangesDict = hash_util::Dictionary<PushConstantRanges>;
using PushConstantRangesId = PushConstantRangesDict::Id;

// Canonical dictionary for the pipeline layout's layout of descriptorsetlayouts
using DescriptorSetLayoutDef = cvdescriptorset::DescriptorSetLayoutDef;
using DescriptorSetLayoutId = std::shared_ptr<const DescriptorSetLayoutDef>;
using PipelineLayoutSetLayoutsDef = std::vector<DescriptorSetLayoutId>;
using PipelineLayoutSetLayoutsDict =
    hash_util::Dictionary<PipelineLayoutSetLayoutsDef, hash_util::IsOrderedContainer<PipelineLayoutSetLayoutsDef>>;
using PipelineLayoutSetLayoutsId = PipelineLayoutSetLayoutsDict::Id;

// Store layouts and pushconstants for PipelineLayout
struct PIPELINE_LAYOUT_STATE {
    VkPipelineLayout layout;
    std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> set_layouts;
    PushConstantRangesId push_constant_ranges;

    PIPELINE_LAYOUT_STATE() : layout(VK_NULL_HANDLE), set_layouts{}, push_constant_ranges{} {}

    void reset() {
        layout = VK_NULL_HANDLE;
        set_layouts.clear();
        push_constant_ranges.reset();
    }
};

class PIPELINE_STATE : public BASE_NODE {
   public:
    VkPipeline pipeline;
    safe_VkGraphicsPipelineCreateInfo graphicsPipelineCI;
    safe_VkComputePipelineCreateInfo computePipelineCI;
    safe_VkRayTracingPipelineCreateInfoNV raytracingPipelineCI;
    // Capture which slots (set#->bindings) are actually used by the shaders of this pipeline
    std::unordered_map<uint32_t, std::map<uint32_t, descriptor_req>> active_slots;
    // Vtx input info (if any)
    std::unordered_map<uint32_t, uint32_t> vertex_binding_to_index_map_;
    std::vector<VkPipelineColorBlendAttachmentState> attachments;
    bool blendConstantsEnabled;  // Blend constants enabled for any attachments
    PIPELINE_LAYOUT_STATE pipeline_layout;

    // Default constructor
    PIPELINE_STATE()
        : pipeline{},
          graphicsPipelineCI{},
          computePipelineCI{},
          raytracingPipelineCI{},
          active_slots(),
          vertex_binding_to_index_map_(),
          attachments(),
          blendConstantsEnabled(false),
          pipeline_layout() {}

    void reset() {
        VkGraphicsPipelineCreateInfo emptyGraphicsCI = {};
        graphicsPipelineCI.initialize(&emptyGraphicsCI, false, false);
        VkComputePipelineCreateInfo emptyComputeCI = {};
        computePipelineCI.initialize(&emptyComputeCI);
        VkRayTracingPipelineCreateInfoNV emptyRayTracingCI = {};
        raytracingPipelineCI.initialize(&emptyRayTracingCI);
    }

    void initGraphicsPipeline(const VkGraphicsPipelineCreateInfo* pCreateInfo, std::shared_ptr<RENDER_PASS_STATE>&& rpstate) {
        reset();
        bool uses_color_attachment = false;
        if (pCreateInfo->subpass < rpstate->subpasses.size()) {
            for (uint32_t i = 0; i < rpstate->subpasses[pCreateInfo->subpass].size(); ++i) {
                if (rpstate->subpasses[pCreateInfo->subpass][i] != VK_ATTACHMENT_UNUSED) {
                    uses_color_attachment = true;
                    break;
                }
            }
        }
        graphicsPipelineCI.initialize(pCreateInfo, uses_color_attachment, false);
        if (pCreateInfo->pDepthStencilState) {
            graphicsPipelineCI.pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(pCreateInfo->pDepthStencilState);
        }

        if (graphicsPipelineCI.pColorBlendState) {
            const auto pCBCI = graphicsPipelineCI.pColorBlendState;
            if (pCBCI->attachmentCount) {
                this->attachments = std::vector<VkPipelineColorBlendAttachmentState>(pCBCI->pAttachments,
                                                                                     pCBCI->pAttachments + pCBCI->attachmentCount);
            }
        }
    }

    void initComputePipeline(const VkComputePipelineCreateInfo* pCreateInfo) {
        reset();
        computePipelineCI.initialize(pCreateInfo);
    }

    void initRayTracingPipelineNV(const VkRayTracingPipelineCreateInfoNV* pCreateInfo) {
        reset();
        raytracingPipelineCI.initialize(pCreateInfo);
    }
};

// Track last states that are bound per pipeline bind point (Gfx & Compute)
struct LAST_BOUND_STATE {
    LAST_BOUND_STATE() { reset(); }  // must define default constructor for portability reasons
    PIPELINE_STATE* pipeline_state;
    VkPipelineLayout pipeline_layout;
    // Track each set that has been bound
    std::vector<cvdescriptorset::DescriptorSet*> boundDescriptorSets;
    std::unique_ptr<cvdescriptorset::DescriptorSet> push_descriptor_set;
    // One dynamic offset per dynamic descriptor bound to this CB
    std::vector<std::vector<uint32_t>> dynamicOffsets;

    void reset() {
        pipeline_state = nullptr;
        pipeline_layout = VK_NULL_HANDLE;
        boundDescriptorSets.clear();
        push_descriptor_set = nullptr;
        dynamicOffsets.clear();
    }
};

// Cmd Buffer Wrapper Struct - TODO : This desperately needs its own class
struct CMD_BUFFER_STATE : public BASE_NODE {
    VkCommandBuffer commandBuffer;
    VkDevice device;
    bool hasDrawCmd;
    // Currently storing "lastBound" objects on per-CB basis
    //  long-term may want to create caches of "lastBound" states and could have
    //  each individual CMD_NODE referencing its own "lastBound" state
    // Store last bound state for Gfx & Compute pipeline bind points
    std::map<uint32_t, LAST_BOUND_STATE> lastBound;

    RENDER_PASS_STATE* activeRenderPass;
    VkCommandBuffer primaryCommandBuffer;
    // If primary, the secondary command buffers we will call.
    // If secondary, the primary command buffers we will be called by.
    std::unordered_set<CMD_BUFFER_STATE*> linkedCommandBuffers;

    // Cache of current insert label...
    LoggingLabel debug_label;
};

struct SHADER_MODULE_STATE;
struct DeviceExtensions;

enum RenderPassCreateVersion { RENDER_PASS_VERSION_1 = 0, RENDER_PASS_VERSION_2 = 1 };

// GPU Validation
struct ShaderTracker {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    std::vector<unsigned int> pgm;
};

std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> const GetDescriptorSetLayout(GpuVal const*, VkDescriptorSetLayout);

/////////////////////////////////////////////////////////////////////#include "descriptor_sets.h"

class GpuVal;

// Descriptor Data structures
namespace cvdescriptorset {

// Utility structs/classes/types
// Index range for global indices below, end is exclusive, i.e. [start,end)
struct IndexRange {
    IndexRange(uint32_t start_in, uint32_t end_in) : start(start_in), end(end_in) {}
    IndexRange() = default;
    uint32_t start;
    uint32_t end;
};

// DescriptorSetLayoutDef/DescriptorSetLayout classes

class DescriptorSetLayoutDef {
   public:
    // Constructors and destructor
    DescriptorSetLayoutDef(const VkDescriptorSetLayoutCreateInfo* p_create_info);
    size_t hash() const;

    uint32_t GetTotalDescriptorCount() const { return descriptor_count_; };
    uint32_t GetDynamicDescriptorCount() const { return dynamic_descriptor_count_; };
    VkDescriptorSetLayoutCreateFlags GetCreateFlags() const { return flags_; }
    // For a given binding, return the number of descriptors in that binding and all successive bindings
    uint32_t GetBindingCount() const { return binding_count_; };
    // Non-empty binding numbers in order
    const std::set<uint32_t>& GetSortedBindingSet() const { return non_empty_bindings_; }
    // Return true if given binding is present in this layout
    bool HasBinding(const uint32_t binding) const { return binding_to_index_map_.count(binding) > 0; };
    uint32_t GetIndexFromBinding(uint32_t binding) const;
    // Various Get functions that can either be passed a binding#, which will be automatically translated into the appropriate
    // index, or the index# can be passed in directly
    uint32_t GetMaxBinding() const { return bindings_[bindings_.size() - 1].binding; }
    VkDescriptorSetLayoutBinding const* GetDescriptorSetLayoutBindingPtrFromIndex(const uint32_t) const;
    VkDescriptorSetLayoutBinding const* GetDescriptorSetLayoutBindingPtrFromBinding(uint32_t binding) const {
        return GetDescriptorSetLayoutBindingPtrFromIndex(GetIndexFromBinding(binding));
    }
    const std::vector<safe_VkDescriptorSetLayoutBinding>& GetBindings() const { return bindings_; }
    const std::vector<VkDescriptorBindingFlagsEXT>& GetBindingFlags() const { return binding_flags_; }
    uint32_t GetDescriptorCountFromIndex(const uint32_t) const;
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return GetDescriptorCountFromIndex(GetIndexFromBinding(binding));
    }
    VkDescriptorType GetTypeFromIndex(const uint32_t) const;
    VkDescriptorType GetTypeFromBinding(const uint32_t binding) const { return GetTypeFromIndex(GetIndexFromBinding(binding)); }
    VkShaderStageFlags GetStageFlagsFromIndex(const uint32_t) const;
    VkShaderStageFlags GetStageFlagsFromBinding(const uint32_t binding) const {
        return GetStageFlagsFromIndex(GetIndexFromBinding(binding));
    }
    VkDescriptorBindingFlagsEXT GetDescriptorBindingFlagsFromIndex(const uint32_t) const;
    VkDescriptorBindingFlagsEXT GetDescriptorBindingFlagsFromBinding(const uint32_t binding) const {
        return GetDescriptorBindingFlagsFromIndex(GetIndexFromBinding(binding));
    }
    uint32_t GetIndexFromGlobalIndex(const uint32_t global_index) const;
    VkSampler const* GetImmutableSamplerPtrFromIndex(const uint32_t) const;
    // For a given binding and array index, return the corresponding index into the dynamic offset array
    int32_t GetDynamicOffsetIndexFromBinding(uint32_t binding) const {
        auto dyn_off = binding_to_dynamic_array_idx_map_.find(binding);
        if (dyn_off == binding_to_dynamic_array_idx_map_.end()) {
            assert(0);  // Requesting dyn offset for invalid binding/array idx pair
            return -1;
        }
        return dyn_off->second;
    }
    // For a particular binding, get the global index range
    //  This call should be guarded by a call to "HasBinding(binding)" to verify that the given binding exists
    const IndexRange& GetGlobalIndexRangeFromBinding(const uint32_t) const;

    // Helper function to get the next valid binding for a descriptor
    uint32_t GetNextValidBinding(const uint32_t) const;
    bool IsPushDescriptor() const { return GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR; };

   private:
    // Only the first three data members are used for hash and equality checks, the other members are derived from them, and are
    // used to speed up the various lookups/queries/validations
    VkDescriptorSetLayoutCreateFlags flags_;
    std::vector<safe_VkDescriptorSetLayoutBinding> bindings_;
    std::vector<VkDescriptorBindingFlagsEXT> binding_flags_;

    // Convenience data structures for rapid lookup of various descriptor set layout properties
    std::set<uint32_t> non_empty_bindings_;  // Containing non-emtpy bindings in numerical order
    std::unordered_map<uint32_t, uint32_t> binding_to_index_map_;
    // The following map allows an non-iterative lookup of a binding from a global index...
    std::map<uint32_t, uint32_t> global_start_to_index_map_;  // The index corresponding for a starting global (descriptor) index
    std::unordered_map<uint32_t, IndexRange> binding_to_global_index_range_map_;  // range is exclusive of .end
    // For a given binding map to associated index in the dynamic offset array
    std::unordered_map<uint32_t, uint32_t> binding_to_dynamic_array_idx_map_;

    uint32_t binding_count_;     // # of bindings in this layout
    uint32_t descriptor_count_;  // total # descriptors in this layout
    uint32_t dynamic_descriptor_count_;
};

static inline bool operator==(const DescriptorSetLayoutDef& lhs, const DescriptorSetLayoutDef& rhs) {
    bool result = (lhs.GetCreateFlags() == rhs.GetCreateFlags()) && (lhs.GetBindings() == rhs.GetBindings()) &&
                  (lhs.GetBindingFlags() == rhs.GetBindingFlags());
    return result;
}

// Canonical dictionary of DSL definitions -- independent of device or handle
using DescriptorSetLayoutDict = hash_util::Dictionary<DescriptorSetLayoutDef, hash_util::HasHashMember<DescriptorSetLayoutDef>>;
using DescriptorSetLayoutId = DescriptorSetLayoutDict::Id;

class DescriptorSetLayout {
   public:
    // Constructors and destructor
    DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* p_create_info, const VkDescriptorSetLayout layout);
    bool HasBinding(const uint32_t binding) const { return layout_id_->HasBinding(binding); }
    // Straightforward Get functions
    VkDescriptorSetLayout GetDescriptorSetLayout() const { return layout_; };
    bool IsDestroyed() const { return layout_destroyed_; }
    void MarkDestroyed() { layout_destroyed_ = true; }
    const DescriptorSetLayoutDef* GetLayoutDef() const { return layout_id_.get(); }
    DescriptorSetLayoutId GetLayoutId() const { return layout_id_; }
    uint32_t GetTotalDescriptorCount() const { return layout_id_->GetTotalDescriptorCount(); };
    uint32_t GetDynamicDescriptorCount() const { return layout_id_->GetDynamicDescriptorCount(); };
    uint32_t GetBindingCount() const { return layout_id_->GetBindingCount(); };
    VkDescriptorSetLayoutCreateFlags GetCreateFlags() const { return layout_id_->GetCreateFlags(); }
    uint32_t GetIndexFromBinding(uint32_t binding) const { return layout_id_->GetIndexFromBinding(binding); }
    // Various Get functions that can either be passed a binding#, which will be automatically translated into the appropriate
    // index, or the index# can be passed in directly
    uint32_t GetMaxBinding() const { return layout_id_->GetMaxBinding(); }
    VkDescriptorSetLayoutBinding const* GetDescriptorSetLayoutBindingPtrFromIndex(const uint32_t index) const {
        return layout_id_->GetDescriptorSetLayoutBindingPtrFromIndex(index);
    }
    VkDescriptorSetLayoutBinding const* GetDescriptorSetLayoutBindingPtrFromBinding(uint32_t binding) const {
        return layout_id_->GetDescriptorSetLayoutBindingPtrFromBinding(binding);
    }
    const std::vector<safe_VkDescriptorSetLayoutBinding>& GetBindings() const { return layout_id_->GetBindings(); }
    const std::set<uint32_t>& GetSortedBindingSet() const { return layout_id_->GetSortedBindingSet(); }
    uint32_t GetDescriptorCountFromIndex(const uint32_t index) const { return layout_id_->GetDescriptorCountFromIndex(index); }
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return layout_id_->GetDescriptorCountFromBinding(binding);
    }
    VkDescriptorType GetTypeFromIndex(const uint32_t index) const { return layout_id_->GetTypeFromIndex(index); }
    VkDescriptorType GetTypeFromBinding(const uint32_t binding) const { return layout_id_->GetTypeFromBinding(binding); }
    VkShaderStageFlags GetStageFlagsFromIndex(const uint32_t index) const { return layout_id_->GetStageFlagsFromIndex(index); }
    VkShaderStageFlags GetStageFlagsFromBinding(const uint32_t binding) const {
        return layout_id_->GetStageFlagsFromBinding(binding);
    }
    VkDescriptorBindingFlagsEXT GetDescriptorBindingFlagsFromIndex(const uint32_t index) const {
        return layout_id_->GetDescriptorBindingFlagsFromIndex(index);
    }
    VkDescriptorBindingFlagsEXT GetDescriptorBindingFlagsFromBinding(const uint32_t binding) const {
        return layout_id_->GetDescriptorBindingFlagsFromBinding(binding);
    }
    uint32_t GetIndexFromGlobalIndex(const uint32_t global_index) const {
        return layout_id_->GetIndexFromGlobalIndex(global_index);
    }
    VkSampler const* GetImmutableSamplerPtrFromIndex(const uint32_t index) const {
        return layout_id_->GetImmutableSamplerPtrFromIndex(index);
    }
    // For a given binding and array index, return the corresponding index into the dynamic offset array
    int32_t GetDynamicOffsetIndexFromBinding(uint32_t binding) const {
        return layout_id_->GetDynamicOffsetIndexFromBinding(binding);
    }
    // For a particular binding, get the global index range
    //  This call should be guarded by a call to "HasBinding(binding)" to verify that the given binding exists
    const IndexRange& GetGlobalIndexRangeFromBinding(const uint32_t binding) const {
        return layout_id_->GetGlobalIndexRangeFromBinding(binding);
    }
    // Helper function to get the next valid binding for a descriptor
    uint32_t GetNextValidBinding(const uint32_t binding) const { return layout_id_->GetNextValidBinding(binding); }
    bool IsPushDescriptor() const { return layout_id_->IsPushDescriptor(); }

   private:
    VkDescriptorSetLayout layout_;
    bool layout_destroyed_;
    DescriptorSetLayoutId layout_id_;
};

// Slightly broader than type, each c++ "class" will has a corresponding "DescriptorClass"
enum DescriptorClass { PlainSampler, ImageSampler, Image, TexelBuffer, GeneralBuffer, InlineUniform, AccelerationStructure };

class Descriptor {
   public:
    Descriptor();
    virtual ~Descriptor(){};
    void WriteUpdate(const VkWriteDescriptorSet*, const uint32_t) { updated = true; };
    void CopyUpdate(const Descriptor*) { updated = true; };
    void BindCommandBuffer(CMD_BUFFER_STATE*);
    void UpdateDrawState(GpuVal*, CMD_BUFFER_STATE*);
    bool updated;  // Has descriptor been updated?
};

struct AllocateDescriptorSetsData {
    std::map<uint32_t, uint32_t> required_descriptors_by_type;
    std::vector<std::shared_ptr<DescriptorSetLayout const>> layout_nodes;
    AllocateDescriptorSetsData(uint32_t);
};

// DescriptorSet class
//
// Overview - This class encapsulates the Vulkan VkDescriptorSet data (set).
//  A set has an underlying layout which defines the bindings in the set and the types and numbers of descriptors in each descriptor
//  slot. Most of the layout interfaces are exposed through identically-named functions in the set class. Please refer to the
//  DescriptorSetLayout comment above for a description of index, binding, and global index.
// At construction a vector of Descriptor* is created with types corresponding to the layout. The primary operation performed on the
// descriptors is to update them via write or copy updates.
class DescriptorSet : public BASE_NODE {
   public:
    DescriptorSet(const VkDescriptorSet, const VkDescriptorPool, const std::shared_ptr<DescriptorSetLayout const>&,
                  uint32_t variable_count, GpuVal*);
    ~DescriptorSet();
    // A number of common Get* functions that return data based on layout from which this set was created
    uint32_t GetTotalDescriptorCount() const { return p_layout_->GetTotalDescriptorCount(); };
    uint32_t GetDynamicDescriptorCount() const { return p_layout_->GetDynamicDescriptorCount(); };
    uint32_t GetBindingCount() const { return p_layout_->GetBindingCount(); };
    VkDescriptorType GetTypeFromIndex(const uint32_t index) const { return p_layout_->GetTypeFromIndex(index); };
    VkDescriptorType GetTypeFromBinding(const uint32_t binding) const { return p_layout_->GetTypeFromBinding(binding); };
    uint32_t GetDescriptorCountFromIndex(const uint32_t index) const { return p_layout_->GetDescriptorCountFromIndex(index); };
    uint32_t GetDescriptorCountFromBinding(const uint32_t binding) const {
        return p_layout_->GetDescriptorCountFromBinding(binding);
    };
    // Return index into dynamic offset array for given binding
    int32_t GetDynamicOffsetIndexFromBinding(uint32_t binding) const {
        return p_layout_->GetDynamicOffsetIndexFromBinding(binding);
    }
    // Return true if given binding is present in this set
    bool HasBinding(const uint32_t binding) const { return p_layout_->HasBinding(binding); };

    const std::shared_ptr<DescriptorSetLayout const> GetLayout() const { return p_layout_; };
    VkDescriptorSet GetSet() const { return set_; };
    // Return unordered_set of all command buffers that this set is bound to
    std::unordered_set<CMD_BUFFER_STATE*> GetBoundCmdBuffers() const { return cb_bindings; }
    // Bind given cmd_buffer to this descriptor set and  update CB image layout map with image/imagesampler descriptor image layouts
    void UpdateDrawState(GpuVal*, CMD_BUFFER_STATE*, const std::map<uint32_t, descriptor_req>&);
    void BindCommandBuffer(CMD_BUFFER_STATE*, const std::map<uint32_t, descriptor_req>&);

    // If given cmd_buffer is in the cb_bindings set, remove it
    void RemoveBoundCommandBuffer(CMD_BUFFER_STATE* cb_node) { cb_bindings.erase(cb_node); }
    // For a particular binding, get the global index
    const IndexRange GetGlobalIndexRangeFromBinding(const uint32_t binding, bool actual_length = false) const {
        if (actual_length && binding == p_layout_->GetMaxBinding() && IsVariableDescriptorCount(binding)) {
            IndexRange range = p_layout_->GetGlobalIndexRangeFromBinding(binding);
            auto diff = GetDescriptorCountFromBinding(binding) - GetVariableDescriptorCount();
            range.end -= diff;
            return range;
        }
        return p_layout_->GetGlobalIndexRangeFromBinding(binding);
    };
    // Return true if any part of set has ever been updated
    bool IsPushDescriptor() const { return p_layout_->IsPushDescriptor(); };
    bool IsVariableDescriptorCount(uint32_t binding) const {
        return !!(p_layout_->GetDescriptorBindingFlagsFromBinding(binding) &
                  VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT);
    }
    bool IsUpdateAfterBind(uint32_t binding) const {
        return !!(p_layout_->GetDescriptorBindingFlagsFromBinding(binding) & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT);
    }
    uint32_t GetVariableDescriptorCount() const { return variable_count_; }
    DESCRIPTOR_POOL_STATE* GetPoolState() const { return pool_state_; }
    const Descriptor* GetDescriptorFromGlobalIndex(const uint32_t index) const { return descriptors_[index].get(); }
    void PerformWriteUpdate(const VkWriteDescriptorSet*);
    void PerformCopyUpdate(const VkCopyDescriptorSet*, const DescriptorSet*);

   private:
    VkDescriptorSet set_;
    DESCRIPTOR_POOL_STATE* pool_state_;
    const std::shared_ptr<DescriptorSetLayout const> p_layout_;
    std::vector<std::unique_ptr<Descriptor>> descriptors_;
    GpuVal* device_data_;
    const VkPhysicalDeviceLimits limits_;
    uint32_t variable_count_;
};

struct DecodedTemplateUpdate {
    std::vector<VkWriteDescriptorSet> desc_writes;
    std::vector<VkWriteDescriptorSetInlineUniformBlockEXT> inline_infos;
    DecodedTemplateUpdate(GpuVal* device_data, VkDescriptorSet descriptorSet, const TEMPLATE_STATE* template_state,
                          const void* pData, VkDescriptorSetLayout push_layout = VK_NULL_HANDLE);
};

void PerformUpdateDescriptorSets(GpuVal* dev_data, uint32_t write_count, const VkWriteDescriptorSet* p_wds, uint32_t copy_count,
                                 const VkCopyDescriptorSet* p_cds);
}  // namespace cvdescriptorset

////////////////////////////////////#include "gpu_validation.h"
struct GpuDeviceMemoryBlock {
    VkBuffer buffer;
    VmaAllocation allocation;
    std::unordered_map<uint32_t, const cvdescriptorset::Descriptor*> update_at_submit;
};

struct GpuBufferInfo {
    GpuDeviceMemoryBlock output_mem_block;
    GpuDeviceMemoryBlock input_mem_block;
    VkDescriptorSet desc_set;
    VkDescriptorPool desc_pool;
    GpuBufferInfo(GpuDeviceMemoryBlock output_mem_block, GpuDeviceMemoryBlock input_mem_block, VkDescriptorSet desc_set,
                  VkDescriptorPool desc_pool)
        : output_mem_block(output_mem_block), input_mem_block(input_mem_block), desc_set(desc_set), desc_pool(desc_pool){};
};

// Class to encapsulate Descriptor Set allocation.  This manager creates and destroys Descriptor Pools
// as needed to satisfy requests for descriptor sets.
class GpuDescriptorSetManager {
   public:
    GpuDescriptorSetManager(GpuVal* dev_data);
    ~GpuDescriptorSetManager();

    VkResult GetDescriptorSets(uint32_t count, VkDescriptorPool* pool, std::vector<VkDescriptorSet>* desc_sets);
    void PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set);

   private:
    static const uint32_t kItemsPerChunk = 512;
    struct PoolTracker {
        uint32_t size;
        uint32_t used;
    };

    GpuVal* dev_data_;
    std::unordered_map<VkDescriptorPool, struct PoolTracker> desc_pool_map_;
};

struct GpuValidationState {
    bool aborted;
    bool reserve_binding_slot;
    VkDescriptorSetLayout debug_desc_layout;
    VkDescriptorSetLayout dummy_desc_layout;
    uint32_t adjusted_max_desc_sets;
    uint32_t desc_set_bind_index;
    uint32_t unique_shader_module_id;
    std::unordered_map<uint32_t, ShaderTracker> shader_map;
    std::unique_ptr<GpuDescriptorSetManager> desc_set_manager;
    VkCommandPool barrier_command_pool;
    VkCommandBuffer barrier_command_buffer;
    std::unordered_map<VkCommandBuffer, std::vector<GpuBufferInfo>> command_buffer_map;  // gpu_buffer_list;
    uint32_t output_buffer_size;
    VmaAllocator vmaAllocator;
    GpuValidationState(bool aborted = false, bool reserve_binding_slot = false, VkCommandPool barrier_command_pool = VK_NULL_HANDLE,
                       VkCommandBuffer barrier_command_buffer = VK_NULL_HANDLE, VmaAllocator vmaAllocator = {})
        : aborted(aborted),
          reserve_binding_slot(reserve_binding_slot),
          barrier_command_pool(barrier_command_pool),
          barrier_command_buffer(barrier_command_buffer),
          vmaAllocator(vmaAllocator){};

    std::vector<GpuBufferInfo>& GetGpuBufferInfo(const VkCommandBuffer command_buffer) {
        auto buffer_list = command_buffer_map.find(command_buffer);
        if (buffer_list == command_buffer_map.end()) {
            std::vector<GpuBufferInfo> new_list{};
            command_buffer_map[command_buffer] = new_list;
            return command_buffer_map[command_buffer];
        }
        return buffer_list->second;
    }
};

using mutex_t = std::mutex;
using lock_guard_t = std::lock_guard<mutex_t>;
using unique_lock_t = std::unique_lock<mutex_t>;

///////////////////////////////////////// End of includes

// A forward iterator over spirv instructions. Provides easy access to len, opcode, and content words
// without the caller needing to care too much about the physical SPIRV module layout.
struct spirv_inst_iter {
    std::vector<uint32_t>::const_iterator zero;
    std::vector<uint32_t>::const_iterator it;

    uint32_t len() {
        auto result = *it >> 16;
        assert(result > 0);
        return result;
    }

    uint32_t opcode() { return *it & 0x0ffffu; }

    uint32_t const& word(unsigned n) {
        assert(n < len());
        return it[n];
    }

    uint32_t offset() { return (uint32_t)(it - zero); }

    spirv_inst_iter() {}

    spirv_inst_iter(std::vector<uint32_t>::const_iterator zero, std::vector<uint32_t>::const_iterator it) : zero(zero), it(it) {}

    bool operator==(spirv_inst_iter const& other) { return it == other.it; }

    bool operator!=(spirv_inst_iter const& other) { return it != other.it; }

    spirv_inst_iter operator++(int) {  // x++
        spirv_inst_iter ii = *this;
        it += len();
        return ii;
    }

    spirv_inst_iter operator++() {  // ++x;
        it += len();
        return *this;
    }

    // The iterator and the value are the same thing.
    spirv_inst_iter& operator*() { return *this; }
    spirv_inst_iter const& operator*() const { return *this; }
};

struct SHADER_MODULE_STATE {
    // The spirv image itself
    std::vector<uint32_t> words;
    std::unordered_map<unsigned, unsigned> def_index;
    bool has_valid_spirv;
    VkShaderModule vk_shader_module;
    uint32_t gpu_validation_shader_id;

    SHADER_MODULE_STATE(VkShaderModuleCreateInfo const* pCreateInfo, VkShaderModule shaderModule, uint32_t unique_shader_id)
        : words((uint32_t*)pCreateInfo->pCode, (uint32_t*)pCreateInfo->pCode + pCreateInfo->codeSize / sizeof(uint32_t)),
          has_valid_spirv(true),
          vk_shader_module(shaderModule),
          gpu_validation_shader_id(unique_shader_id) {
        BuildDefIndex();
    }

    SHADER_MODULE_STATE() : has_valid_spirv(false), vk_shader_module(VK_NULL_HANDLE) {}

    // Expose begin() / end() to enable range-based for
    spirv_inst_iter begin() const { return spirv_inst_iter(words.begin(), words.begin() + 5); }  // First insn
    spirv_inst_iter end() const { return spirv_inst_iter(words.begin(), words.end()); }          // Just past last insn
    // Given an offset into the module, produce an iterator there.
    spirv_inst_iter at(unsigned offset) const { return spirv_inst_iter(words.begin(), words.begin() + offset); }

    // Gets an iterator to the definition of an id
    spirv_inst_iter get_def(unsigned id) const {
        auto it = def_index.find(id);
        if (it == def_index.end()) {
            return end();
        }
        return at(it->second);
    }
    void BuildDefIndex();
};

struct PHYSICAL_DEVICE_STATE {
    VkPhysicalDevice phys_device = VK_NULL_HANDLE;
    uint32_t queue_family_count = 0;
    std::vector<VkQueueFamilyProperties> queue_family_properties;
};

// This structure is used to save data across the CreateGraphicsPipelines down-chain API call
struct create_graphics_pipeline_api_state {
    std::vector<safe_VkGraphicsPipelineCreateInfo> gpu_create_infos;
    std::vector<std::unique_ptr<PIPELINE_STATE>> pipe_state;
    const VkGraphicsPipelineCreateInfo* pCreateInfos;
};

// This structure is used to save data across the CreateComputePipelines down-chain API call
struct create_compute_pipeline_api_state {
    std::vector<safe_VkComputePipelineCreateInfo> gpu_create_infos;
    std::vector<std::unique_ptr<PIPELINE_STATE>> pipe_state;
    const VkComputePipelineCreateInfo* pCreateInfos;
};

// This structure is used modify parameters for the CreatePipelineLayout down-chain API call
struct create_pipeline_layout_api_state {
    std::vector<VkDescriptorSetLayout> new_layouts;
    VkPipelineLayoutCreateInfo modified_create_info;
};

// This structure is used modify and pass parameters for the CreateShaderModule down-chain API call

struct create_shader_module_api_state {
    uint32_t unique_shader_id;
    VkShaderModuleCreateInfo instrumented_create_info;
    std::vector<unsigned int> instrumented_pgm;
};

using std::unordered_map;
struct GpuValidationState;

class GpuVal : public ValidationObject {
   public:
    unordered_map<VkPipeline, std::unique_ptr<PIPELINE_STATE>> pipelineMap;
    unordered_map<VkShaderModule, std::unique_ptr<SHADER_MODULE_STATE>> shaderModuleMap;
    unordered_map<VkDescriptorUpdateTemplateKHR, std::unique_ptr<TEMPLATE_STATE>> desc_template_map;
    unordered_map<VkDescriptorPool, std::unique_ptr<DESCRIPTOR_POOL_STATE>> descriptorPoolMap;
    unordered_map<VkDescriptorSet, std::unique_ptr<cvdescriptorset::DescriptorSet>> setMap;
    unordered_map<VkCommandBuffer, std::unique_ptr<CMD_BUFFER_STATE>> commandBufferMap;
    unordered_map<VkCommandPool, std::unique_ptr<COMMAND_POOL_STATE>> commandPoolMap;
    unordered_map<VkPipelineLayout, std::unique_ptr<PIPELINE_LAYOUT_STATE>> pipelineLayoutMap;
    unordered_map<VkQueue, uint32_t> queueMap;
    unordered_map<VkRenderPass, std::shared_ptr<RENDER_PASS_STATE>> renderPassMap;
    unordered_map<VkDescriptorSetLayout, std::shared_ptr<cvdescriptorset::DescriptorSetLayout>> descriptorSetLayoutMap;

    // Used for instance versions of this object
    unordered_map<VkPhysicalDevice, PHYSICAL_DEVICE_STATE> physical_device_map;
    // Link to the device's physical-device data
    PHYSICAL_DEVICE_STATE* physical_device_state;

    // Link for derived device objects back to their parent instance object
    GpuVal* instance_state;

    // Device specific data
    VkPhysicalDeviceProperties phys_dev_props = {};
    std::unique_ptr<GpuValidationState> gpu_validation_state;

    // Class Declarations for helper functions
    cvdescriptorset::DescriptorSet* GetSetNode(VkDescriptorSet);
    DESCRIPTOR_POOL_STATE* GetDescriptorPoolState(const VkDescriptorPool);
    CMD_BUFFER_STATE* GetCBState(const VkCommandBuffer cb);
    PIPELINE_STATE* GetPipelineState(VkPipeline pipeline);
    RENDER_PASS_STATE* GetRenderPassState(VkRenderPass renderpass);
    std::shared_ptr<RENDER_PASS_STATE> GetRenderPassStateSharedPtr(VkRenderPass renderpass);
    COMMAND_POOL_STATE* GetCommandPoolState(VkCommandPool pool);
    SHADER_MODULE_STATE const* GetShaderModuleState(VkShaderModule module);
    uint32_t GetQueueState(VkQueue queue);
    PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState(VkPhysicalDevice phys);
    PHYSICAL_DEVICE_STATE* GetPhysicalDeviceState();

    void RecordPipelineShaderStage(VkPipelineShaderStageCreateInfo const* pStage, PIPELINE_STATE* pipeline,
                                   SHADER_MODULE_STATE const** out_module,
                                   spirv_inst_iter* out_entrypoint /*,bool check_point_size*/);
    void CapturePipelineShaderState(PIPELINE_STATE* pipeline);
    void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
    void PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                         VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);
    void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                      const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate,
                                                         const VkAllocationCallbacks* pAllocator);
    void RecordCreateDescriptorUpdateTemplateState(const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                   VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate);
    void PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate, VkResult result);
    void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                         const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorUpdateTemplateKHR* pDescriptorUpdateTemplate, VkResult result);

    void RecordComputePipeline(PIPELINE_STATE* pipeline);
    void RecordRayTracingPipelineNV(PIPELINE_STATE* pipeline);

    void PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet, const TEMPLATE_STATE* template_state,
                                                    const void* pData);
    void PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                           const VkCopyDescriptorSet* pDescriptorCopies);
    void RecordUpdateDescriptorSetWithTemplateState(VkDescriptorSet descriptorSet,
                                                    VkDescriptorUpdateTemplateKHR descriptorUpdateTemplate, const void* pData);

    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
    void RecordCreateRenderPassState(RenderPassCreateVersion rp_version, std::shared_ptr<RENDER_PASS_STATE>& render_pass,
                                     VkRenderPass* pRenderPass);
    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result);
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2KHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass, VkResult result);

    void UpdateStateCmdDrawDispatchType(CMD_BUFFER_STATE* cb_state, VkPipelineBindPoint bind_point);
    void UpdateStateCmdDrawType(CMD_BUFFER_STATE* cb_state, VkPipelineBindPoint bind_point);
    void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
    void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
    void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                               uint32_t firstInstance);
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                       uint32_t stride);
    void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride);
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride);
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride);
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride);
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride);
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                void* cgpl_state_data);
    void ResetCommandBufferState(const VkCommandBuffer cb);
    PIPELINE_LAYOUT_STATE const* GetPipelineLayout(VkPipelineLayout pipeLayout);
    void FreeCommandBufferStates(COMMAND_POOL_STATE* pool_state, const uint32_t command_buffer_count,
                                 const VkCommandBuffer* command_buffers);
    void FreeDescriptorSet(cvdescriptorset::DescriptorSet* descriptor_set);
    void DeletePools();
    void UpdateDrawState(CMD_BUFFER_STATE* cb_state, const VkPipelineBindPoint bind_point);
    void InitGpuValidation();
    void RecordCmdPushDescriptorSetState(CMD_BUFFER_STATE* cb_state, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                         uint32_t set, uint32_t descriptorWriteCount,
                                         const VkWriteDescriptorSet* pDescriptorWrites);
    void UpdateLastBoundDescriptorSets(CMD_BUFFER_STATE* cb_state, VkPipelineBindPoint pipeline_bind_point,
                                       const PIPELINE_LAYOUT_STATE* pipeline_layout, uint32_t first_set, uint32_t set_count,
                                       const std::vector<cvdescriptorset::DescriptorSet*> descriptor_sets,
                                       uint32_t dynamic_offset_count, const uint32_t* p_dynamic_offsets);

    void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                VkPhysicalDevice* pPhysicalDevices, VkResult result);

    // Descriptor Set Validation Functions
    void UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo*, cvdescriptorset::AllocateDescriptorSetsData*);
    void PerformAllocateDescriptorSets(const VkDescriptorSetAllocateInfo*, const VkDescriptorSet*,
                                       const cvdescriptorset::AllocateDescriptorSetsData*);

    // Stuff from shader_validation
    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, void* csm_state);
    void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, VkResult result,
                                          void* csm_state);

    // Gpu Validation Functions
    void GpuPostCallRecordCreateDevice(const CHECK_ENABLED* enables);
    void GpuResetCommandBuffer(const VkCommandBuffer commandBuffer);
    void GpuAllocateValidationResources(const VkCommandBuffer cmd_buffer, VkPipelineBindPoint bind_point);
    void AnalyzeAndReportError(CMD_BUFFER_STATE* cb_node, VkQueue queue, uint32_t draw_index, uint32_t* const debug_output_buffer);
    void ProcessInstrumentationBuffer(VkQueue queue, CMD_BUFFER_STATE* cb_node);
    void UpdateInstrumentationBuffer(CMD_BUFFER_STATE* cb_node);
    void SubmitBarrier(VkQueue queue);
    bool GpuInstrumentShader(const VkShaderModuleCreateInfo* pCreateInfo, std::vector<unsigned int>& new_pgm,
                             uint32_t* unique_shader_id);
    void GpuPreCallRecordPipelineCreations(uint32_t count, const VkGraphicsPipelineCreateInfo* pGraphicsCreateInfos,
                                           const VkComputePipelineCreateInfo* pComputeCreateInfos,
                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                           std::vector<std::unique_ptr<PIPELINE_STATE>>& pipe_state,
                                           std::vector<safe_VkGraphicsPipelineCreateInfo>* new_graphics_pipeline_create_infos,
                                           std::vector<safe_VkComputePipelineCreateInfo>* new_compute_pipeline_create_infos,
                                           const VkPipelineBindPoint bind_point);
    void GpuPostCallRecordPipelineCreations(const uint32_t count, const VkGraphicsPipelineCreateInfo* pGraphicsCreateInfos,
                                            const VkComputePipelineCreateInfo* pComputeCreateInfos,
                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                            const VkPipelineBindPoint bind_point);
    VkResult GpuInitializeVma();
    void ReportSetupProblem(VkDebugReportObjectTypeEXT object_type, uint64_t object_handle, const char* const specific_message);

    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* cgpl_state);
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                               void* cgpl_state);
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               void* ccpl_state_data);
    void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                             const VkComputePipelineCreateInfo* pCreateInfos,
                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                             void* ccpl_state_data);
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                              void* ccpl_state_data);
    void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                           void* cpl_state);
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                            VkResult result);
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, void* ads_state);
    void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                              VkDescriptorSet* pDescriptorSets, VkResult result, void* ads_state);
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, VkResult result,
                                                   void* pipe_state);
    void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                      VkInstance* pInstance, VkResult result);
    void PreCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                   std::unique_ptr<safe_VkDeviceCreateInfo>& modified_create_info);
    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, VkResult result);
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   VkResult result);
    void PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                 const VkAllocationCallbacks* pAllocator);
    void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                            const VkAllocationCallbacks* pAllocator);
    void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                         const VkCommandBuffer* pCommandBuffers);
    void PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                                 VkResult result);
    void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pCreateInfo,
                                              VkCommandBuffer* pCommandBuffer, VkResult result);
    void PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags, VkResult result);
    void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
    void PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                            const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                            const uint32_t* pDynamicOffsets);
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites);
    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
    void PreCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                  VkPhysicalDeviceProperties* pPhysicalDeviceProperties);
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance);
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                      uint32_t stride);
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride);
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);

};  // Class GpuVal
