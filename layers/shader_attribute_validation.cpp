#include "shader_attribute_validation.h"

#include "range_vector.h"

ShaderAttributeValidation::ShaderAttributeValidation(
    const SHADER_MODULE_STATE &shader_module, const std::map<location_t, interface_var>::const_iterator &shader_attributes_iterator,
    const std::map<location_t, interface_var>::const_iterator &shader_attributes_iterator_end)
    : is_valid_(shader_attributes_iterator != shader_attributes_iterator_end) {
    if (!is_valid_) return;

    start_location_ = shader_attributes_iterator->first.first;
    start_component_ = shader_attributes_iterator->first.second;
    current_global_component_ = start_location_ * 4 + start_component_;
    const auto def = shader_module.FindDef(shader_attributes_iterator->second.type_id);
    const auto base_type_components_count = shader_module.GetNumComponentsInBaseType(def);
    const auto total_byte_size = shader_module.GetTypeBytesSize(def);
    const auto array_size = shader_module.GetArraySize(def);  // Will be 1 if not an array
    assert(array_size > 0);
    // a 32 bit or less component count as 1, a 64 bit component as 2.
    if (total_byte_size == 2 * base_type_components_count * 4 * array_size) {
        components_count_ = base_type_components_count * 2;
    } else {
        components_count_ = base_type_components_count;
    }
    assert(components_count_ > 0);
    assert(components_count_ <= static_cast<uint32_t>(components_.size()));
    for (uint32_t component_i = 0; component_i < components_count_; ++component_i) {
        components_[component_i] = ComponentStatus::Unseen;
    }
    current_components_left_ = components_count_;
    interface_ = &shader_attributes_iterator->second;
}

void ShaderAttributeValidation::TagMatchingComponentsAsSeen(ShaderAttributeValidation &lhs, ShaderAttributeValidation &rhs) {
    const sparse_container::range<uint32_t> lhs_range(lhs.current_global_component_,
                                                      lhs.current_global_component_ + lhs.current_components_left_);
    const sparse_container::range<uint32_t> rhs_range(rhs.current_global_component_,
                                                      rhs.current_global_component_ + rhs.current_components_left_);
    const sparse_container::range<uint32_t> range_intersection = lhs_range & rhs_range;
    if (range_intersection.invalid() || range_intersection.empty()) return;

    {
        const uint32_t lhs_global_start_component = lhs.start_location_ * 4 + lhs.start_component_;
        const auto lhs_component_end = range_intersection.end - lhs_global_start_component;
        for (auto lhs_component_i = range_intersection.begin - lhs_global_start_component; lhs_component_i < lhs_component_end;
             ++lhs_component_i) {
            lhs.components_[lhs_component_i] = ComponentStatus::Seen;
        }
        assert(lhs.current_global_component_ < range_intersection.end);
        lhs.current_global_component_ = range_intersection.end;
        assert((lhs.start_location_ * 4 + lhs.start_component_ + lhs.components_count_) >= lhs.current_global_component_);
        lhs.current_components_left_ =
            (lhs.start_location_ * 4 + lhs.start_component_ + lhs.components_count_) - lhs.current_global_component_;
    }
    {
        const uint32_t rhs_global_start_component = rhs.start_location_ * 4 + rhs.start_component_;
        const auto rhs_component_end = range_intersection.end - rhs_global_start_component;
        for (auto rhs_component_i = range_intersection.begin - rhs_global_start_component; rhs_component_i < rhs_component_end;
             ++rhs_component_i) {
            rhs.components_[rhs_component_i] = ComponentStatus::Seen;
        }
        assert(rhs.current_global_component_ < range_intersection.end);
        rhs.current_global_component_ = range_intersection.end;
        assert((rhs.start_location_ * 4 + rhs.start_component_ + rhs.components_count_) >= rhs.current_global_component_);
        rhs.current_components_left_ =
            (rhs.start_location_ * 4 + rhs.start_component_ + rhs.components_count_) - rhs.current_global_component_;
    }
}