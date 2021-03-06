
#include <shadow_common.glsl>

#include <deferred_shading_common.glsl>
#include <light_cascades.glsl>
#include <project.glsl>

const float shadow_directional_max_penumbra = 8.f / shadow_dirmap_size;
const float shadow_directional_min_penumbra = .0f;//1.f / shadow_dirmap_size;

/*
 *	Creates a slightly offseted testing depth for shadowmaps lookups
 */
float shadow_calculate_test_depth_dir(float z, float near, float far, vec3 n, vec3 l) {	
	float d = project_depth_linear(z, near, far);

	float slope = 1.f - abs(dot(l,n));
	
	float df_dx = 1.f / (far - near);
	float delta = df_dx * 1.1f * (1.f + slope * 2.5f);

	return d + delta;
}

float shadow_blocker_search(sampler2DArray directional_shadow_maps, uint idx, vec2 uv) {
	float d = texture(directional_shadow_maps, vec3(uv, idx)).x;
	return d;
}

/*
 *	Shadow lookup and filtering implementation for directional lights.
 */
float shadow_impl(uint idx,
				  vec3 position,
				  vec3 l,
				  vec3 normal,
				  light_cascade_data cascade_data,
				  float light_distance,
				  float light_radius,
				  ivec2 frag_coords,
				  float max_penumbra_multiplier,		// Modulates the maximal allowed penumbra
				  bool override_clusters,				// Overrides the calculated clusters amount
				  int override_clusters_amount) {
	vec3 v = vec4(position, 1) * cascade_data.M;
	vec2 uv = v.xy * .5f + vec2(.5f);

	float depth_blocker = shadow_blocker_search(directional_shadow_maps, idx, uv);
	float dt = shadow_calculate_test_depth_dir(v.z, cascade_projection_near_clip, cascade_data.proj_far_clip, normal, l);

	// No shadowing if distance to blocker is further away from receiver
	if (dt >= depth_blocker)
		return 1.f;
		
	float cascade_space_dist_receiver = -v.z;
	float cascade_space_d_blocker = -unproject_depth_linear(depth_blocker, cascade_projection_near_clip, cascade_data.proj_far_clip);

	// For directional lights, the distance to the receiver is constant. Based on that, adjust the blocker distance.
	float dist_receiver = light_distance;
	float d_blocker = light_distance - (cascade_space_dist_receiver - cascade_space_d_blocker);

	// Calculate penumbra based on distance and light radius
	float penumbra_world = shadow_calculate_penumbra(d_blocker, light_radius, dist_receiver);
	// Transform penumbra size to cascade texture space
	vec2 to_texture_space = .25f * cascade_data.recp_vp;
	vec2 penumbra = clamp(penumbra_world * to_texture_space, 
						  shadow_directional_min_penumbra, 
						  shadow_directional_max_penumbra * max_penumbra_multiplier);
	
	// Calculate number of sampling clusters anisotropically
	float clusters = shadow_clusters_to_sample(penumbra_world, position, normal, l);
	int clusters_to_sample = min(int(ceil(clusters)), shadow_max_clusters);
	if (override_clusters)
		clusters_to_sample = override_clusters_amount;
	
	// Interleaved gradient noise
	float noise = interleaved_gradient_noise(vec2(frag_coords));
	float rcp = 1.f / float(clusters_to_sample);
	
	float accum = texture(directional_shadow_depth_maps, vec4(uv, idx, dt)).x;
	float w = float(clusters_to_sample) * 8.f + 1.f;

	// Accumulate samples
	// Each cluster is rotated around and offseted by the generated interleaved noise
	for (int i=0; i<clusters_to_sample; ++i) {
		float noise_offset = float(i) * rcp;
		float x = two_pi * (noise + noise_offset);
		float sin_noise = sin(x);
		float cos_noise = cos(x);
		mat2 sample_rotation_matrix = mat2(cos_noise,  sin_noise, 
										   -sin_noise, cos_noise);

		// Constant sample pattern per cluster
		// Calculate the offset, lookup shadowmap texture
		for (int s=0; s<8; ++s) {
			vec2 u = penumbra * (sample_rotation_matrix * shadow_cluster_samples[s]);
			accum += texture(directional_shadow_depth_maps, vec4(uv + u, idx, dt)).x;
		}

		// Stop early if fully shadowed
		/*if (i == 1 && 
			accum < 1e-10f)
			break;*/
	}
	return min(1.f, accum / w / shadow_cutoff);
}

/*
 *	Calculate percentage of unshadowed light irradiating in direction and distance given by 'shadow_v' from light.
 *	Filters shadow maps using samples from an interleaved gradient noise pattern, based on penumbra size.
 */
