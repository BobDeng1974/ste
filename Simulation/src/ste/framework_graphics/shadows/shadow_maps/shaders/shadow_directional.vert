
#type vert
#version 450
#extension GL_ARB_shader_draw_parameters : enable

#include <mesh_descriptor.glsl>
#include <renderer_transform_buffers.glsl>

layout(location = 0) in vec4 tangent_frame_quat;
layout(location = 1) in vec3 vert;
layout(location = 2) in vec2 tex_coords;

layout(location = 0) out vs_out {
	vec3 position;
	flat int instanceIdx;
	flat uint drawIdx;
} vout;

void main() {
	uint draw_id = gl_BaseInstanceARB;
	mesh_descriptor md = mesh_descriptor_buffer[draw_id];
	
	vout.position = transform_view(transform_model(md, vert));
	vout.instanceIdx = gl_InstanceIndex;
	vout.drawIdx = draw_id;
}
