shader_type canvas_item;

instance uniform float speed : hint_range(-0.1, 0.1, 0.0001) = 0.002;

void fragment() {
	vec2 new_uv = UV;
	new_uv.x = fract(UV.x + (TIME * speed));
	COLOR = texture(TEXTURE, new_uv);
}
