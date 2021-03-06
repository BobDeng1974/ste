#pragma once

#include <stdafx.hpp>

#include <ste_shader_stage_binding.hpp>
#include <ste_shader_stage_variable.hpp>
#include <ste_shader_exceptions.hpp>

#include <lib/vector.hpp>
#include <lib/string.hpp>
#include <optional.hpp>

namespace ste {
namespace gl {

namespace _internal {

struct ste_shader_spirv_reflected_variable {
	ste_shader_stage_variable_type type{ ste_shader_stage_variable_type::unknown };
	lib::string name;

	std::uint32_t rows{ 1 };						// Matrix/vector rows. 1 for scalars.
	std::uint32_t columns{ 1 };						// Columns. >1 for matrix types.
	byte_t matrix_stride{ 0_B };					// Array stride between elements. 0 for tightly packed.

	std::uint32_t array_elements{ 1 };				// Array elements. >1 for arrays or 0 for a run-time array.
	byte_t array_stride{ 0_B };						// Array stride between elements. 0 for tightly packed.
	optional<std::uint32_t> array_length_specialization_constant_id;	// id of array length specialization constant

	byte_t offset{ 0_B };							// Member offset in a struct.
	byte_t width{ 0_B };							// Integer/float width, only used for int_t and uint_t.
	optional<std::uint64_t> constant_value;			// Constant integer value, if available.

	lib::vector<ste_shader_spirv_reflected_variable> struct_members;

	void consume(const ste_shader_spirv_reflected_variable &src) {
		// Consume only non-default attributes

		lib::string name = std::move(src.name.size() ?
									 src.name :
									 this->name);

		this->name = std::move(name);
		this->rows = src.rows;
		this->columns = src.columns;
		if (src.matrix_stride > 0_B)
			this->matrix_stride = src.matrix_stride;

		if (src.type != ste_shader_stage_variable_type::unknown) {
			this->type = src.type;
			this->width = src.width;
		}

		if (src.array_elements != 1) {
			this->array_elements = src.array_elements;
			this->array_length_specialization_constant_id = src.array_length_specialization_constant_id;
		}
		if (src.array_stride > 0_B)
			this->array_stride = src.array_stride;

		if (src.constant_value)
			this->constant_value = std::move(src.constant_value);
		if (src.struct_members.size())
			this->struct_members = std::move(src.struct_members);
	}

	/**
	*	@brief	Checks if type is known
	*/
	bool has_type() const {
		return type != ste_shader_stage_variable_type::unknown;
	}
	/**
	*	@brief	Checks if type is opaque
	*/
	bool is_opaque() const {
		return ste_shader_stage_variable_type_is_opaque(type);
	}
	/**
	*	@brief	Checks if variable is an array
	*/
	bool is_array() const {
		return array_elements != 1;
	}
	/**
	*	@brief	Checks if variable is a run-time sized array
	*/
	bool is_runtime_array() const {
		return array_elements == 0;
	}
	/**
	*	@brief	Checks if variable is a struct
	*/
	bool is_struct() const {
		return struct_members.size() > 0;
	}
	/**
	*	@brief	Returns number of elements in the struct.
	*/
	std::size_t struct_size() const {
		if (is_struct())
			return struct_members.size();
		return 0;
	}
	/**
	*	@brief	Checks if variable is a matrix
	*/
	bool is_matrix() const {
		return columns > 1;
	}

	lib::unique_ptr<ste_shader_stage_variable> generate_variable(bool is_spec_constant,
																 const lib::vector<ste_shader_stage_binding> &binds) {
		// Unknown type is an internal error
		if (!has_type()) {
			throw ste_shader_opaque_or_unknown_type();
		}

		// Arrays, matrices and vectors are composite types.
		// Create underlying types first.
		lib::unique_ptr<ste_shader_stage_variable> var;

		if (is_opaque()) {
			var = lib::unique_ptr<ste_shader_stage_variable>(lib::allocate_unique<ste_shader_stage_variable_opaque>(type,
																												name,
																												offset));
		}
		else if (is_struct()) {
			// Handle structs recursively
			lib::vector<lib::unique_ptr<ste_shader_stage_variable>> elements;

            // Some SPIR-v compilers omit the offset. In this case allow recostruction manually
            bool reconstruct_offsets = true;
            for (auto &e : struct_members)
                reconstruct_offsets &= e.offset == 0_B;

            if (!reconstruct_offsets) {
                // We have offsets, generate variables and sort by offset
                for (auto &e : struct_members) {
                    auto it = elements.begin();
                    for (; it != elements.end() && (*it)->offset() < e.offset; ++it) {}

                    assert(it == elements.end() || (*it)->offset() > e.offset);
                    elements.insert(it, e.generate_variable(false,
                                                            binds));
                }
            }
            else {
                // We create offsets, assuming tight packing and correct sorting
                auto offset = 0_B;
                for (auto &e : struct_members) {
                    e.offset = offset;
                    auto member_variable = e.generate_variable(false,
                                                               binds);
                    const auto size = member_variable->size_bytes();
                    elements.push_back(std::move(member_variable));

                    offset += size;
                }
            }

			var = lib::unique_ptr<ste_shader_stage_variable>(lib::allocate_unique<ste_shader_stage_variable_struct>(std::move(elements),
																													name,
																													offset));
		}
		else {
			// Handle scalars, vectors and matrices
			auto scalar_var = lib::allocate_unique<ste_shader_stage_variable_scalar>(type,
																				 name,
																				 offset,
																				 width);

			if (rows > 1 || columns > 1) {
				// Vector/matrix
				auto matrix_var = lib::allocate_unique<ste_shader_stage_variable_matrix>(std::move(scalar_var),
																					 name,
																					 offset,
																					 rows,
																					 columns,
																					 matrix_stride);
				var = lib::unique_ptr<ste_shader_stage_variable>(std::move(matrix_var));
			}
			else {
				// Scalar
				var = lib::unique_ptr<ste_shader_stage_variable>(std::move(scalar_var));
			}
		}

		// Arrays
		if (is_array()) {
			// For specializeable array lengths, find the specialization constant
			optional<const ste_shader_stage_variable_scalar*> length_specialization_constant;
			if (array_length_specialization_constant_id) {
				for (auto &b : binds) {
					if (b.binding_type == ste_shader_stage_binding_type::spec_constant &&
						b.bind_idx == array_length_specialization_constant_id.get()) {
						auto ptr = dynamic_cast<const ste_shader_stage_variable_scalar *>(b.variable.get());
						assert(ptr);
						if (ptr)
							length_specialization_constant = ptr;
						break;
					}
				}

				assert(!!length_specialization_constant && "Specialization constant not found!");
			}

			auto array_var = lib::allocate_unique<ste_shader_stage_variable_array>(std::move(var),
																			   name,
																			   offset,
																			   array_elements,
																			   matrix_stride,
																			   length_specialization_constant);
			var = lib::unique_ptr<ste_shader_stage_variable>(std::move(array_var));
		}

		// Specialization constants: Save default value
		if (is_spec_constant) {
			auto default_specialized_value = constant_value.get();
			lib::string data;
			data.resize(sizeof(std::uint64_t));
			memcpy(data.data(), &default_specialized_value, sizeof(std::uint64_t));
			var->set_default_specialized_value(data);
		}

		return std::move(var);
	}
};

}

}
}
