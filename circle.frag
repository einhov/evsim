#version 330

out vec4 colour;

in vec2 uv;

uniform vec3 box_colour;

void main() {
	if(length(uv) > 1.0)
		discard;
	colour = vec4(box_colour, 1.0);
}
