//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_logical_device.hpp>
#include <vk_descriptor_set.hpp>
#include <vk_descriptor_pool.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_unique_descriptor_set : public allow_type_decay<vk_unique_descriptor_set<host_allocator>, vk_descriptor_set<host_allocator>> {
private:
	using binding_t = vk_descriptor_set_layout_binding;
	using binding_set_t = lib::vector<vk_descriptor_set_layout_binding>;

private:
	vk_descriptor_pool<host_allocator> pool;
	vk_descriptor_set_layout<host_allocator> layout;
	vk_descriptor_set<host_allocator> set;

public:
	vk_unique_descriptor_set(const vk_logical_device<host_allocator> &device,
							 const binding_set_t &bindings)
		: pool(device, 1, bindings, false),
		layout(device, bindings),
		set(pool.allocate_descriptor_set(layout))
	{}

	~vk_unique_descriptor_set() noexcept {}

	vk_unique_descriptor_set(vk_unique_descriptor_set &&) = default;
	vk_unique_descriptor_set &operator=(vk_unique_descriptor_set &&) = default;
	vk_unique_descriptor_set(const vk_unique_descriptor_set &) = delete;
	vk_unique_descriptor_set &operator=(const vk_unique_descriptor_set &) = delete;

	auto& get() const { return set; }
	auto& get() { return set; }

	auto& get_layout() const { return layout; }
};

}

}
}
