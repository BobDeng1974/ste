//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_host_allocator.hpp>
#include <vk_image.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <format_rtti.hpp>

#include <image_type.hpp>
#include <image_type_traits.hpp>
#include <image_view_swizzle.hpp>

#include <optional.hpp>
#include <limits>
#include <alias.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

namespace vk {

template <image_type type, typename host_allocator = vk_host_allocator<>>
class vk_image_view : public allow_type_decay<vk_image_view<type, host_allocator>, VkImageView> {
private:
	static constexpr std::uint32_t ctor_array_layers_multiplier = image_is_cubemap<type>::value ? 6 : 1;

	struct ctor {};

private:
	optional<VkImageView> view;
	alias<const vk_logical_device<host_allocator>> device;
	VkFormat image_format;

protected:
	vk_image_view(const vk::vk_image<host_allocator> &parent,
				  VkFormat image_format,
				  levels_t base_mip,
				  levels_t mips,
				  layers_t base_layer,
				  layers_t layers,
				  const image_view_swizzle &swizzle,
				  const ctor&)
		: device(parent.get_creating_device()), image_format(image_format)
	{
		VkImageView view;

		VkImageSubresourceRange range = {};
		range.baseArrayLayer = static_cast<std::uint32_t>(base_layer);
		range.layerCount = static_cast<std::uint32_t>(layers);
		range.baseMipLevel = static_cast<std::uint32_t>(base_mip);
		range.levelCount = static_cast<std::uint32_t>(mips);
		range.aspectMask = static_cast<VkImageAspectFlags>(format_aspect(static_cast<format>(image_format)));

		VkImageViewCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.image = parent;
		create_info.viewType = image_vk_type<type>::value;
		create_info.format = image_format;
		create_info.components = swizzle;
		create_info.subresourceRange = range;

		const vk_result res = vkCreateImageView(device.get(), &create_info, &host_allocator::allocation_callbacks(), &view);
		if (!res) {
			throw vk_exception(res);
		}

		this->view = view;
	}

public:
	/**
	*	@brief	Ctor for non-array image view.
	*
	*	@param parent		Parent image object
	*	@param image_format		View image_format
	*	@param base_layer	View base array layer. (Internally multiplied by six for cubemaps)
	*	@param base_mip		View base mipmap level
	*	@param mips			View mipmap levels. Use all_mip_levels for remaining levels.
	*	@param swizzle		View component swizzling
	*/
	template <bool sfinae = image_has_arrays<type>::value>
	vk_image_view(const vk::vk_image<host_allocator> &parent,
				  VkFormat image_format,
				  layers_t base_layer,
				  levels_t base_mip,
				  levels_t mips,
				  const image_view_swizzle &swizzle = image_view_swizzle(),
				  std::enable_if_t<!sfinae>* = nullptr)
		: vk_image_view(parent,
						image_format,
						base_mip,
						std::min(parent.get_mips() - base_mip, mips),
						base_layer * ctor_array_layers_multiplier,
						1_layer * ctor_array_layers_multiplier,
						swizzle,
						ctor()) {}
	/**
	*	@brief	Ctor for non-array image view.
	*
	*	@param parent		Parent image object
	*	@param image_format		View image_format
	*	@param base_layer	View base array layer. (Internally multiplied by six for cubemaps)
	*	@param mips			View mipmap levels. Use all_mip_levels for remaining levels.
	*	@param swizzle		View component swizzling
	*/
	template <bool sfinae = image_has_arrays<type>::value>
	vk_image_view(const vk::vk_image<host_allocator> &parent,
				  VkFormat image_format,
				  layers_t base_layer,
				  levels_t mips = all_mips,
				  const image_view_swizzle &swizzle = image_view_swizzle(),
				  std::enable_if_t<!sfinae>* = nullptr)
		: vk_image_view(parent,
						image_format,
						base_layer,
						0_mip,
						std::min(parent.get_mips(), mips),
						swizzle) {}
	/**
	*	@brief	Ctor for non-array image view.
	*
	*	@param parent		Parent image object
	*	@param image_format		View image_format
	*	@param swizzle		View component swizzling
	*/
	template <bool sfinae = image_has_arrays<type>::value>
	vk_image_view(const vk::vk_image<host_allocator> &parent,
				  VkFormat image_format,
				  const image_view_swizzle &swizzle = image_view_swizzle(),
				  std::enable_if_t<!sfinae>* = nullptr)
		: vk_image_view(parent,
						image_format,
						0_layer,
						all_mips,
						swizzle) {}

