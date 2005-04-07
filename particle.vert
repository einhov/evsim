#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec2 instance_position;
layout(location = 3) in vec3 colour;

out vec2 frag_uv;
out vec3 frag_colour;

uniform mat4 projection;

void main() {
	frag_colour = colour;
	frag_uv = uv;
	gl_Position = projection * vec4(position + instance_position, 0.0, 1.0);
}
