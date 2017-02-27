//	StE
// � Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <vk_image_type.hpp>

namespace StE {
namespace GL {

template <int dimensions>
class vk_image {
public:
	using size_type = typename vk_image_extent_type<dimensions>::extent_type;

private:
	VkImage image{ VK_NULL_HANDLE };
	const vk_logical_device &device;
	VkFormat format;
	size_type size;
	std::uint32_t mips;
	std::uint32_t layers;
	VkImageUsageFlags usage;
	bool sparse;

private:
	static auto vk_type() {
		switch (dimensions) {
		default:
			assert(false);
		case 1:
			return VK_IMAGE_TYPE_1D;
		case 2:
			return VK_IMAGE_TYPE_2D;
		case 3:
			return VK_IMAGE_TYPE_3D;
		}
	}

public:
	vk_image(const vk_logical_device &device,
			 const VkFormat &format,
			 const size_type &size,
			 std::uint32_t mips,
			 std::uint32_t layers,
			 const VkImageUsageFlags &usage,
			 const VkImageLayout &initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
			 bool sparse = false)
		: device(device), format(format), size(size), mips(mips), layers(layers), usage(usage), sparse(sparse)
	{
		assert(mips > 0 && "Non-positive mipmap level count");
		assert(layers > 0 && "Non-positive array layers count");

		VkImage image;

		glm::u32vec3 extent(0);
		for (int i = 0; i < dimensions; ++i)
			extent[i] = size[i];

		auto flags = sparse ?
			VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT :
			0;
		flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VkImageCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = flags;
		create_info.imageType = vk_type();
		create_info.format = format;
		create_info.extent = { extent.x, extent.y, extent.z };
		create_info.mipLevels = mips;
		create_info.arrayLayers = layers;
		create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.usage = usage;
		create_info.initialLayout = initial_layout;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;

		vk_result res = vkCreateImage(device, &create_info, nullptr, &image);
		if (!res) {
			throw vk_exception(res);
		}

		this->image = image;
	}
	~vk_image() noexcept { destroy_image(); }

	vk_image(vk_image &&) = default;
	vk_image& operator=(vk_image &&) = default;
	vk_image(const vk_image &) = delete;
	vk_image& operator=(const vk_image &) = delete;

	void destroy_image() {
		if (image != VK_NULL_HANDLE) {
			vkDestroyImage(device, image, nullptr);
			image = VK_NULL_HANDLE;
		}
	}

	auto& get_image() const { return image; }

	auto& get_creating_device() const { return device; }
	auto& get_format() const { return format; };
	auto& get_size() const{ return size; };
	auto& get_mips() const{ return mips; };
	auto& get_layers() const{ return layers; };
	auto& get_usage() const{ return usage; };
	auto is_sparse() const{ return sparse; };

	operator VkImage() const { return get_image(); }
};

}
}
