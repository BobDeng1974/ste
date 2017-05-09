//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline.hpp>
#include <device_pipeline_exceptions.hpp>
#include <device_pipeline_graphics_configurations.hpp>

#include <framebuffer_layout.hpp>
#include <framebuffer.hpp>

#include <pipeline_vertex_input_bindings_collection.hpp>

#include <vk_render_pass.hpp>
#include <vk_pipeline_graphics.hpp>

#include <cmd_begin_render_pass.hpp>
#include <cmd_end_render_pass.hpp>
#include <cmd_bind_pipeline.hpp>

#include <optional.hpp>
#include <format_rtti.hpp>
#include <vector>

namespace ste {
namespace gl {

class device_pipeline_graphics : public device_pipeline {
	using Base = device_pipeline;

	friend class pipeline_auditor_graphics;

	struct ctor {};

private:
	device_pipeline_graphics_configurations pipeline_settings;
	pipeline_vertex_input_bindings_collection::pipeline_vertex_input_bindings_descriptor vertex_input_descriptor;
	framebuffer_layout fb_layout;

	optional<vk::vk_render_pass> device_renderpass;
	framebuffer* attached_framebuffer{ nullptr };
	optional<vk::vk_pipeline_graphics> graphics_pipeline;

private:
	void invalidate_pipeline() {
		graphics_pipeline = none;
	}

private:
	// Creates the Vulkan renderpass object
	void create_renderpass() {
		auto renderpass = fb_layout.create_compatible_renderpass(ctx.get());
		device_renderpass.emplace(std::move(renderpass));
	}

	// Creates the graphics pipeline object
	void create_pipeline() {
		if (!device_renderpass)
			create_renderpass();

		// Vulkan shader stage descriptors
		auto shader_stage_descriptors = get_layout().shader_stage_descriptors();

		// Blend operation descriptor for each attachment
		std::vector<vk::vk_blend_op_descriptor> attachment_blend_ops;
		for (auto &a : fb_layout) {
			const framebuffer_attachment_layout &attachment = a.second;

			// Blend operation is only applicable for color attachments
			bool is_depth_attachment = format_is_depth(attachment.image_format);
			if (is_depth_attachment)
				continue;

			attachment_blend_ops.push_back(attachment.blend_op);
		}

		// Create the graphics pipeline object
		VkViewport viewport = { 
			pipeline_settings.viewport.origin.x,
			pipeline_settings.viewport.origin.y,
			pipeline_settings.viewport.size.x,
			pipeline_settings.viewport.size.y,
			pipeline_settings.depth.min_depth,
			pipeline_settings.depth.max_depth,
		};
		VkRect2D scissor = {
			{ pipeline_settings.scissor.origin.x,pipeline_settings.scissor.origin.y },
			{ pipeline_settings.scissor.size.x,pipeline_settings.scissor.size.y },
		};
		graphics_pipeline.emplace(ctx.get().device(),
								  shader_stage_descriptors,
								  get_layout(),
								  device_renderpass.get(),
								  0,
								  viewport,
								  scissor,
								  vertex_input_descriptor.vertex_input_binding_descriptors,
								  vertex_input_descriptor.vertex_input_attribute_descriptors,
								  static_cast<VkPrimitiveTopology>(pipeline_settings.topology),
								  pipeline_settings.rasterizer_op,
								  pipeline_settings.depth_op,
								  attachment_blend_ops,
								  pipeline_settings.blend_constants,
								  &ctx.get().device().pipeline_cache().current_thread_cache());
	}

protected:
	VkPipelineBindPoint get_pipeline_vk_bind_point() const override final {
		return VK_PIPELINE_BIND_POINT_GRAPHICS;
	}

	void bind_pipeline(const command_buffer &, command_recorder &recorder) const override final {
		recorder << cmd_begin_render_pass(*attached_framebuffer,
										  device_renderpass.get(),
										  { 0,0 },
										  fb_layout.extent(),
										  attached_framebuffer->get_fb_clearvalues());
		recorder << cmd_bind_pipeline(graphics_pipeline.get());
	}
	
	void unbind_pipeline(const command_buffer &, command_recorder &recorder) const override final {
		recorder << cmd_end_render_pass();
	}

	void recreate_pipeline() override final {
		create_pipeline();
	}

	void update() override final {
		// Make sure we have framebuffer attached
		if (attached_framebuffer == nullptr) {
			throw device_pipeline_no_attached_framebuffer_exception("Attempting to bind graphics pipeline without attaching a framebuffer first");
		}
		// Update framebuffer
		attached_framebuffer->update();

		// Recreate invalidated pipeline object before binding, as needed
		if (!graphics_pipeline) {
			create_pipeline();
		}
	}

public:
	device_pipeline_graphics(ctor,
							 const ste_context &ctx,
							 const device_pipeline_graphics_configurations &graphics_pipeline_settings,
							 const pipeline_vertex_input_bindings_collection::pipeline_vertex_input_bindings_descriptor &vertex_input_descriptor,
							 const framebuffer_layout &fb_layout,
							 pipeline_layout &&layout,
							 optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets)
		: Base(ctx,
			   std::move(layout),
			   external_binding_sets),
		pipeline_settings(graphics_pipeline_settings),
		vertex_input_descriptor(vertex_input_descriptor),
		fb_layout(fb_layout)
	{
	}
	~device_pipeline_graphics() noexcept {}

	device_pipeline_graphics(device_pipeline_graphics&&) = default;
	device_pipeline_graphics &operator=(device_pipeline_graphics&&) = default;

	/**
	*	@brief	Attaches a framebuffer object.
	*			The new framebuffer object replaces any previously attached framebuffer objects, if any.
	*/
	auto attach_framebuffer(framebuffer &fb) {
		// Validate
		if (!fb_layout.compatible(fb.get_layout())) {
			throw device_pipeline_incompatible_framebuffer_exception("Attempting to attach a framebuffer object with incompatible layout");
		}

		return this->attached_framebuffer = &fb;
	}
};

}
}
