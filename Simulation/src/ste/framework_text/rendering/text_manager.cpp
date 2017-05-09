
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
	vert(context, std::string("text_distance_map_contour.vert")),
	geom(context, std::string("text_distance_map_contour.geom")),
	frag(context, std::string("text_distance_map_contour.frag")),
	pipeline(create_pipeline(context, vert, geom, frag))
{
	bind_pipeline_resources();
}

gl::device_pipeline_graphics text_manager::create_pipeline(const ste_context &ctx,
														   gl::device_pipeline_shader_stage &vert,
														   gl::device_pipeline_shader_stage &geom,
														   gl::device_pipeline_shader_stage &frag) {
	auto surface_extent = ctx.device().get_surface().extent();
	auto fb_layout = gl::framebuffer_layout(surface_extent);
	fb_layout[0] = gl::clear_store(ctx.device().get_surface().surface_format(),
								   gl::image_layout::present_src_khr);

	auto auditor = gl::pipeline_auditor_graphics(gl::device_pipeline_graphics_configurations(surface_extent));
	auditor.attach_shader_stage(vert);
	auditor.attach_shader_stage(geom);
	auditor.attach_shader_stage(frag);

	auditor.set_vertex_attributes(0, gl::vertex_attributes<glyph_point>());
	auditor.set_framebuffer_layout(std::move(fb_layout));

	return auditor.pipeline(ctx);
}

void text_manager::bind_pipeline_resources() {
	pipeline["glyph_sampler"] = gl::bind(gm.sampler());

	std::uint32_t texture_count = static_cast<std::uint32_t>(gm.textures().size());

	if (texture_count) {
		std::vector<std::pair<const gl::image_view_generic*, gl::image_layout>> images;
		images.reserve(texture_count);
		for (std::uint32_t i = 0; i < texture_count; ++i) {
			auto &glyph_texture = gm.textures()[i];
			images.push_back(std::make_pair(&glyph_texture.view, gl::image_layout::shader_read_only_optimal));
		}
		pipeline["glyph_textures"] = gl::bind(0, images);
	}
}

void text_manager::recreate_pipeline() {
	pipeline = create_pipeline(context.get(), vert, geom, frag);
	bind_pipeline_resources();
}

void text_manager::adjust_line(std::vector<glyph_point> &points, const attributed_wstring &wstr, unsigned line_start_index, float line_start, float line_height, const glm::vec2 &ortho_pos) {
	if (points.size() - line_start_index) {
		auto alignment_attrib = optional_dynamic_cast<const Attributes::align*>(
			wstr.attrib_of_type(Attributes::align::attrib_type_s(),
								range<>{ line_start_index, points.size() - line_start_index }));
		auto line_height_attrib = optional_dynamic_cast<const Attributes::line_height*>(
			wstr.attrib_of_type(Attributes::line_height::attrib_type_s(),
								range<>{ line_start_index, points.size() - line_start_index }));

		if (alignment_attrib && alignment_attrib->get() != Attributes::align::alignment::Left) {
			float line_len = ortho_pos.x - line_start;
			float offset = alignment_attrib->get() == Attributes::align::alignment::Center ? -line_len*.5f : -line_len;
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

std::vector<glyph_point> text_manager::create_points(glm::vec2 ortho_pos, const attributed_wstring &wstr) {
	float line_start = ortho_pos.x;
	int line_start_index = 0;
	float prev_line_height = 0;
	float line_height = 0;
	int num_lines = 1;

	std::vector<glyph_point> points;
	for (unsigned i = 0; i < wstr.length(); ++i) {
		if (wstr[i] == '\n') {
			adjust_line(points, wstr, line_start_index, line_start, prev_line_height, ortho_pos);

			ortho_pos.x = line_start;
			ortho_pos.y -= prev_line_height;
			prev_line_height = line_height;
			line_height = 0;
			line_start_index = points.size();

			++num_lines;

			continue;
		}

		auto font_attrib = optional_dynamic_cast<const Attributes::font*>(wstr.attrib_of_type(Attributes::font::attrib_type_s(), { i,1 }));
		auto color_attrib = optional_dynamic_cast<const Attributes::rgb*>(wstr.attrib_of_type(Attributes::rgb::attrib_type_s(), { i,1 }));
		auto size_attrib = optional_dynamic_cast<const Attributes::size*>(wstr.attrib_of_type(Attributes::size::attrib_type_s(), { i,1 }));
		auto stroke_attrib = optional_dynamic_cast<const Attributes::stroke*>(wstr.attrib_of_type(Attributes::stroke::attrib_type_s(), { i,1 }));
		auto weight_attrib = optional_dynamic_cast<const Attributes::weight*>(wstr.attrib_of_type(Attributes::weight::attrib_type_s(), { i,1 }));

		const font &font = font_attrib ? font_attrib->get() : default_font;
		int size = size_attrib ? size_attrib->get() : default_size;
		glm::u8vec4 color = color_attrib ? color_attrib->get() : glm::u8vec4{ 255, 255, 255, 255 };

		auto optional_g = gm.glyph_for_font(font, wstr[i]);
		if (!optional_g) {
			// No glyph found or loading
			continue;
		}

		const auto& g = optional_g.get();

		float f = static_cast<float>(size) / static_cast<float>(glyph::ttf_pixel_size);
		float w = weight_attrib ? weight_attrib->get() : 400.f;
		float lh = (g->metrics.height + g->metrics.start_y) * f * 2 + 1;

		float advance = static_cast<float>(i + 1 < wstr.length() ? gm.spacing(font, { wstr[i], wstr[i + 1] }, size) : 0);

		glm::u32vec2 pos = { static_cast<std::uint32_t>(ortho_pos.x + .5f), static_cast<std::uint32_t>(ortho_pos.y + .5f) };
		std::uint32_t osize = static_cast<std::uint32_t>(f * 2.f * glyph_point::size_scale + .5f);
		std::uint32_t glyph_index = static_cast<std::uint32_t>(g->buffer_index);
		float weight = glm::clamp<float>(w - 400, -300, 500) * f * .003f;

		glyph_point p;
		p.data().x = (pos.x & 0xFFFF) | (pos.y << 16);
		p.data().y = (osize & 0xFFFF) | (glyph_index << 16);
		p.data().z = glm::packUnorm4x8(glm::vec4(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f,
											   .5f + weight * glyph_point::weight_scale));
		p.data().w = 0;

		if (stroke_attrib) {
			float stroke_width = stroke_attrib->get_width();
			advance += glm::floor(stroke_width * .5f);

			auto c = stroke_attrib->get_color().get();
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

bool text_manager::update_glyphs(gl::command_recorder &recorder) {
	auto updated_range = gm.update_pending_glyphs(recorder);
	if (!updated_range.length)
		return false;

	std::uint32_t texture_count = static_cast<std::uint32_t>(gm.textures().size());

	pipeline["glyph_texture_count"] = texture_count;
	pipeline["glyph_data"] = gl::bind(gm.ssbo().get(), 0, texture_count);

	std::vector<std::pair<const gl::image_view_generic*, gl::image_layout>> images;
	images.reserve(updated_range.length);
	for (std::uint32_t i = updated_range.start; i < updated_range.start + updated_range.length; ++i) {
		auto &glyph_texture = gm.textures()[i];
		images.push_back(std::make_pair(&glyph_texture.view, gl::image_layout::shader_read_only_optimal));
	}
	pipeline["glyph_textures"] = gl::bind(updated_range.start, images);

	return true;
}

text_fragment text_manager::create_fragment() {
	return text_fragment(this);
}
