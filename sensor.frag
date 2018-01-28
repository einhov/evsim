#version 330

out vec4 colour;

in vec2 frag_uv;

uniform sampler1D vision;

void main() {
	float magnitude = texture(vision, (frag_uv.s / frag_uv.t) / 2.0 + 0.5).r;
	colour = vec4(mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), magnitude), 1.0);
}
