shader_type spatial;

instance uniform vec4 InstanceColour;

void vertex() {
	// Called for every vertex the material is visible on.
}

void fragment() {
	ALBEDO = InstanceColour.rgb;
}

//void light() {
//	// Called for every pixel for every light affecting the material.
//	// Uncomment to replace the default light processing function with this one.
//}
