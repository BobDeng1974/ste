
#include "chromaticity.glsl"

#include "material.glsl"

#include "shadow.glsl"
#include "light.glsl"
#include "light_cascades.glsl"
#include "linked_light_lists.glsl"

#include "gbuffer.glsl"

#include "volumetric_scattering.glsl"

#include "project.glsl"
#include "girenderer_transform_buffer.glsl"

float get_thickness(ivec2 coord,
					sampler2D back_face_depth, 
					sampler2D front_face_depth) {
	float fd = unproject_depth(texelFetch(front_face_depth, coord, 0).x);
	float bd = unproject_depth(texelFetch(back_face_depth, coord, 0).x);

	return fd - bd;
}

vec3 deferred_shade_fragment(g_buffer_element frag, ivec2 coord,
							 samplerCubeArrayShadow shadow_depth_maps, 
							 samplerCubeArray shadow_maps, 
							 sampler2DArrayShadow directional_shadow_depth_maps,
							 sampler2DArray directional_shadow_maps,
							 sampler3D scattering_volume, 
							 sampler2D microfacet_refraction_fit_lut, 
							 sampler2DArray microfacet_transmission_fit_lut, 
							 sampler2D back_face_depth, 
							 sampler2D front_face_depth) {
	// Read G-buffer data from fragment 
	int draw_idx = gbuffer_parse_material(frag);
	material_descriptor md = mat_descriptor[draw_idx];

	vec2 uv = gbuffer_parse_uv(frag);
	vec2 duvdx = gbuffer_parse_duvdx(frag);
	vec2 duvdy = gbuffer_parse_duvdy(frag);

	// Calculate depth and extrapolate world position
	float depth = gbuffer_parse_depth(frag);
	vec3 position = unproject_screen_position(depth, gl_FragCoord.xy / vec2(backbuffer_size()));

	// Normal map
	vec3 n = gbuffer_parse_normal(frag);
	vec3 t = gbuffer_parse_tangent(frag);
	vec3 b = cross(t, n);
	normal_map(md, uv, duvdx, duvdy, n, t, b);

	// Calculate perceived object thickness in camera space (used for subsurface scattering)
	float thickness = get_thickness(coord, back_face_depth, front_face_depth);

	// Read material data
	material_layer_descriptor head_layer = mat_layer_descriptor[md.head_layer];
	vec4 material_texture = material_base_texture(md, uv, duvdx, duvdy);
	float cavity = material_cavity(md, uv, duvdx, duvdy);

	// Iterate lights in the linked-light-list structure
	vec3 rgb = vec3(.0f);
	ivec2 lll_coords = coord / lll_image_res_multiplier;
	uint32_t lll_ptr = imageLoad(lll_heads, lll_coords).x;
	for (;; ++lll_ptr) {
		lll_element lll_p = lll_buffer[lll_ptr];
		if (lll_eof(lll_p))
			break;

		// Check that light is in depth range
		vec2 lll_depth_range = lll_parse_depth_range(lll_p);
		if (depth >= lll_depth_range.x) {
			// Translate light id from linked-light-list to light-buffer index and load light data
			uint light_idx = uint(lll_parse_light_idx(lll_p));
			light_descriptor ld = light_buffer[light_idx];

			// Light id is used for shadow map access
			uint light_id = uint(lll_parse_ll_idx(lll_p));

			// Compute light incident ray and range
			float l_radius = ld.radius;
			float l_dist;
			vec3 l;
			vec3 incident = light_incidant_ray(ld, position);
			if (ld.type == LightTypeSphere) {
				float light_effective_range = ld.effective_range;
				float dist2 = dot(incident, incident);
				if (dist2 >= light_effective_range*light_effective_range)
					continue;

				l_dist = sqrt(dist2);
				l = incident / l_dist;
			}
			else {
				l_dist = abs(ld.directional_distance);
				l = incident;
			}

			// Shadow map query
			float shdw = 1.f;
			if (ld.type == LightTypeSphere) {
				vec3 shadow_v = position - ld.transformed_position;
				shdw = shadow(shadow_depth_maps,
							  shadow_maps,
							  light_id,
							  shadow_v,
							  l_radius);
			}
			else {
				// Query cascade index, and shadowmap index and construct cascade projection matrix
				uint32_t cascade_idx = light_get_cascade_descriptor_idx(ld);
				light_cascade_descriptor cascade_descriptor = directional_lights_cascades[cascade_idx];

				int cascade = light_which_cascade_for_position(cascade_descriptor, position);
				int shadowmap_idx = light_get_cascade_shadowmap_idx(ld, cascade);

				// Read cascade projection parameters
				float cascade_proj_far, cascade_eye_dist;
				vec2 cascde_recp_vp;
				light_cascade_data(cascade_descriptor, cascade, cascade_proj_far, cascade_eye_dist, cascde_recp_vp);

				// Light incident equals minus light direction for directional lights
				mat3x4 M = light_cascade_projection(cascade_descriptor, cascade, -l, cascade_eye_dist, cascde_recp_vp);

				shdw = shadow(directional_shadow_depth_maps,
							  directional_shadow_maps,
							  shadowmap_idx,
							  position,
							  M,
							  cascde_recp_vp,
							  1000,//l_dist,
							  10);//l_radius);

				//? TODO: Remove!
				// Inject some ambient, still without global illumination...
				rgb += ld.diffuse * ld.luminance * 1e-10 * (1-shdw);
			}

			// Calculate occlusion, distance to light, normalized incident and reflection (eye) vectors
			float occlusion = max(.0f, cavity * shdw);
			vec3 v = normalize(-position);
			
			// Evaluate material radiance for given light
			rgb += material_texture.rgb * material_evaluate_radiance(head_layer,
																	 position,
																	 n, t, b,
																	 v, l,
																	 thickness,
																	 ld,
																	 microfacet_refraction_fit_lut,
																	 microfacet_transmission_fit_lut,
																	 shadow_maps, light_id,
																	 l_dist,
																	 occlusion);
		}
	}

	// Add material emission
	rgb += material_emission(md);

	// Apply volumetric lighting to computed radiance
	rgb = volumetric_scattering(scattering_volume, rgb, vec2(gl_FragCoord.xy), depth);

	return rgb;
}
