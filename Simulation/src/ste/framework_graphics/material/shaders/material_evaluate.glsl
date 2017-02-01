
#include "material.glsl"
#include "material_layer_unpack.glsl"

#include "subsurface_scattering.glsl"

#include "linearly_transformed_cosines.glsl"
#include "microfacet_ggx_fitting.glsl"

#include "cook_torrance.glsl"
#include "lambert_diffuse.glsl"
#include "disney_diffuse.glsl"

#include "fresnel.glsl"

#include "light_transport.glsl"

#include "deferred_shading_common.glsl"
#include "common.glsl"

vec3 material_evaluate_layer_radiance(material_layer_unpacked_descriptor descriptor,
									  deferred_material_ltc_luts ltc_luts, 
									  vec3 light_polygon[4],
									  vec3 wp,
									  vec3 wn,
									  vec3 wv,
									  vec3 l,
									  vec3 h,
									  float cos_critical, 
									  float refractive_ratio,
									  vec3 irradiance,
									  vec3 albedo,
									  vec3 diffuse_illuminance) {
	// Anisotropic roughness
	//float rx = descriptor.roughness * descriptor.anisotropy_ratio;
	//float ry = descriptor.roughness / descriptor.anisotropy_ratio;
	
	// Specular color
	vec3 specular_tint = vec3(1);
	vec3 c_spec = mix(specular_tint, albedo, descriptor.metallic);

	// Claculate polygonal light irradiance using linearly transformed cosines
	vec2 ltccoords = LTC_Coords(ltc_luts.ltc_ggx_fit, dot(wn, wv), descriptor.roughness);
	mat3 ltc_M_inv = LTC_Matrix(ltc_luts.ltc_ggx_fit, ltccoords);
	float ltc_ampl = texture(ltc_luts.ltc_ggx_amplitude, ltccoords).x;
	
	vec3 specular_irradiance = LTC_Evaluate(wn, wv, wp, ltc_M_inv, light_polygon, true) * ltc_ampl;
	vec3 diffuse_irradiance  = LTC_Evaluate(wn, wv, wp, mat3(1), light_polygon, true);
	
	// Specular
	vec3 Specular = c_spec * specular_irradiance;
	float F = fresnel(dot(l, h), cos_critical, refractive_ratio);

	// Diffuse
	//vec3 Diffuse = diffuse_illuminance * disney_diffuse_brdf(n, v, l, h, descriptor.roughness);
	vec3 Diffuse = diffuse_illuminance * diffuse_irradiance;

	// Evaluate BRDF
	vec3 brdf = Specular * F + (1.f - descriptor.metallic) * Diffuse;

	return brdf * irradiance;
}

float material_attenuation_through_layer(float transmittance,
										 float metallic,
										 float masking) {
	float passthrough = 1.f - metallic;
	return transmittance * masking * passthrough;
}

/*
 *	Evaluate radiance of material at fragment
 *
 *	@param layer		Material layer
 *	@param frag			Fragment shading parameters
 *	@param light		Light shading parameters
 *	@param object_thickness	Object thickness at shaded fragment
 *	@param material_microfacet_luts	Microfacet GGX fitting LUTs
 *	@param shadow_maps	Shadow maps
 *	@param occlusion	Light occlusion
 *	@param external_medium_ior	Index-of-refraction of source medium
 */
