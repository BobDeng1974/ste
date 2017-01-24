
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

#include "chromaticity.glsl"

#include "material.glsl"
#include "light.glsl"
#include "light_cascades.glsl"
#include "linked_light_lists.glsl"

#include "gbuffer.glsl"

layout(std430, binding = 0) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

layout(std430, binding = 1) restrict readonly buffer material_layer_data {
	material_layer_descriptor mat_layer_descriptor[];
};

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 6) restrict readonly buffer gbuffer_data {
	g_buffer_element gbuffer[];
};
layout(shared, binding = 7) restrict readonly buffer directional_lights_cascades_data {
	light_cascade_descriptor directional_lights_cascades[];
};

layout(r8ui,  binding = 5) restrict readonly uniform uimage2D lll_size;
layout(r32ui, binding = 6) restrict readonly uniform uimage2D lll_heads;
layout(shared, binding = 11) restrict readonly buffer lll_data {
	lll_element lll_buffer[];
};

uniform float cascades_depths[directional_light_cascades];

#include "linked_light_lists_load.glsl"

#include "gbuffer_load.glsl"

#include "material_evaluate.glsl"

#include "deferred_shading.glsl"


layout(bindless_sampler) uniform samplerCubeArrayShadow shadow_depth_maps;
layout(bindless_sampler) uniform samplerCubeArray shadow_maps;

layout(bindless_sampler) uniform sampler2DArrayShadow directional_shadow_depth_maps;
layout(bindless_sampler) uniform sampler2DArray directional_shadow_maps;

layout(bindless_sampler) uniform sampler3D scattering_volume;

layout(bindless_sampler) uniform sampler2D microfacet_refraction_fit_lut;
layout(bindless_sampler) uniform sampler2DArray microfacet_transmission_fit_lut;

layout(bindless_sampler) uniform sampler2DArray atmospheric_optical_length_lut;
layout(bindless_sampler) uniform sampler3D atmospheric_scattering_lut;
layout(bindless_sampler) uniform sampler3D atmospheric_mie0_scattering_lut;
layout(bindless_sampler) uniform sampler3D atmospheric_ambient_lut;

layout(binding = 0) uniform sampler2D back_face_depth;
layout(binding = 1) uniform sampler2D front_face_depth;

out vec4 gl_FragColor;

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);

	g_buffer_element g_frag = gbuffer_load(coord);

	vec3 shaded_fragment = deferred_shade_fragment(g_frag, coord,
												   shadow_depth_maps, 
												   shadow_maps, 
												   directional_shadow_depth_maps,
												   directional_shadow_maps,
												   scattering_volume, 
												   microfacet_refraction_fit_lut,
												   microfacet_transmission_fit_lut,
												   atmospheric_optical_length_lut,
												   atmospheric_scattering_lut,
												   atmospheric_mie0_scattering_lut,
												   atmospheric_ambient_lut,
												   back_face_depth, 
												   front_face_depth);

	vec3 xyY = XYZtoxyY(RGBtoXYZ(shaded_fragment));

	gl_FragColor = vec4(xyY, 1);
}
