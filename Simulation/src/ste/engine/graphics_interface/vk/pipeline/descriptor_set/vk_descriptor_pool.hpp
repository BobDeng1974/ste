//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_ext_debug_marker.hpp>

#include <vk_descriptor_set_layout_binding.hpp>
#include <vk_descriptor_set.hpp>
#include <vk_descriptor_set_layout.hpp>
#include <vk_host_allocator.hpp>

#include <lib/vector.hpp>
#include <lib/unordered_map.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename host_allocator = vk_host_allocator<>>
class vk_descriptor_pool : public allow_type_decay<vk_descriptor_pool<host_allocator>, VkDescriptorPool> {
private:
	optional<VkDescriptorPool> pool;
	alias<const vk_logical_device<host_allocator>> device;
	bool allow_free_individual_sets;

public:
	vk_descriptor_pool(const vk_logical_device<host_allocator> &device,
					   std::uint32_t max_sets,
					   const lib::vector<vk_descriptor_set_layout_binding> &set_layout_bindings,
					   bool allow_free_individual_sets = false) : device(device), allow_free_individual_sets(allow_free_individual_sets) {
		lib::unordered_map<VkDescriptorType, std::uint32_t> type_counts;
		for (auto &l : set_layout_bindings) {
			auto it = type_counts.find(l.get_type());
			if (it != type_counts.end())
				it->second += l.get_count();
			else
				type_counts[l.get_type()] = l.get_count();
		}

		lib::vector<VkDescriptorPoolSize> set_sizes;
		set_sizes.reserve(type_counts.size());
		for (auto &p : type_counts) {
			VkDescriptorPoolSize size = {};
			size.type = p.first;
			size.descriptorCount = p.second;

			set_sizes.push_back(size);
		}

		VkDescriptorPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = allow_free_individual_sets ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;
		create_info.maxSets = max_sets;
		create_info.poolSizeCount = static_cast<std::uint32_t>(set_sizes.size());
		create_info.pPoolSizes = set_sizes.data();

		VkDescriptorPool pool;
		const vk_result res = vkCreateDescriptorPool(device, &create_info, &host_allocator::allocation_callbacks(), &pool);
		if (!res) {
			throw vk_exception(res);
		}

		this->pool = pool;
	}
	~vk_descriptor_pool() noexcept {
		destroy_pool();
	}

	vk_descriptor_pool(vk_descriptor_pool &&) = default;
	vk_descriptor_pool &operator=(vk_descriptor_pool &&o) noexcept {
		destroy_pool();

		pool = std::move(o.pool);
		device = std::move(o.device);
		allow_free_individual_sets = o.allow_free_individual_sets;

		return *this;
	}
	vk_descriptor_pool(const vk_descriptor_pool &) = delete;
	vk_descriptor_pool &operator=(const vk_descriptor_pool &) = delete;

	void destroy_pool() {
		if (pool) {
			vkDestroyDescriptorPool(device.get(), *this, &host_allocator::allocation_callbacks());
			pool = none;
		}
	}

	auto allocate_descriptor_sets(const lib::vector<const vk_descriptor_set_layout<host_allocator>*> &set_layouts) const {
		lib::vector<VkDescriptorSetLayout> set_layout_descriptors;
		set_layout_descriptors.reserve(set_layouts.size());
		for (auto &s : set_layouts)
			set_layout_descriptors.push_back(*s);

		VkDescriptorSetAllocateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		create_info.pNext = nullptr;
		create_info.descriptorPool = *this;
		create_info.descriptorSetCount = static_cast<std::uint32_t>(set_layout_descriptors.size());
		create_info.pSetLayouts = set_layout_descriptors.data();

		lib::vector<VkDescriptorSet> sets;
		sets.resize(create_info.descriptorSetCount);
		const vk_result res = vkAllocateDescriptorSets(device.get(), &create_info, sets.data());
		if (!res) {
			throw vk_exception(res);
		}

		lib::vector<vk_descriptor_set<host_allocator>> descriptor_sets;
		descriptor_sets.reserve(sets.size());
		for (int i=0; i<set_layouts.size(); ++i) {
			auto s = sets[i];
			const auto &l = *set_layouts[i];

			// Set object debug marker
			vk_debug_marker_set_object_name(device.get(),
											s,
											VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
											l.get_name());

			descriptor_sets.emplace_back(vk_descriptor_set<host_allocator>(device,
																		   s,
																		   *this,
																		   allows_freeing_individual_sets()));
		}

		return descriptor_sets;
	}

	vk_descriptor_set<host_allocator> allocate_descriptor_set(const vk_descriptor_set_layout<host_allocator> &layout) const {
		auto sets = allocate_descriptor_sets({ &layout });
		return std::move(sets.front());
	}

	void reset_pool() const {
		vk_result res = vkResetDescriptorPool(device.get(), *this, 0);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get() const { return pool.get(); }
	bool allows_freeing_individual_sets() const { return allow_free_individual_sets; }
};

}

}
}
