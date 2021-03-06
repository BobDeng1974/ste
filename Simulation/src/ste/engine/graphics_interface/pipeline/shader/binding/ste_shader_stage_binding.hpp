// StE
// � Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_variable_layout_validator.hpp>
#include <ste_shader_stage_binding_utility.hpp>

#include <ste_shader_stage_binding_type.hpp>
#include <pipeline_layout_set_index.hpp>

namespace ste {
namespace gl {

struct ste_shader_stage_binding : ste_shader_stage_variable_layout_validator {
	pipeline_layout_set_index set_idx;
	std::uint32_t bind_idx;

	ste_shader_stage_binding_type binding_type{ ste_shader_stage_binding_type::unknown };

	/**
	*	@brief	Returns the appropriate VkDescriptorType type.
	*
	*	@throws	ste_shader_binding_incompatible_type	If variable type isn't a block or image/combined_image_sampler/sampler
	*	@throws	ste_shader_binding_specialization_or_push_constant_exception	If binding_type isn't storage or uniform
	*/
	operator VkDescriptorType() const {
		return vk_descriptor_for_binding(binding_type,
										 variable->type());
	}

	/*
	*	@brief	Checks compatibility between a couple of stage bindings
	*/
	bool compatible(const ste_shader_stage_binding &b) const {
		return
			this->binding_type == b.binding_type &&
			this->block_layout == b.block_layout &&
			this->variable->compatible(*b.variable);
	}
};

}
}
