//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>

namespace ste {
namespace gl {

namespace vk {

class vk_push_constant_layout {
private:
	VkShaderStageFlags stage;
	byte_t size;
	byte_t offset;

public:
	vk_push_constant_layout(const VkShaderStageFlags &stage,
							byte_t size,
							byte_t offset = 0_B)
		: stage(stage), size(size), offset(offset)
	{}

	vk_push_constant_layout(vk_push_constant_layout &&) = default;
	vk_push_constant_layout &operator=(vk_push_constant_layout &&) = default;
	vk_push_constant_layout(const vk_push_constant_layout &) = default;
	vk_push_constant_layout &operator=(const vk_push_constant_layout &) = default;

	auto get_layout() const {
		VkPushConstantRange layout = {};
		layout.stageFlags = stage;
		layout.size = static_cast<std::uint32_t>(size);
		layout.offset = static_cast<std::uint32_t>(offset);

		return layout;
	}

	operator VkPushConstantRange() const { return get_layout(); }

	auto get_stage() const { return stage; }
	auto get_size() const { return size; }
	auto get_offset() const { return offset; }
};

}

}
}
