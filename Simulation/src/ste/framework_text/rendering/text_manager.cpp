
#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_pipeline_graphics_configurations.hpp>
#include <pipeline_auditor_graphics.hpp>
#include <framebuffer_layout.hpp>

#include <command_recorder.hpp>
#include <cmd_pipeline_barrier.hpp>
#include <pipeline_barrier.hpp>
#include <device_image_layout_transform.hpp>

#include <font.hpp>
#include <text_manager.hpp>
#include <glyph_point.hpp>
#include <text_fragment.hpp>

#include <optional.hpp>

using namespace ste;
using namespace text;

text_manager::text_manager(const ste_context &context,
						   const font &default_font,
						   int default_size)
	: context(context),
	gm(context),
	default_font(default_font),
	default_size(default_size),
	vert(context, lib::string("text_distance_map_contour.vert")),
	geom(context, lib::string("text_distance_map_contour.geom")),
	frag(context, lib::string("text_distance_map_contour.frag")),
	pipeline(create_pipeline(context,
							 *vert,
							 *geom,
							 *frag))
{
	bind_pipeline_resources(0);

	// Set framebuffer extent push constant, and keep it up to date.
	pipeline["push_constants_t.fb_size"] = glm::vec2(context.device().get_surface().extent());
	surface_recreate_signal_connection = make_connection(context.device().get_queues_and_surface_recreate_signal(), [this](const gl::ste_device*) {
		pipeline["push_constants_t.fb_size"] = glm::vec2(this->context.get().device().get_surface().extent());
	});
}

gl::device_pipeline_graphics text_manager::create_pipeline(const ste_context &ctx,
														   gl::device_pipeline_shader_stage &vert,
														   gl::device_pipeline_shader_stage &geom,
														   gl::device_pipeline_shader_stage &frag) {
	// Framebuffer layout
	auto fb_layout = gl::framebuffer_layout();
	fb_layout[0] = gl::load_store(ctx.device().get_surface().surface_format(),
								  gl::image_layout::color_attachment_optimal,
								  gl::image_layout::color_attachment_optimal,
								  gl::blend_operation(gl::blend_factor::src_alpha,
													  gl::blend_factor::one_minus_src_alpha,
													  gl::blend_op::add));

	// Create pipeline object
	gl::device_pipeline_graphics_configurations config{};
	config.topology = gl::primitive_topology::point_list;
	config.rasterizer_op = gl::rasterizer_operation(gl::cull_mode::none, gl::front_face::cw);

	auto auditor = gl::pipeline_auditor_graphics(std::move(config));
	auditor.attach_shader_stage(vert);
	auditor.attach_shader_stage(geom);
	auditor.attach_shader_stage(frag);

	auditor.set_vertex_attributes(0, gl::vertex_attributes<glyph_point>());
	auditor.set_framebuffer_layout(std::move(fb_layout));

	return auditor.pipeline(ctx,
							"text_manager pipeline");
}

/**
 *	@brief	Binds current available resources. For glyph_textures, starts binding from first_offset.
 */
void text_manager::bind_pipeline_resources(std::uint32_t first_offset) {
	pipeline["glyph_sampler"] = gl::bind(gm.sampler());

	{
		std::unique_lock<std::mutex> l(gm.mutex);

		std::uint32_t texture_count = static_cast<std::uint32_t>(gm.glyph_textures.size());
		if (texture_count) {
			pipeline["glyph_texture_count"] = texture_count;
			pipeline["glyph_data"] = gl::bind(gm.ssbo().get(), 0, texture_count);

			lib::vector<gl::pipeline::image> textures;
			textures.reserve(texture_count);
			for (std::uint32_t i = first_offset; i < texture_count; ++i) {
				auto &glyph_texture = gm.glyph_textures[i];
				textures.emplace_back(glyph_texture);
			}
			pipeline["glyph_textures"] = gl::bind(textures);
		}
	}
}

void text_manager::adjust_line(lib::vector<glyph_point> &points, const attributed_wstring &wstr, std::uint32_t line_start_index, float line_start, float line_height, const glm::vec2 &ortho_pos) {
	// Adjusts line height
	if (points.size() - line_start_index) {
		const auto alignment_attrib = attributes::align::bind(wstr.attrib_of_type(attributes::attrib_type::align, { line_start_index, static_cast<std::uint32_t>(points.size()) - line_start_index }));
		const auto line_height_attrib = attributes::line_height::bind(wstr.attrib_of_type(attributes::attrib_type::line_height, { line_start_index, static_cast<std::uint32_t>(points.size()) - line_start_index }));

		if (alignment_attrib && alignment_attrib->get() != attributes::align::alignment::Left) {
			const float line_len = ortho_pos.x - line_start;
			const float offset = alignment_attrib->get() == attributes::align::alignment::Center ? -line_len*.5f : -line_len;
			for (unsigned i = line_start_index; i < points.size(); ++i) {
				points[i].data().x += static_cast<std::uint16_t>(offset);
			}
		}

		if (line_height_attrib && line_height > 0)
			line_height = line_height_attrib->get();
	}
	for (unsigned i = line_start_index; i < points.size(); ++i) {
		*(reinterpret_cast<std::uint16_t*>(&points[i].data().x) + 1) -= static_cast<std::uint16_t>(line_height);
	}
}