float shadow(uint idx,
			 vec3 position,
			 vec3 l,
			 vec3 normal,
			 light_cascade_data cascade_data,
			 float light_distance,
			 float light_radius,
			 ivec2 frag_coords) {
	return shadow_impl(idx,
					   position,
					   l,
					   normal,
					   cascade_data,
					   light_distance,
					   light_radius,
					   frag_coords,
					   1.f,
					   false,
					   0);
}

/*
 *	Calculate percentage of unshadowed light irradiating in direction and distance given by 'shadow_v' from light.
 *	Filters shadow maps using samples from an interleaved gradient noise pattern, based on penumbra size.
 *	Fast version, filtering is limited to a single cluster with reduced penumbras.
 */
float shadow_fast(uint idx,
				  vec3 position,
				  vec3 l,
				  vec3 normal,
				  light_cascade_data cascade_data,
				  float light_distance,
				  float light_radius,
				  ivec2 frag_coords) {
	return shadow_impl(idx,
					   position,
					   vec3(0,1,0),
					   vec3(0,1,0),
					   cascade_data,
					   light_distance,
					   light_radius,
					   frag_coords,
					   .25f,
					   true,
					   1);
}

/*
 *	Fast unfiltered shadow lookup
 */
float shadow_test(sampler2DArrayShadow directional_shadow_depth_maps, 
				  uint idx, 
				  vec3 position, 
				  vec3 l,
				  vec3 normal,
				  light_cascade_data cascade_data) {
	vec3 v = vec4(position, 1) * cascade_data.M;
	vec2 uv = v.xy * .5f + vec2(.5f);
	
	float dt = shadow_calculate_test_depth_dir(v.z, cascade_projection_near_clip, cascade_data.proj_far_clip, normal, l);

	return texture(directional_shadow_depth_maps, vec4(uv, idx, dt)).x;
}

/*
 *	Lookup unfiltered depth from shadowmap
 */
float shadow_depth(sampler2DArray directional_shadow_maps, uint idx, vec3 position, light_cascade_data cascade_data) {
	vec3 v = vec4(position, 1) * cascade_data.M;
	vec2 uv = v.xy * .5f + vec2(.5f);
	return texture(directional_shadow_maps, vec3(uv, idx)).x;
}

/*
 *	Lookup shadow occluder in shadow_v direction
 *	Filtered, like 'shadow' method but limited to a single cluster and smaller upper limit on penumbra size
 */
vec3 shadow_occluder(sampler2DArray directional_shadow_maps, 
					 uint idx, 
					 vec3 position, 
					 vec3 l,
					 vec3 normal,
					 light_cascade_data cascade_data,
					 float light_distance,
					 float light_radius,
					 ivec2 frag_coords) {
	vec3 v = vec4(position, 1) * cascade_data.M;
	vec2 uv = v.xy * .5f + vec2(.5f);
	float cascade_space_dist_receiver = -v.z;

	float depth_blocker = shadow_blocker_search(directional_shadow_maps, idx, uv);
	float dt = shadow_calculate_test_depth_dir(v.z, cascade_projection_near_clip, cascade_data.proj_far_clip, normal, l);
	
	// No shadowing if distance to blocker is further away from receiver
	if (dt < depth_blocker) {
		float dist_receiver = light_distance;
		float dist_blocker = -unproject_depth_linear(depth_blocker, cascade_projection_near_clip, cascade_data.proj_far_clip);
		vec2 to_texture_space = .5f * cascade_data.recp_vp;
		vec2 penumbra = to_texture_space * shadow_calculate_penumbra(dist_blocker, light_radius, dist_receiver);
		
		float noise = two_pi * interleaved_gradient_noise(vec2(frag_coords));
		float sin_noise = sin(noise);
		float cos_noise = cos(noise);
		mat2 sample_rotation_matrix = mat2(cos_noise,  sin_noise, 
										   -sin_noise, cos_noise);
		
		if (any(greaterThan(penumbra, vec2(shadow_directional_min_penumbra)))) {
			penumbra = clamp(penumbra, shadow_directional_min_penumbra, shadow_directional_max_penumbra);

			float accum = .0f;
			float w = .0f;
			for (int s=0; s<8; ++s) {
				vec2 u = penumbra * sample_rotation_matrix * shadow_cluster_samples[s];
			
				float gaussian = mix(penumbra.x, penumbra.y, .5f);
				float l = length(shadow_cluster_samples[s] * penumbra);
				float t = exp(-l*l / (2 * gaussian * gaussian)) * one_over_pi / (2 * gaussian * gaussian);

				float shadow_sample = texture(directional_shadow_maps, vec3(uv + u, idx)).x;
				accum += shadow_sample * t;
				w += t;
			}

			depth_blocker = accum / w;
		}
	}

	float z = -unproject_depth(depth_blocker, cascade_projection_near_clip);
	return -l * z;
}
