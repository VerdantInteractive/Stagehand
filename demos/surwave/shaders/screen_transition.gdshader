shader_type canvas_item;

uniform vec4 transition_colour: source_color;
uniform sampler2D transition_texture : filter_nearest;
uniform float progress: hint_range(0.0, 1.0);

void fragment() {
	vec4 sampled_transition_colour = texture(transition_texture, UV);
	if (sampled_transition_colour.r > progress) {
		COLOR.a = 0.0;
	}
	else {
		COLOR = transition_colour;
	}
}
