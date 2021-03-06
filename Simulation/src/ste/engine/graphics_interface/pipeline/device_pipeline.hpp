//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <pipeline_layout.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_bind_point.hpp>
#include <pipeline_push_constant_bind_point.hpp>
#include <pipeline_specialization_constant_bind_point.hpp>
#include <pipeline_resource_bind_point.hpp>
#include <device_pipeline_exceptions.hpp>

#include <device_pipeline_resources_marked_for_deletion.hpp>

#include <push_constant_path.hpp>
#include <pipeline_binding_set_collection.hpp>

#include <command_buffer.hpp>
#include <command_recorder.hpp>

#include <optional.hpp>
#include <lib/unique_ptr.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

class device_pipeline {
private:
	// Bind command
	class device_pipeline_cmd_bind : public command {
		const device_pipeline *pipeline;

	public:
		device_pipeline_cmd_bind(const device_pipeline *pipeline)
			: pipeline(pipeline)
		{}
		virtual ~device_pipeline_cmd_bind() noexcept {}

	private:
		void operator()(const command_buffer &buffer, command_recorder &recorder) && override final {
			// Bind pipeline's binding sets
			recorder << pipeline->binding_sets.cmd_bind(pipeline->get_pipeline_vk_bind_point(),
														0);
			// Bind external binding sets
			if (pipeline->external_binding_set != nullptr) {
				auto external_set_base_binding_index = static_cast<std::uint32_t>(pipeline->binding_sets.get_sets().size());
				recorder << pipeline->external_binding_set->cmd_bind(pipeline->get_pipeline_vk_bind_point(),
																	 &pipeline->layout->get(),
																	 external_set_base_binding_index);
			}

			// Bind pipeline
			pipeline->bind_pipeline(buffer, recorder);

			// Push constants
			recorder << pipeline->layout->cmd_push_constants();
		}
	};

	// Unbind command
	class device_pipeline_cmd_unbind : public command {
		const device_pipeline *pipeline;

	public:
		device_pipeline_cmd_unbind(const device_pipeline *pipeline)
			: pipeline(pipeline)
		{}
		virtual ~device_pipeline_cmd_unbind() noexcept {}

	private:
		void operator()(const command_buffer &buffer, command_recorder &recorder) && override final {
			pipeline->unbind_pipeline(buffer, recorder);
		}
	};

protected:
	alias<const ste_context> ctx;

	lib::unique_ptr<pipeline_layout> layout;
	pipeline_resource_binding_queue binding_queue;

	pipeline_binding_set_collection binding_sets;

	// External binding sets
	const pipeline_external_binding_set *external_binding_set;

	pipeline_layout::set_layout_modified_signal_t::connection_type set_modified_connection;

private:
	void prebind_update() {
		// Sanity check. Make sure the external binding set was updated. This is the external binding set's owner responsibility.
		if (external_binding_set &&
			(external_binding_set->has_pending_writes() || external_binding_set->is_invalidated())) {
			throw pipeline_layout_exception("External binding set not updated and/or has pending writes");
		}

		// Store old resources, and delete them in a controlled manner without disrupting current submitted commands.
		device_pipeline_resources_marked_for_deletion old_resources;

		// Update sets, as needed
		auto recreate_indices = layout->read_and_clear_modified_sets_queue();
		if (recreate_indices.size()) {
			// Recreate set layouts
			for (auto &set_idx : recreate_indices) {
				auto old_set_layout = layout->recreate_set_layout(set_idx);
				old_resources.binding_set_layouts.push_back(std::move(old_set_layout));
			}

			// Recreate sets
			old_resources.binding_sets = binding_sets.recreate_sets(ctx.get().device(), 
																	recreate_indices);
		}

		// Recreate pipeline if pipeline layout was invalidated for any reason
		if (layout->is_layout_invalidated()) {
			old_resources.pipeline_layout = layout->recreate_layout();
			old_resources.pipeline = recreate_pipeline();
		}

		// Write resource descriptors to binding sets from the binding queue and clear it
		if (!binding_queue.empty()) {
			binding_sets.write(binding_queue);
			binding_queue.clear();
		}

		// Call overloadable update
		update();

		if (old_resources) {
			// If we have any old resources, dispose of them
			ctx.get().device().pipeline_disposer().queue_deletion(std::move(old_resources));
		}
	}

protected:
	/**
	 *	@brief	Update is called just before binding the pipeline.
	 *			Any kind of lazy (re)instantiation should be done here.
	 */
	virtual void update() {}
	/**
	*	@brief	Overrides should specify the pipeline Vulkna bind point name here.
	*/
	virtual VkPipelineBindPoint get_pipeline_vk_bind_point() const = 0;
	/**
	*	@brief	Overrides should bind the pipeline and other resources here.
	*/
	virtual void bind_pipeline(const command_buffer &, command_recorder &) const = 0;
	/**
	*	@brief	Overrides should clean up and unbind the pipeline and other resources here.
	*/
	virtual void unbind_pipeline(const command_buffer &, command_recorder &) const {}
	/**
	*	@brief	Recreates the pipeline, should return the old pipeline (sliced to a vk::vk_pipeline object), if any.
	*/
	virtual optional<vk::vk_pipeline<>> recreate_pipeline() = 0;