	/**
	*	@brief	Ctor for array image view.
	*
	*	@param parent		Parent image object
	*	@param image_format		View image_format
	*	@param base_layer	View base array layer. (Internally multiplied by six for cubemaps)
	*	@param base_mip		View base mipmap level
	*	@param layers		View array layers. (Internally multiplied by six for cubemaps)
	*	@param mips			View mipmap levels. Use all_mip_levels for remaining levels.
	*	@param swizzle		View component swizzling
	*/
	template <bool sfinae = image_has_arrays<type>::value>
	vk_image_view(const vk::vk_image<host_allocator> &parent,
				  VkFormat image_format,
				  layers_t base_layer,
				  levels_t base_mip,
				  layers_t layers,
				  levels_t mips = all_mips,
				  const image_view_swizzle &swizzle = image_view_swizzle(),
				  std::enable_if_t<sfinae>* = nullptr)
		: vk_image_view(parent,
						image_format,
						base_mip,
						std::min(parent.get_mips() - base_mip, mips),
						base_layer * ctor_array_layers_multiplier,
						layers * ctor_array_layers_multiplier,
						swizzle,
						ctor()) {}
	/**
	*	@brief	Ctor for array image view.
	*

	*	@param parent		Parent image object
	*	@param image_format		View image_format
	*	@param base_layer	View base array layer. (Internally multiplied by six for cubemaps)
	*	@param layers		View array layers. (Internally multiplied by six for cubemaps)
	*	@param swizzle		View component swizzling
	*/
	template <bool sfinae = image_has_arrays<type>::value>
	vk_image_view(const vk::vk_image<host_allocator> &parent,
				  VkFormat image_format,
				  layers_t base_layer,
				  layers_t layers,
				  const image_view_swizzle &swizzle = image_view_swizzle(),
				  std::enable_if_t<sfinae>* = nullptr)
		: vk_image_view(parent,
						image_format,
						base_layer,
						0_mip,
						layers,
						all_mips,
						swizzle) {}
	/**
	*	@brief	Ctor for array image view.
	*
	*	@param parent		Parent image object
	*	@param image_format		View image_format
	*	@param layers		View array layers. (Internally multiplied by six for cubemaps)
	*	@param swizzle		View component swizzling
	*/
	template <bool sfinae = image_has_arrays<type>::value>
	vk_image_view(const vk::vk_image<host_allocator> &parent,
				  VkFormat image_format,
				  layers_t layers,
				  const image_view_swizzle &swizzle = image_view_swizzle(),
				  std::enable_if_t<sfinae>* = nullptr)
		: vk_image_view(parent,
						image_format,
						0,
						layers,
						swizzle) {}

	/**
	*	@brief	Ctor for image view. Creates a view of the entire mipmap chain and array layers, with identical image_format to parent.
	*
	*	@param parent		Parent image object
	*	@param swizzle		View component swizzling
	*/
	vk_image_view(const vk::vk_image<host_allocator> &parent,
				  const image_view_swizzle &swizzle = image_view_swizzle())
		: vk_image_view(parent,
						parent.get_format(),
						0_mip,
						parent.get_mips(),
						0_layer,
						parent.get_layers(),
						swizzle,
						ctor()) {}

	~vk_image_view() noexcept { destroy_view(); }

	vk_image_view(vk_image_view &&) = default;
	vk_image_view& operator=(vk_image_view &&o) noexcept {
		destroy_view();

		view = std::move(o.view);
		device = std::move(o.device);
		image_format = o.image_format;

		return *this;
	}
	vk_image_view(const vk_image_view &) = delete;
	vk_image_view& operator=(const vk_image_view &) = delete;

	void destroy_view() {
		if (view) {
			vkDestroyImageView(device.get(), *this, &host_allocator::allocation_callbacks());
			view = none;
		}
	}

	auto& get() const { return view.get(); }
	auto& get_format() const { return image_format; }
};

}

}
}
