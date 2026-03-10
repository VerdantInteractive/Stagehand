shader_type canvas_item;

varying vec4 instance_custom;

void vertex() {
    instance_custom = INSTANCE_CUSTOM;
}

void fragment() {
    COLOR = instance_custom;
}
