shader_type canvas_item;
render_mode unshaded;

uniform vec4 cell_color : source_color;

void fragment() {
	float value = texture(TEXTURE, UV).r;
	COLOR = value * cell_color;
}