/**
 *	@brief	Creates points vector from attributed string.
 */
lib::vector<glyph_point> text_manager::create_points(glm::vec2 ortho_pos, const attributed_wstring &wstr) {
	const float line_start = ortho_pos.x;
	std::uint32_t line_start_index = 0;
	float prev_line_height = 0;
	float line_height = 0;
	int num_lines = 1;

	lib::vector<glyph_point> points;
	for (unsigned i = 0; i < wstr.length(); ++i) {
		if (wstr[i] == '\n') {
			adjust_line(points, wstr, line_start_index, line_start, prev_line_height, ortho_pos);

			ortho_pos.x = line_start;
			ortho_pos.y -= prev_line_height;
			prev_line_height = line_height;
			line_height = 0;
			line_start_index = static_cast<std::uint32_t>(points.size());

			++num_lines;

			continue;
		}

		const auto font_attrib = attributes::font::bind(wstr.attrib_of_type(attributes::attrib_type::font, { i,1 }));
		const auto color_attrib = attributes::rgb::bind(wstr.attrib_of_type(attributes::attrib_type::color, { i,1 }));
		const auto size_attrib = attributes::size::bind(wstr.attrib_of_type(attributes::attrib_type::size, { i,1 }));
		const auto stroke_attrib = attributes::stroke::bind(wstr.attrib_of_type(attributes::attrib_type::stroke, { i,1 }));
		const auto weight_attrib = attributes::weight::bind(wstr.attrib_of_type(attributes::attrib_type::weight, { i,1 }));

		const font &font = font_attrib ? font_attrib->get() : default_font;
		const int size = size_attrib ? size_attrib->get() : default_size;
		const glm::u8vec4 color = color_attrib ? color_attrib->get() : glm::u8vec4{ 255, 255, 255, 255 };

		auto optional_g = gm.glyph_for_font(font, wstr[i]);
		if (!optional_g) {
			// No glyph found or loading
			continue;
		}

		const auto& g = optional_g.get();

		const float f = static_cast<float>(size) / static_cast<float>(glyph::ttf_pixel_size);
		const float w = weight_attrib ? weight_attrib->get() : 400.f;
		const float lh = (g->metrics.height + g->metrics.start_y) * f * 2 + 1;

		float advance = static_cast<float>(i + 1 < wstr.length() ? gm.spacing(font, { wstr[i], wstr[i + 1] }, size) : 0);

		const glm::u32vec2 pos = { static_cast<std::uint32_t>(ortho_pos.x + .5f), static_cast<std::uint32_t>(ortho_pos.y + .5f) };
		const std::uint32_t osize = static_cast<std::uint32_t>(f * 2.f * glyph_point::size_scale + .5f);
		const std::uint32_t glyph_index = static_cast<std::uint32_t>(g->buffer_index);
		const float weight = glm::clamp<float>(w - 400, -300, 500) * f * .003f;

		glyph_point p;
		p.data().x = (pos.x & 0xFFFF) | (pos.y << 16);
		p.data().y = (osize & 0xFFFF) | (glyph_index << 16);
		p.data().z = glm::packUnorm4x8(glm::vec4(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f,
											   .5f + weight * glyph_point::weight_scale));
		p.data().w = 0;

		if (stroke_attrib) {
			const float stroke_width = stroke_attrib->get_width();
			advance += glm::floor(stroke_width * .5f);

			const auto c = stroke_attrib->get_color();
			p.data().w = glm::packUnorm4x8(glm::vec4(c.x / 255.0f, c.y / 255.0f, c.z / 255.0f,
												   stroke_width * glyph_point::stroke_width_scale));
		}

		points.push_back(p);

		line_height = std::max(lh, line_height);
		ortho_pos.x += advance;
	}

	adjust_line(points, wstr, line_start_index, line_start, num_lines > 1 ? line_height : 0, ortho_pos);

	return points;
}

/**
 *	@brief	Updates resources bound to pipeline and records glyph buffer update commands.
 */
bool text_manager::update_glyphs(gl::command_recorder &recorder) {
	const auto updated_range = gm.update_pending_glyphs(recorder);
	if (!updated_range.length)
		return false;

	{
		std::unique_lock<std::mutex> l(gm.mutex);

		std::uint32_t texture_count = static_cast<std::uint32_t>(gm.glyph_textures.size());

		pipeline["glyph_texture_count"] = texture_count;
		pipeline["glyph_data"] = gl::bind(gm.ssbo().get(), 0, texture_count);

		lib::vector<gl::pipeline::image> textures;
		textures.reserve(static_cast<std::size_t>(updated_range.length));
		for (auto i = updated_range.start; i < updated_range.start + updated_range.length; ++i) {
			const auto tex_idx = static_cast<std::size_t>(i);
			auto &glyph_texture = gm.glyph_textures[tex_idx];
			textures.emplace_back(glyph_texture);
		}
		pipeline["glyph_textures"] = gl::bind(static_cast<std::uint32_t>(updated_range.start),
											  textures);
	}

	return true;
}

text_fragment text_manager::create_fragment() {
	return text_fragment(context, this);
}
