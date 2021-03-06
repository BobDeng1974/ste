//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vulkan/vulkan.h>
#include <vk_handle.hpp>
#include <vk_host_allocator.hpp>
#include <vk_logical_device.hpp>

#include <lib/vector.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

template <typename>
class vk_command_pool;

enum class vk_command_buffer_type : std::uint32_t {
	primary,
	secondary,
};

class vk_command_buffer : public allow_type_decay<vk_command_buffer, VkCommandBuffer> {
	template <typename A>
	friend class vk_command_pool;

private:
	VkCommandBuffer buffer{ vk_null_handle };

	vk_command_buffer() = default;
	vk_command_buffer(VkCommandBuffer b) : buffer(b) {}

public:
	void begin(const VkCommandBufferUsageFlags &flags = 0,
			   VkCommandBufferInheritanceInfo *inheritance = nullptr) {
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.pNext = nullptr;
		begin_info.pInheritanceInfo = inheritance;
		begin_info.flags = flags;

		const vk_result res = vkBeginCommandBuffer(*this, &begin_info);
		if (!res) {
			throw vk_exception(res);
		}
	}
	void end() {
		const vk_result res = vkEndCommandBuffer(*this);
		if (!res) {
			throw vk_exception(res);
		}
	}

	void reset() {
		const vk_result res = vkResetCommandBuffer(*this, 0);
		if (!res) {
			throw vk_exception(res);
		}
	}
	void reset_release() {
		const vk_result res = vkResetCommandBuffer(*this, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		if (!res) {
			throw vk_exception(res);
		}
	}

	auto& get() const { return buffer; }
};

template <typename host_allocator = vk_host_allocator<>>
class vk_command_buffers {
	friend vk_command_pool<host_allocator>;

private:
	lib::vector<vk_command_buffer> buffers;
	VkCommandPool pool;
	alias<const vk_logical_device<host_allocator>> device;
	vk_command_buffer_type type;

private:
	vk_command_buffers(lib::vector<vk_command_buffer> &&buffers,
					   const vk_logical_device<host_allocator> &device,
					   const VkCommandPool &pool,
					   const vk_command_buffer_type &type)
		: buffers(std::move(buffers)), pool(pool), device(device), type(type)
	{}

public:
	~vk_command_buffers() noexcept {
		free();
	}

	vk_command_buffers(vk_command_buffers &&) = default;
	vk_command_buffers &operator=(vk_command_buffers &&) = default;
	vk_command_buffers(const vk_command_buffers &) = delete;
	vk_command_buffers &operator=(const vk_command_buffers &) = delete;

	void free() {
		if (buffers.size() == 1) {
			vkFreeCommandBuffers(device.get(), pool, 1, &buffers[0].get());
			buffers.clear();
		}
		else if (buffers.size()) {
			lib::vector<VkCommandBuffer> b;
			b.reserve(buffers.size());
			for (auto &e : buffers)
				b.push_back(e.get());

			vkFreeCommandBuffers(device.get(),
								 pool,
								 static_cast<std::uint32_t>(b.size()),
								 b.data());
			buffers.clear();
		}
	}

	auto& operator[](std::size_t n) { return buffers[n]; }
	auto& operator[](std::size_t n) const { return buffers[n]; }

	auto get_type() const { return type; }
	auto size() const { return buffers.size(); }

	auto begin() const { return buffers.begin(); }
	auto begin() { return buffers.begin(); }
	auto end() const { return buffers.end(); }
	auto end() { return buffers.end(); }
};

}

}
}
