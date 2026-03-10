shader_type canvas_item;

uniform float radius = 0.0;
uniform float inner_ring = 0.5;
uniform float outer_ring = 0.05;
uniform vec4 shockwave_colour : source_color = vec4(1.0);

const vec2 uv_center = vec2(0.5, 0.5);

void vertex() {
}

void fragment() {
	float outer_edge = 1.0 - smoothstep(radius - outer_ring, radius, length(UV - uv_center));
	float inner_edge = smoothstep(radius - outer_ring - inner_ring, radius - inner_ring, length(UV - uv_center));
	float mask = outer_edge * inner_edge;

	COLOR = vec4(shockwave_colour * mask);
}