	device_pipeline(const ste_context &ctx,
					lib::unique_ptr<pipeline_layout> &&layout,
					optional<std::reference_wrapper<const pipeline_external_binding_set>> external_binding_set)
		: ctx(ctx),
		layout(std::move(layout)),
		binding_sets(*this->layout,
					 ctx.device().binding_set_pool()),
		external_binding_set(external_binding_set ? &external_binding_set.get().get() : nullptr)
	{}

public:
	device_pipeline(device_pipeline&&) = default;
	device_pipeline &operator=(device_pipeline&&) = default;

	virtual ~device_pipeline() noexcept {}

	/**
	 *	@brief	Creates a resource binder for a given variable name
	 */
	pipeline_bind_point operator[](const lib::string &resource_name) {
		const pipeline_binding_layout *bind;

		// Check first if resource_name refers to a push constant path
		push_constant_path push_path(resource_name);
		auto optional_push_constant = (*layout->push_constants_layout)[push_path];
		if (optional_push_constant) {
			// Found push constant
			auto bp = lib::allocate_unique<pipeline_push_constant_bind_point>(layout->push_constants_layout.get(),
																			  optional_push_constant.get());
			return pipeline_bind_point(std::move(bp));
		}

		// Not a push constant, look at other variables
		auto var_it = layout->variables_map.find(resource_name);
		if (var_it != layout->variables_map.end()) {
			// Name references a variable
			bind = &var_it->second;
		}
		else {
			throw device_pipeline_unrecognized_variable_name_exception("Resource with provided name doesn't exist in pipeline layout");
		}

		// Create the binder
		if (bind->binding->binding_type == ste_shader_stage_binding_type::spec_constant) {
			auto bp = lib::allocate_unique<pipeline_specialization_constant_bind_point>(bind->binding->variable.get(),
																						layout.get(),
																						resource_name);
			return pipeline_bind_point(std::move(bp));
		}
		if (bind->binding->binding_type == ste_shader_stage_binding_type::storage ||
			bind->binding->binding_type == ste_shader_stage_binding_type::uniform) {
			auto bp = lib::allocate_unique<pipeline_resource_bind_point>(&binding_queue,
																		 layout.get(),
																		 bind);
			return pipeline_bind_point(std::move(bp));
		}

		// Can't be...
		assert(false);
		throw device_pipeline_unrecognized_variable_name_exception("Resource with provided name doesn't exist in pipeline layout");
	}

	/**
	*	@brief	Creates a bind command that binds the pipeline.
	*			Should be called before issuing any commands that use the pipeline.
	*
	*			Each call to bind should be followed by a call to unbind.
	*/
	auto cmd_bind() {
		prebind_update();
		return device_pipeline_cmd_bind(this);
	}

	/**
	*	@brief	Creates a bind command that unbinds the pipeline.
	*/
	auto cmd_unbind() {
		return device_pipeline_cmd_unbind(this);
	}

	const auto& get_layout() const { return *layout; }
};

}
}
