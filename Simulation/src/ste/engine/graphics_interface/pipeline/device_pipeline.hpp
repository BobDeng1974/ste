//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <pipeline_layout.hpp>
#include <pipeline_resource_binding_queue.hpp>
#include <pipeline_resource_bind_point.hpp>
#include <device_pipeline_exceptions.hpp>

#include <pipeline_layout_set_index.hpp>
#include <pipeline_binding_set_collection.hpp>
#include <pipeline_binding_set_pool.hpp>

#include <command_buffer.hpp>
#include <command_recorder.hpp>
#include <cmd_bind_descriptor_sets.hpp>

namespace StE {
namespace GL {

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
		void operator()(const command_buffer &buffer, command_recorder &recorder) const override final {
			// Bind external binding sets
			if (pipeline->external_binding_sets != nullptr)
				recorder << pipeline->external_binding_sets->cmd_bind(pipeline->pipeline_bind_point(),
																	  &pipeline->layout.get());
			// Bind pipeline's binding sets
			recorder << pipeline->binding_sets.cmd_bind(pipeline->pipeline_bind_point());
		
			// Bind pipeline
			pipeline->bind_pipeline(buffer, recorder);
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
		void operator()(const command_buffer &buffer, command_recorder &recorder) const override final {
			pipeline->unbind_pipeline(buffer, recorder);
		}
	};

protected:
	const ste_context &ctx;

	pipeline_layout layout;
	pipeline_resource_binding_queue binding_queue;

	pipeline_binding_set_collection binding_sets;

	// External binding sets
	const pipeline_external_binding_set_collection *external_binding_sets;

private:
	void update() {
		// Update sets, as needed
		layout.recreate_invalidated_set_layouts();
		
		// Recreate pipeline if pipeline layout was invalidated for any reason
		if (layout.read_and_reset_invalid_layout_flag()) {
			recreate_pipeline();
		}

		// Write resource descriptors to binding sets from the binding queue and clear it
		if (!binding_queue.empty()) {
			binding_sets.write(binding_queue);
			binding_queue.clear();
		}
	}

protected:
	virtual VkPipelineBindPoint pipeline_bind_point() const = 0;
	virtual void bind_pipeline(const command_buffer &, command_recorder &) const = 0;
	virtual void unbind_pipeline(const command_buffer &, command_recorder &) const {}
	virtual void recreate_pipeline() = 0;

	device_pipeline(const ste_context &ctx,
					pipeline_binding_set_pool &pool,
					pipeline_layout &&layout,
					optional<std::reference_wrapper<const pipeline_external_binding_set_collection>> external_binding_sets)
		: ctx(ctx),
		layout(std::move(layout)),
		binding_sets(this->layout, 
					 pool),
		external_binding_sets(external_binding_sets ? &external_binding_sets.get().get() : nullptr)
	{}

public:
	device_pipeline(device_pipeline&&) = default;
	device_pipeline &operator=(device_pipeline&&) = default;

	virtual ~device_pipeline() noexcept {}

	/**
	 *	@brief	Creates a resource binder for a given variable name
	 */
	auto operator[](const std::string &resource_name) {
		const pipeline_binding_layout *bind = nullptr;
		
		auto var_it = layout.variables_map.find(resource_name);
		if (var_it != layout.variables_map.end()) {
			// Name references a variable
			bind = &var_it->second;
		}
		else {
			throw device_pipeline_unrecognized_variable_name_exception("Resource with provided name doesn't exist in pipeline layout");
		}

		// Create the binder
		return pipeline_resource_bind_point(&binding_queue,
											&layout,
											resource_name,
											bind);
	}

	/**
	*	@brief	Creates a bind command that binds the pipeline.
	*			Should be called before issuing any commands that use the pipeline.
	*
	*			Each call to bind should be followed by a call to unbind.
	*/
	auto cmd_bind() {
		update();
		return device_pipeline_cmd_bind(this);
	}

	/**
	*	@brief	Creates a bind command that unbinds the pipeline.
	*/
	auto cmd_unbind() {
		return device_pipeline_cmd_unbind(this);
	}
};

}
}
