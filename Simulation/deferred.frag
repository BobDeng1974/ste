
#type frag
#version 450
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D normal_tex;
layout(binding = 1) uniform sampler2D position_tex;
layout(binding = 2) uniform sampler2D color_tex;
layout(binding = 3) uniform sampler2D tangent_tex;
layout(binding = 4) uniform isampler2D mat_idx_tex;

uniform vec3 light_diffuse;
uniform float light_luminance;
uniform float light_radius = 5;
uniform vec3 light_pos;

layout(std430, binding = 0) buffer material_data {
	material_descriptor mat_descriptor[];
};

void main() {
	int draw_idx = texelFetch(mat_idx_tex, ivec2(gl_FragCoord.xy), 0).x;
	vec4 c = texelFetch(color_tex, ivec2(gl_FragCoord.xy), 0);

	vec3 diffuse = c.rgb;
	float specular = c.w;

	if (draw_idx < 0) {
		gl_FragColor = vec4(XYZtoxyY(RGBtoXYZ(diffuse)), 1);
		return;
	}

	vec3 n = texelFetch(normal_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 t = texelFetch(tangent_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 b = cross(t, n);
	vec3 position = texelFetch(position_tex, ivec2(gl_FragCoord.xy), 0).xyz;

	material_descriptor md = mat_descriptor[draw_idx];

	vec3 v = light_pos - position;
	float dist = length(v);

	float brdf = calc_brdf(md, position, n, t, b, v);
	float attenuation_factor = max(.01f, dist / (light_radius * 100));
	float incident_radiance = light_luminance / pow(attenuation_factor, 2);
	
	vec3 xyY = XYZtoxyY(RGBtoXYZ(diffuse * light_diffuse + diffuse * md.emission));
	xyY.z += min_luminance;
	xyY.z *= max(0, mix(0.5f, 1.f, specular) * brdf * incident_radiance);
	xyY.z += md.emission;

	gl_FragColor = vec4(xyY, 1);
}
