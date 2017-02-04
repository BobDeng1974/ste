// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "light.hpp"
#include "light_cascade_descriptor.hpp"

#include "directional_light.hpp"
#include "point_light.hpp"
#include "sphere_light.hpp"
#include "shaped_light.hpp"

#include "resource_storage_dynamic.hpp"
#include "gstack_stable.hpp"

#include "shader_storage_buffer.hpp"
#include "atomic_counter_buffer_object.hpp"

#include <array>
#include <memory>
#include <type_traits>

namespace StE {
namespace Graphics {

constexpr std::size_t max_active_lights_per_frame = 24;
constexpr std::size_t max_active_directional_lights_per_frame = 4;
constexpr std::size_t total_max_active_lights_per_frame = max_active_lights_per_frame + max_active_directional_lights_per_frame;

class light_storage : public Core::resource_storage_dynamic<light_descriptor> {
	using Base = Core::resource_storage_dynamic<light_descriptor>;

public:
	using shaped_light_point = shaped_light::shaped_light_point_type;

private:
	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr std::size_t pages = 1024;

	using lights_ll_type = Core::shader_storage_buffer<std::uint32_t, usage>;
	using directional_lights_cascades_storage_type = Core::gstack_stable<light_cascade_descriptor>;
	using shaped_lights_points_storage_type = Core::gstack_stable<shaped_light_point>;

private:
	Core::atomic_counter_buffer_object<> active_lights_ll_counter;

	lights_ll_type active_lights_ll;
	directional_lights_cascades_storage_type directional_lights_cascades_storage;
	shaped_lights_points_storage_type shaped_lights_points_storage;

	std::array<float, directional_light_cascades> cascades_depths;

private:
	void build_cascade_depth_array();

public:
	light_storage() : active_lights_ll_counter(1),
					  active_lights_ll(pages * std::max<std::size_t>(65536, lights_ll_type::page_size()) / 2) {
		build_cascade_depth_array();
	}

	template <typename ... Ts>
	auto allocate_point_light(Ts&&... args) {
		auto res = Base::allocate_resource<point_light>(std::forward<Ts>(args)...);
		active_lights_ll.commit_range(0, Base::size());

		return std::move(res);
	}

	template <typename ... Ts>
	auto allocate_sphere_light(Ts&&... args) {
		auto res = Base::allocate_resource<sphere_light>(std::forward<Ts>(args)...);
		active_lights_ll.commit_range(0, Base::size());

		return std::move(res);
	}

	template <typename ... Ts>
	auto allocate_directional_light(Ts&&... args) {
		auto res = Base::allocate_resource<directional_light>(std::forward<Ts>(args)...);
		active_lights_ll.commit_range(0, Base::size());

		// For directional lights we also need to allocate the cascade storage
		auto cascade_idx = directional_lights_cascades_storage.insert(light_cascade_descriptor());
		res->set_cascade_idx(cascade_idx);

		return std::move(res);
	}

	template <typename Light, typename ... Ts>
	auto allocate_shaped_light(Ts&&... args) {
		static_assert(std::is_base_of<shaped_light, Light>::value, "Light must be a shaped_light derived type");

		// For shaped light need to pass pointer to points storage
		shaped_light::shaped_light_points_storage_info info{ &shaped_lights_points_storage };

		auto res = Base::allocate_resource<Light>(std::forward<Ts>(args)..., info);
		active_lights_ll.commit_range(0, Base::size());

		return std::move(res);
	}

	void erase_light(const light *l) {
		auto cascade_idx = l->get_descriptor().cascade_idx;
		if (cascade_idx != 0xFFFFFFFF)
			directional_lights_cascades_storage.mark_tombstone(cascade_idx);

		erase_resource(l);
	}

	void clear_active_ll() {
		std::uint32_t zero = 0;
		active_lights_ll_counter.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
	}

	void bind_lights_buffer(int idx) const { Base::buffer().bind_range(Core::shader_storage_layout_binding(idx), 0, Base::size()); }
	auto& get_active_ll_counter() const { return active_lights_ll_counter; }
	auto& get_active_ll() const { return active_lights_ll; }

	auto& get_directional_lights_cascades_buffer() const { return directional_lights_cascades_storage.get_buffer(); }
	auto& get_shaped_lights_points_buffer() const { return shaped_lights_points_storage.get_buffer(); }

	auto& get_cascade_depths_array() const { return cascades_depths; }
};

}
}
