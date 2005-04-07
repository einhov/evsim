#version 330

uniform float alpha;
out vec4 colour;

void main() {
	colour = vec4(0.0, 0.0, 0.0, alpha);
}