vec3 material_evaluate_radiance(material_layer_descriptor layer,
								fragment_shading_parameters frag,
								light_shading_parameters light,
								float object_thickness,
								deferred_material_microfacet_luts material_microfacet_luts,
								deferred_material_ltc_luts ltc_luts, 
								deferred_shading_shadow_maps shadow_maps,
								float occlusion,
								float external_medium_ior = 1.0002772f) {
	vec3 rgb = vec3(0);

	vec3 n = frag.n;
	vec3 t = frag.t;
	vec3 b = frag.b;
	vec3 position = frag.p;
	ivec2 frag_coords = frag.coords;
	
	material_layer_unpacked_descriptor descriptor = material_layer_unpack(layer);
	
	// We have subsurface scattering if all layers have an attenuation of less than infinity
	bool has_subsurface_scattering = object_thickness > .0f && all(lessThan(descriptor.attenuation_coefficient, vec3(inf)));

	// Early bail
	if (occlusion <= .0f && !has_subsurface_scattering)
		return vec3(.0f);
		
	vec3 l = light.l;
	vec3 v = frag.v;
	float top_medium_ior = external_medium_ior;

	// Attenuation at current layer
	vec3 attenuation = vec3(1.f);
	// Attenuation of outgoing radiance, for sub-surface scattering
	vec3 sss_attenuation = vec3(1.f);

	while (true) {
		// Read layer properties
		vec3 albedo = descriptor.albedo.rgb;
		float thickness = material_is_base_layer(descriptor) ? object_thickness : descriptor.thickness;
		float metallic = descriptor.metallic;
		float roughness = descriptor.roughness;
		vec3 attenuation_coefficient = descriptor.attenuation_coefficient;
		float bottom_medium_ior = descriptor.ior;
		
		// Compute sine and cosine of critical angle
		float refractive_ratio = bottom_medium_ior / top_medium_ior;
		float cos_critical = refractive_ratio < 1.f ? 
								sqrt(1.f - refractive_ratio * refractive_ratio) :
								.0f;
		// Compute Fresnel reflection at parallel incidence
		float F0 = fresnel_F0(refractive_ratio);

		// Evaluate refracted vectors
		vec3 refracted_v = -ggx_refract(material_microfacet_luts.microfacet_refraction_fit_lut,
										v, n,
										roughness,
										refractive_ratio);
		vec3 refracted_l = -ggx_refract(material_microfacet_luts.microfacet_refraction_fit_lut,
										l, n,
										roughness,
										refractive_ratio);

		// Evaluate total inner (downwards into material) and outer (upwards towards eye) transmission
		float inner_transmission_ratio = ggx_transmission_ratio_v4(material_microfacet_luts.microfacet_transmission_fit_lut, 
																   v, n, 
																   roughness, 
																   refractive_ratio);
		float outer_transmission_ratio = ggx_transmission_ratio_v4(material_microfacet_luts.microfacet_transmission_fit_lut, 
																   /*refracted_l*/l, n, 
																   roughness, 
																   1.f / refractive_ratio);
	
		// Compute total and outer path lengths inside current layer
		//float dotNV = max(epsilon, dot(n,refracted_v));
		//float dotNL = max(epsilon, dot(n,refracted_l));
		float dotNV = max(epsilon, dot(n,v));
		float dotNL = max(epsilon, dot(n,l));
		float path_length = thickness * (1.f / dotNV + 1.f / dotNL);
		float outer_path_length = thickness / dotNL;
		
		// Compute light attenuation in layer
		vec3 k = beer_lambert(attenuation_coefficient, path_length);
		// Compute light attenuation in layer on outgoing direction only
		vec3 sss_outer_extinction = beer_lambert(attenuation_coefficient, outer_path_length);

		// Diffused light is a portion of the energy scattered inside layer (based on albedo), unattenuated light doesn't contribute to diffuse
		vec3 scattering = inner_transmission_ratio * outer_transmission_ratio * (vec3(1.f) - k) * albedo;

		// Half vector
		vec3 h = normalize(v + l);
		// Evaluate layer BRDF

	vec3 u_quadPoints[4] = {
		light.ld.position + vec3(-45,0 ,-30),
		light.ld.position + vec3(-25,30,+30),
		light.ld.position + vec3(+25,30,+40),
		light.ld.position + vec3(+25,0 ,-30),
	};
		rgb += attenuation * material_evaluate_layer_radiance(descriptor,
															  ltc_luts,
															  u_quadPoints,
															  frag.world_position,
															  frag.world_normal,
															  frag.world_v, 
															  l, h,
															  cos_critical, 
															  refractive_ratio,
															  light.lux,
															  albedo,
															  scattering);
							
		// Update incident and outgoing vectors to refracted ones before continuing to next layer
		//v = refracted_v;
		//l = refracted_l;
	
		// Compute attenuated light due to Fresnel transmittance, microfacet masking-shadowing and metallicity
		float layer_surface_inner_attenuation = material_attenuation_through_layer(inner_transmission_ratio, 
																				   metallic,
																				   1/*Gshadow*/);
		float layer_surface_outer_attenuation = material_attenuation_through_layer(outer_transmission_ratio, 
																				   metallic,
																				   1/*Gmask*/);
		float total_layer_surface_attenuation = layer_surface_inner_attenuation * layer_surface_outer_attenuation;

		// Update attenuation at layer surface
		attenuation *= total_layer_surface_attenuation;
		// For sub-surface scattering we assume normal incidence of light, and we attenuated on (presumed) incident and outgoing sides
		sss_attenuation *= layer_surface_outer_attenuation * material_attenuation_through_layer(1.f - F0, metallic, 1.f);

		// If this is the base layer, stop
		if (material_is_base_layer(descriptor))
			break;

		// Otherwise, update attenuation with attenuated (absorbed and scattered) light
		attenuation *= k;
		// Update sss attenuation with attenuation on the way out and attenuation on the way in with parallel incident
		sss_attenuation *= sss_outer_extinction * beer_lambert(attenuation_coefficient, thickness);

		// Update ior and descriptor for next layer
		top_medium_ior = bottom_medium_ior;
		descriptor = material_layer_unpack(mat_layer_descriptor[descriptor.next_layer_id]);
	}
	
	// Sub-surface scattering
	has_subsurface_scattering = has_subsurface_scattering && all(lessThan(descriptor.attenuation_coefficient, vec3(inf)));
	bool fully_attenuated = all(lessThan(sss_attenuation, vec3(epsilon)));

	if (has_subsurface_scattering && !fully_attenuated) {
		rgb += sss_attenuation * subsurface_scattering(descriptor,
													   position,
													   n,
							 						   object_thickness,
													   light.ld,
													   shadow_maps.shadow_maps, light.ll_id,
													   -v,
													   frag_coords);
	}

	return rgb * occlusion;
}
