#version 330

layout(location = 0) in vec2 position;
layout(location = 2) in vec2 instance_position;

uniform mat4 projection;

void main() {
	gl_Position = projection * vec4(position, 0.0, 1.0);
}
