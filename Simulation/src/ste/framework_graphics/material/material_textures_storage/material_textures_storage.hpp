//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <material_texture.hpp>
#include <image_vector.hpp>

#include <surface.hpp>
#include <surface_factory.hpp>

namespace ste {
namespace graphics {

class material_textures_storage {
	using storage_t = gl::image_vector<gl::image_type::image_2d>;
	using texture_t = storage_t::texture_t;

	static constexpr auto blank_format = gl::format::r8_unorm;

private:
	storage_t storage;
	material_texture blank;

private:
	auto generate_blank_texture(const ste_context &ctx) {
		auto surface = resource::surface_2d<blank_format>({ 1, 1 }, 1_mips);
		surface[0_mip][0].r() = 0;

		auto image = resource::surface_factory::image_from_surface_2d<blank_format>(ctx,
																					std::move(surface),
																					gl::image_usage::sampled,
																					gl::image_layout::shader_read_only_optimal,
																					"material_textures_storage blank texture",
																					false);
		return gl::texture<gl::image_type::image_2d>(std::move(image).get());
	}

	auto generate_uninitialized_texture(const ste_context &ctx) {
		gl::device_image<2> image(ctx,
								  gl::image_initial_layout::unused,
								  gl::format::r32_sfloat,
								  { 1, 1 },
								  gl::image_usage::transfer_dst | gl::image_usage::sampled,
								  1_mips,
								  1_layers,
								  "material_textures_storage texture");

		return gl::texture<gl::image_type::image_2d>(std::move(image));
	}

public:
	material_textures_storage(const ste_context &ctx) {
		blank = allocate_texture(generate_blank_texture(ctx));
	}

	/**
	*	@brief	Allocates a new texture slot
	*/
	material_texture allocate_texture(texture_t &&texture) {
		auto slot = storage.allocate_slot(std::move(texture));
		return material_texture(std::move(slot));
	}

	/**
	*	@brief	Allocates a new texture slot with an uninitialized texture of format r32_sfloat
	*			Texture intended for dynamic updating via, e.g., cmd_clear_color_image.
	*/
	material_texture allocate_uninitialized_texture(const ste_context &ctx) {
		auto slot = storage.allocate_slot(generate_uninitialized_texture(ctx));
		return material_texture(std::move(slot));
	}

	/**
	 *	@brief	Returns a pipeline resource binder, which binds all the modified storage textures since last call.
	 */
	auto binder() const { return storage.binder(); }

	/*
	 *	@brief	Returns a preallocated "blank" texture, which is a 1x1 single-channel unorm image with its sole texel initialized to 0.
	 */
	auto &blank_texture() const { return blank; }

	auto size() const { return storage.size(); }
};

}
}
