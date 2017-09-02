//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_block.hpp>

#include <ste_type_traits.hpp>

namespace ste {
namespace resource {

/**
*	@brief	4-component block
*/
template <block_type type, unsigned comp0_bits, unsigned comp1_bits, unsigned comp2_bits, unsigned comp3_bits, gl::component_swizzle comp0, gl::component_swizzle comp1, gl::component_swizzle comp2, gl::component_swizzle comp3>
class block_4components {
	static_assert(comp0 == gl::component_swizzle::r || comp1 == gl::component_swizzle::r || comp2 == gl::component_swizzle::r || comp3 == gl::component_swizzle::r);
	static_assert(comp0 == gl::component_swizzle::g || comp1 == gl::component_swizzle::g || comp2 == gl::component_swizzle::g || comp3 == gl::component_swizzle::g);
	static_assert(comp0 == gl::component_swizzle::b || comp1 == gl::component_swizzle::b || comp2 == gl::component_swizzle::b || comp3 == gl::component_swizzle::b);
	static_assert(comp0 == gl::component_swizzle::a || comp1 == gl::component_swizzle::a || comp2 == gl::component_swizzle::a || comp3 == gl::component_swizzle::a);

	static constexpr auto a_type = type != block_type::block_srgb ? type : block_type::block_unorm;		// sRGB formats always use unorm for the alpha channel
	using common_type_selector_t = _detail::block_common_type_selector<type, std::max(std::max(comp0_bits, comp1_bits), std::max(comp2_bits, comp3_bits))>;

public:
	static constexpr unsigned elements = 4;
	static constexpr block_type blocktype = type;

	static constexpr unsigned r_index = comp0 == gl::component_swizzle::r ? 0 :
		(comp1 == gl::component_swizzle::r ? 1 :
		(comp2 == gl::component_swizzle::r ? 2 : 3));
	static constexpr unsigned g_index = comp0 == gl::component_swizzle::g ? 0 :
		(comp1 == gl::component_swizzle::g ? 1 :
		(comp2 == gl::component_swizzle::g ? 2 : 3));
	static constexpr unsigned b_index = comp0 == gl::component_swizzle::b ? 0 :
		(comp1 == gl::component_swizzle::b ? 1 :
		(comp2 == gl::component_swizzle::b ? 2 : 3));
	static constexpr unsigned a_index = comp0 == gl::component_swizzle::a ? 0 :
		(comp1 == gl::component_swizzle::a ? 1 :
		(comp2 == gl::component_swizzle::a ? 2 : 3));
	static constexpr unsigned r_bits = comp0 == gl::component_swizzle::r ? comp0_bits :
		(comp1 == gl::component_swizzle::r ? comp1_bits :
		(comp2 == gl::component_swizzle::r ? comp2_bits : comp3_bits));
	static constexpr unsigned g_bits = comp0 == gl::component_swizzle::g ? comp0_bits :
		(comp1 == gl::component_swizzle::g ? comp1_bits :
		(comp2 == gl::component_swizzle::g ? comp2_bits : comp3_bits));
	static constexpr unsigned b_bits = comp0 == gl::component_swizzle::b ? comp0_bits :
		(comp1 == gl::component_swizzle::b ? comp1_bits :
		(comp2 == gl::component_swizzle::b ? comp2_bits : comp3_bits));
	static constexpr unsigned a_bits = comp0 == gl::component_swizzle::a ? comp0_bits :
		(comp1 == gl::component_swizzle::a ? comp1_bits :
		(comp2 == gl::component_swizzle::a ? comp2_bits : comp3_bits));
	static constexpr unsigned r_offset = comp0 == gl::component_swizzle::r ? 0 :
		(comp1 == gl::component_swizzle::r ? comp0_bits :
		(comp2 == gl::component_swizzle::r ? comp0_bits + comp1_bits : comp0_bits + comp1_bits + comp2_bits));
	static constexpr unsigned g_offset = comp0 == gl::component_swizzle::g ? 0 :
		(comp1 == gl::component_swizzle::g ? comp0_bits :
		(comp2 == gl::component_swizzle::g ? comp0_bits + comp1_bits : comp0_bits + comp1_bits + comp2_bits));
	static constexpr unsigned b_offset = comp0 == gl::component_swizzle::b ? 0 :
		(comp1 == gl::component_swizzle::b ? comp0_bits :
		(comp2 == gl::component_swizzle::b ? comp0_bits + comp1_bits : comp0_bits + comp1_bits + comp2_bits));
	static constexpr unsigned a_offset = comp0 == gl::component_swizzle::a ? 0 :
		(comp1 == gl::component_swizzle::a ? comp0_bits :
		(comp2 == gl::component_swizzle::a ? comp0_bits + comp1_bits : comp0_bits + comp1_bits + comp2_bits));

	static constexpr int index_for_component(gl::component_swizzle c) {
		if constexpr (c == gl::component_swizzle::r) return r_index;
		if constexpr (c == gl::component_swizzle::g) return g_index;
		if constexpr (c == gl::component_swizzle::b) return b_index;
		if constexpr (c == gl::component_swizzle::a) return a_index;
		static_assert(c == gl::component_swizzle::r || c == gl::component_swizzle::g || c == gl::component_swizzle::b || c == gl::component_swizzle::a);
		return -1;
	}

	static constexpr unsigned total_bits = comp0_bits + comp1_bits + comp2_bits + comp3_bits;
	static constexpr unsigned bytes = total_bits / 8;

