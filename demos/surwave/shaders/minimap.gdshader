shader_type canvas_item;

const int MAX_ENEMY_POINTS = 256;

uniform vec4 background_color : source_color = vec4(0.0, 0.0, 0.0, 1.0);
uniform vec2 minimap_size = vec2(80.0, 80.0);
uniform float opacity = 0.5;

uniform vec2 altar_points[4];
uniform float altar_radius = 3.0;
uniform float altar_states[4];
uniform vec4 altar_available_color : source_color = vec4(0.5, 0.95, 1.0, 1.0);
uniform vec4 altar_spent_color : source_color = vec4(0.2, 0.4, 1.0, 1.0);

uniform vec2 enemy_points[MAX_ENEMY_POINTS];
uniform int enemy_count = 0;
uniform float enemy_radius = 1.0;
uniform vec4 enemy_color : source_color = vec4(0.87, 0.15, 0.15, 1.0);

uniform vec2 player_point = vec2(0.5, 0.5);
uniform float player_radius = 2.0;
uniform float player_visible = 0.0;
uniform vec4 player_color : source_color = vec4(1.0, 1.0, 1.0, 1.0);

uniform vec4 camera_rect = vec4(0.0, 0.0, 1.0, 1.0);
uniform float camera_visible = 0.0;
uniform float camera_line_width = 1.2;
uniform vec4 camera_color : source_color = vec4(0.6, 0.6, 0.6, 1.0);

float disc_mask(vec2 frag, vec2 center, float radius) {
	float distance_to_center = length(frag - center);
	return smoothstep(radius + 0.5, radius - 0.5, distance_to_center);
}

vec2 to_pixels(vec2 point) {
	return point * minimap_size;
}

float camera_outline(vec2 frag) {
	vec2 rect_min = camera_rect.xy * minimap_size;
	vec2 rect_max = (camera_rect.xy + camera_rect.zw) * minimap_size;
	float top = disc_mask(frag, vec2(frag.x, rect_min.y), camera_line_width);
	float bottom = disc_mask(frag, vec2(frag.x, rect_max.y), camera_line_width);
	float left = disc_mask(frag, vec2(rect_min.x, frag.y), camera_line_width);
	float right = disc_mask(frag, vec2(rect_max.x, frag.y), camera_line_width);
	float horizontal_gate = step(rect_min.x, frag.x) * step(frag.x, rect_max.x);
	float vertical_gate = step(rect_min.y, frag.y) * step(frag.y, rect_max.y);
	float horizontal = max(top, bottom) * horizontal_gate;
	float vertical = max(left, right) * vertical_gate;
	return max(horizontal, vertical);
}

void fragment() {
	vec2 frag = vec2(UV.x, UV.y) * minimap_size;
	vec4 color = background_color;

	for (int altar_index = 0; altar_index < 4; altar_index++) {
		vec2 altar_center = to_pixels(altar_points[altar_index]);
		vec2 inset = vec2(altar_radius);
		vec2 max_bounds = max(minimap_size - inset, inset);
		altar_center = clamp(altar_center, inset, max_bounds);
		float altar_mask = disc_mask(frag, altar_center, altar_radius);
		float state_factor = clamp(altar_states[altar_index], 0.0, 1.0);
		vec4 altar_color = mix(altar_available_color, altar_spent_color, state_factor);
		color = mix(color, altar_color, altar_mask * altar_color.a);
	}

	for (int enemy_index = 0; enemy_index < MAX_ENEMY_POINTS; enemy_index++) {
		if (enemy_index >= enemy_count) {
			break;
		}
		vec2 enemy_center = to_pixels(enemy_points[enemy_index]);
		float enemy_mask = disc_mask(frag, enemy_center, enemy_radius);
		color = mix(color, enemy_color, enemy_mask * enemy_color.a);
	}

	if (player_visible > 0.5) {
		vec2 player_center = to_pixels(player_point);
		float player_mask = disc_mask(frag, player_center, player_radius);
		color = mix(color, player_color, player_mask * player_color.a);
	}

	if (camera_visible > 0.5) {
		float outline = camera_outline(frag);
		color = mix(color, camera_color, outline * camera_color.a);
	}

	color.a = opacity;
	COLOR = color;
}
