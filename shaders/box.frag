#version 330

out vec4 colour;

uniform vec3 box_colour;

void main() {
	colour = vec4(box_colour, 1.0);
}