	using r_comp_type = typename _detail::block_primary_type_selector<type, r_bits>::type;
	using g_comp_type = typename _detail::block_primary_type_selector<type, g_bits>::type;
	using b_comp_type = typename _detail::block_primary_type_selector<type, b_bits>::type;
	using a_comp_type = typename _detail::block_primary_type_selector<a_type, a_bits>::type;
	using r_comp_writer_type = typename _detail::block_primary_type_selector<type, r_bits>::block_writer_type;
	using g_comp_writer_type = typename _detail::block_primary_type_selector<type, g_bits>::block_writer_type;
	using b_comp_writer_type = typename _detail::block_primary_type_selector<type, b_bits>::block_writer_type;
	using a_comp_writer_type = typename _detail::block_primary_type_selector<a_type, a_bits>::block_writer_type;
	using common_type = typename common_type_selector_t::type;
	static constexpr block_common_type common_type_name = common_type_selector_t::common_type_name;

private:
	static_assert((total_bits % 8) == 0, "Block straddles bytes");
	static_assert(bytes > 0, "bytes is zero");

	using data_t = std::uint8_t[bytes];

	data_t data{};

public:
	block_4components() = default;
	block_4components(r_comp_type in_r, g_comp_type in_g, b_comp_type in_b, a_comp_type in_a) {
		r() = in_r;
		g() = in_g;
		b() = in_b;
		a() = in_a;
	}

	/**
	 *	@brief 	Reads red (1st) component's value
	 */
	r_comp_type r() const { return _detail::block_component<type, r_bits, r_offset, true>(data); }
	/**
	*	@brief 	Reads green (2nd) component's value
	*/
	g_comp_type g() const { return _detail::block_component<type, g_bits, g_offset, true>(data); }
	/**
	*	@brief 	Reads blue (3rd) component's value
	*/
	b_comp_type b() const { return _detail::block_component<type, b_bits, b_offset, true>(data); }
	/**
	*	@brief 	Reads alpha (4th) component's value
	*/
	a_comp_type a() const { return _detail::block_component<a_type, a_bits, a_offset, true>(data); }

	/**
	*	@brief 	Provides read-write access to red (1st) component
	*/
	auto r() { return _detail::block_component<type, r_bits, r_offset, false>(data); }
	/**
	*	@brief 	Provides read-write access to green (2nd) component
	*/
	auto g() { return _detail::block_component<type, g_bits, g_offset, false>(data); }
	/**
	*	@brief 	Provides read-write access to blue (3rd) component
	*/
	auto b() { return _detail::block_component<type, b_bits, b_offset, false>(data); }
	/**
	*	@brief 	Provides read-write access to alpha (4th) component
	*/
	auto a() { return _detail::block_component<a_type, a_bits, a_offset, false>(data); }

	template <int index>
	auto component() {
		static_assert(index >= 0 && index <= 3);
		if constexpr (index == 0) return r();
		if constexpr (index == 1) return g();
		if constexpr (index == 2) return b();
		if constexpr (index == 3) return a();
	}
	template <int index>
	auto component() const {
		static_assert(index >= 0 && index <= 3);
		if constexpr (index == 0) return r();
		if constexpr (index == 1) return g();
		if constexpr (index == 2) return b();
		if constexpr (index == 3) return a();
	}

	/**
	 *	@brief Loads a block from memory and writes the decoded data, in RGBA swizzling, to output buffer.
	 *	
	 *	@param	data		Input buffer
	 *	@param	rgba_output	Output buffer, must be able to hold sizeof(common_type)*4 bytes
	 *	
	 *	@return	Element count written
	 */
	static std::size_t load_block(const std::uint8_t *data, common_type *rgba_output) {
		auto &block = *reinterpret_cast<const block_4components*>(data);
		*(rgba_output + 0) = static_cast<common_type>(block.r());
		*(rgba_output + 1) = static_cast<common_type>(block.g());
		*(rgba_output + 2) = static_cast<common_type>(block.b());
		*(rgba_output + 3) = static_cast<common_type>(block.a());

		return 4;
	}

	/**
	*	@brief	Encodes from buffer, assumed to be in RGBA swizzling, to block.
	*
	*	@param	data			Input buffer
	*	@param	max_elements	Max elements to read
	*/
	template <typename src_type>
	void write_block(const src_type *data, std::size_t max_elements = 4) {
		a_comp_writer_type a_default_val;
		if constexpr (is_floating_point_v<a_comp_writer_type>)
			a_default_val = static_cast<a_comp_writer_type>(1.0);
		else
			a_default_val = std::numeric_limits<a_comp_type>::max();

		r() = static_cast<r_comp_type>(max_elements > 0 ? static_cast<r_comp_writer_type>(*(data + 0)) : static_cast<r_comp_writer_type>(0));
		g() = static_cast<g_comp_type>(max_elements > 1 ? static_cast<g_comp_writer_type>(*(data + 1)) : static_cast<g_comp_writer_type>(0));
		b() = static_cast<b_comp_type>(max_elements > 2 ? static_cast<b_comp_writer_type>(*(data + 2)) : static_cast<b_comp_writer_type>(0));
		a() = static_cast<a_comp_type>(max_elements > 3 ? static_cast<a_comp_writer_type>(*(data + 3)) : a_default_val);
	}

//	static void load_8blocks(const std::uint8_t *data, common_type *rgba_output) {
//		auto *blocks = reinterpret_cast<const block_4components*>(data);
//		*(rgba_output + 0) = static_cast<common_type>(block.r());
//		*(rgba_output + 1) = static_cast<common_type>(block.g());
//		*(rgba_output + 2) = static_cast<common_type>(block.b());
//		*(rgba_output + 3) = static_cast<common_type>(block.a());
//	}
};

}
}
