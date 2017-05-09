//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <framebuffer_layout.hpp>
#include <framebuffer_attachment.hpp>
#include <framebuffer_bind_point.hpp>
#include <framebuffer_exceptions.hpp>

#include <vk_render_pass.hpp>
#include <vk_framebuffer.hpp>

#include <pipeline_layout_attachment_location.hpp>

#include <boost/container/flat_map.hpp>
#include <optional.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A pipeline framebuffer object
*/
class framebuffer : public allow_type_decay<framebuffer, vk::vk_framebuffer> {
public:
	using attachment_map_t = boost::container::flat_map<pipeline_layout_attachment_location, framebuffer_attachment>;

private:
	std::reference_wrapper<const ste_context> ctx;

	const framebuffer_layout *layout;
	attachment_map_t attachments;

	vk::vk_render_pass compatible_renderpass;
	optional<vk::vk_framebuffer> fb;
	optional<std::vector<VkClearValue>> clear_values;

private:
	void attach(pipeline_layout_attachment_location location, framebuffer_attachment &&attachment) {
		// Validate
		{
			auto layout_it = layout->find(location);
			if (layout_it == layout->end()) {
				// Location doesn't exist in layout
				throw framebuffer_invalid_attachment_location_exception("Attachment location doesn't exist in framebuffer layout");
			}

			if (layout_it->second.image_format != attachment.get_attachment().get_format()) {
				// Formats do not match
				throw framebuffer_attachment_format_mismatch_exception("Attachment format doesn't match layout's format");
			}

			if (attachment.has_explicit_clear_value() != layout_it->second.clears_on_load()) {
				// Attachment specifies a clear value, but attachment layout doesn't define a clear on load operation,
				// or the other way around.
				throw framebuffer_attachment_mismatch_exception("Mismatch between attachment clear value and layout load operation");
			}
		}

		bool invalidate_framebuffer = true;

		// Insert or overwrite attachment
		auto it = attachments.find(location);
		if (it != attachments.end()) {
			// Anything modified?
			if (it->second == attachment)
				return;

			// Was attachment modified?
			if (it->second.get_attachment().get_image_view_handle() == attachment.get_attachment().get_image_view_handle())
				invalidate_framebuffer = false;

			it->second = std::move(attachment);
		}
		else {
			attachments.insert(it, std::make_pair(location, std::move(attachment)));
		}

		if (!fb) {
			if (attachments.size() == layout->size()) {
				// Create framebuffer as early as possible the very first time we have enough attachments
				update();
			}
		}
		else {
			// Framebuffer was modified.
			// Recreate resources on next update
			if (invalidate_framebuffer)
				fb = none;
			clear_values = none;
		}
	}

	auto create_clear_values() {
		std::vector<VkClearValue> ret;
		for (auto &a : attachments) {
			ret.push_back(a.second.get_vk_clear_value());
		}


		return ret;
	}

	auto create_framebuffer() {
		std::vector<VkImageView> image_view_handles;
		for (auto &a : attachments) {
			image_view_handles.push_back(a.second.get_attachment().get_image_view_handle());
		}

		// Create Vulkan framebuffer
		return vk::vk_framebuffer(ctx.get().device(),
								  compatible_renderpass,
								  image_view_handles,
								  layout->extent());
	}

private:
	using bind_point_t = framebuffer_bind_point<framebuffer, &framebuffer::attach>;

	friend class bind_point_t;

public:
	framebuffer(const ste_context &ctx,
				const framebuffer_layout &layout)
		: ctx(ctx),
		layout(&layout), 
		compatible_renderpass(this->layout->create_compatible_renderpass(ctx))
	{}

	framebuffer(framebuffer&&) = default;
	framebuffer&operator=(framebuffer&&) = default;

	/**
	 *	@brief	Creates a bind point to the framebuffer.
	 *	
	 *	@param	location		Attachment location
	 */
	auto operator[](const pipeline_layout_attachment_location &location) {
		return bind_point_t(location, this);
	}

	/**
	 *	@brief	Updates the framebuffer. Should be called before using the framebuffer object.
	 */
	void update() {
		if (!fb) {
			// Recreate framebuffer
			fb = create_framebuffer();
		}
		if (!clear_values) {
			// Recreate clear values array
			clear_values = create_clear_values();
		}
	}

	/**
	 *	@brief	Returns the Vulkan framebuffer object
	 */
	const auto& get() const {
		assert(fb && "update() not called?");
		return fb.get();
	}

	/**
	*	@brief	Returns the framebuffer attachment clear values array
	*/
	const auto& get_fb_clearvalues() const {
		assert(fb && "clear_values() not called?");
		return clear_values.get();
	}

	auto &get_layout() const { return *layout; }

	auto begin() const { return attachments.begin(); }
	auto end() const { return attachments.end(); }

	auto size() const {
		return attachments.size();
	}
};

}
}
