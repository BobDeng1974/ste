//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>

#include <pipeline_binding_layout_interface.hpp>
#include <binding_set_pool_instance.hpp>

#include <atomic>
#include <concurrent_unordered_map.hpp>
#include <boost/intrusive_ptr.hpp>

namespace ste {
namespace gl {

class pipeline_binding_set_pool {
private:
	using pool_key = std::uint32_t;
	using pool_ptr_t = boost::intrusive_ptr<binding_set_pool_instance>;
	using pools_t = concurrent_unordered_map<pool_key, pool_ptr_t>;

private:
	std::reference_wrapper<const vk::vk_logical_device> device;
	pools_t pools;
	std::atomic<pool_key> key_counter{ 0 };

private:
	template <typename Layout>
	auto allocate_pool_instance(const std::vector<const Layout*> &layouts) const {
		auto sets_count = layouts.size();
		std::vector<const pipeline_binding_layout_interface*> pool_bindings;
		pool_bindings.reserve(sets_count * 10);

		// Copy bindings
		for (auto &l : layouts) {
			auto &layout = *l;
			for (std::size_t i=0; i<layout.size(); ++i)
				pool_bindings.push_back(&layout[i]);
		}

		// Max sets
		auto max_sets = sets_count;

		return pool_ptr_t(new binding_set_pool_instance(binding_set_pool_instance::ctor(),
														device.get(),
														max_sets,
														pool_bindings));
	}

	void release_one(const pool_key &k) {
		pools.remove(k);
	}

public:
	pipeline_binding_set_pool(const vk::vk_logical_device &device)
		: device(device)
	{}
	~pipeline_binding_set_pool() noexcept {}

	pipeline_binding_set_pool(pipeline_binding_set_pool&&) = default;
	pipeline_binding_set_pool &operator=(pipeline_binding_set_pool&&) = default;

	/**
	*	@brief	Allocates the required layouts.
	*			Allocation is thread-safe. The returned sets are not thread-safe are assumed to exist on and accessed
	*			from a single thread only.
	*/
	template <typename Layout>
	auto allocate_binding_sets(const std::vector<const Layout*> &layouts) {
		// Create a key
		auto key = key_counter.fetch_add(1);

		// Create new slot
		auto instance = allocate_pool_instance<Layout>(layouts);
		instance->release_func = binding_set_pool_instance::release_func_t([this, key]() {
			this->release_one(key);
		});

		// Allocate
		auto sets = instance->allocate<Layout>(layouts);

		// Insert pool instance into pools
		pools.emplace(key, std::move(instance));

		return sets;
	}
};

}
}
