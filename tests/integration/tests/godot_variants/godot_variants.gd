extends FlecsWorld

## Tests the built-in Godot Variant component types that are registered
## by stagehand directly (Color, Vector2, Vector3, etc.) as singletons.
## These are separate from the GODOT_VARIANT macro-defined components in
## the test module.

func _ready() -> void:
	print("Test: Built-in Godot Variant types as ECS singletons")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Color ─────────────────────────────────────────────────────────────────
	set_component("Color", Color(0.25, 0.5, 0.75, 1.0))
	var col = get_component("Color")
	assert_eq(typeof(col), TYPE_COLOR, "Color type")
	assert_eq(col, Color(0.25, 0.5, 0.75, 1.0), "Color value")

	# ── Vector2 ───────────────────────────────────────────────────────────────
	set_component("Vector2", Vector2(11.5, 22.5))
	assert_eq(get_component("Vector2"), Vector2(11.5, 22.5), "Vector2 value")

	# ── Vector2i ──────────────────────────────────────────────────────────────
	set_component("Vector2i", Vector2i(-3, 7))
	assert_eq(get_component("Vector2i"), Vector2i(-3, 7), "Vector2i value")

	# ── Vector3 ───────────────────────────────────────────────────────────────
	set_component("Vector3", Vector3(1.1, 2.2, 3.3))
	var v3 = get_component("Vector3")
	assert_approx(v3.x, 1.1, "Vector3.x")
	assert_approx(v3.y, 2.2, "Vector3.y")
	assert_approx(v3.z, 3.3, "Vector3.z")

	# ── Vector3i ──────────────────────────────────────────────────────────────
	set_component("Vector3i", Vector3i(10, 20, 30))
	assert_eq(get_component("Vector3i"), Vector3i(10, 20, 30), "Vector3i value")

	# ── Vector4 ───────────────────────────────────────────────────────────────
	set_component("Vector4", Vector4(1, 2, 3, 4))
	assert_eq(get_component("Vector4"), Vector4(1, 2, 3, 4), "Vector4 value")

	# ── Vector4i ──────────────────────────────────────────────────────────────
	set_component("Vector4i", Vector4i(5, 6, 7, 8))
	assert_eq(get_component("Vector4i"), Vector4i(5, 6, 7, 8), "Vector4i value")

	# ── Rect2 ─────────────────────────────────────────────────────────────────
	set_component("Rect2", Rect2(1, 2, 100, 200))
	assert_eq(get_component("Rect2"), Rect2(1, 2, 100, 200), "Rect2 value")

	# ── Rect2i ────────────────────────────────────────────────────────────────
	set_component("Rect2i", Rect2i(0, 0, 64, 64))
	assert_eq(get_component("Rect2i"), Rect2i(0, 0, 64, 64), "Rect2i value")

	# ── Plane ─────────────────────────────────────────────────────────────────
	set_component("Plane", Plane(Vector3(0, 1, 0), 5.0))
	var plane = get_component("Plane")
	assert_eq(typeof(plane), TYPE_PLANE, "Plane type")
	assert_eq(plane.normal, Vector3(0, 1, 0), "Plane normal")
	assert_approx(plane.d, 5.0, "Plane.d")

	# ── Quaternion ────────────────────────────────────────────────────────────
	set_component("Quaternion", Quaternion(0, 0, 0, 1))
	var q = get_component("Quaternion")
	assert_eq(typeof(q), TYPE_QUATERNION, "Quaternion type")
	assert_approx(q.w, 1.0, "Quaternion.w (identity)")

	# ── Basis ─────────────────────────────────────────────────────────────────
	var basis = Basis.IDENTITY
	set_component("Basis", basis)
	var b = get_component("Basis")
	assert_eq(typeof(b), TYPE_BASIS, "Basis type")
	assert_eq(b, Basis.IDENTITY, "Basis identity")

	# ── Transform2D ──────────────────────────────────────────────────────────
	var t2d = Transform2D(1.0, Vector2(50, 100))
	set_component("Transform2D", t2d)
	var got2d = get_component("Transform2D")
	assert_eq(typeof(got2d), TYPE_TRANSFORM2D, "Transform2D type")
	assert_approx(got2d.origin.x, 50.0, "Transform2D origin.x")
	assert_approx(got2d.origin.y, 100.0, "Transform2D origin.y")

	# ── Transform3D ──────────────────────────────────────────────────────────
	var t3d = Transform3D(Basis.IDENTITY, Vector3(7, 8, 9))
	set_component("Transform3D", t3d)
	var got3d = get_component("Transform3D")
	assert_eq(typeof(got3d), TYPE_TRANSFORM3D, "Transform3D type")
	assert_eq(got3d.origin, Vector3(7, 8, 9), "Transform3D origin")

	# ── AABB ──────────────────────────────────────────────────────────────────
	set_component("AABB", AABB(Vector3(0, 0, 0), Vector3(10, 10, 10)))
	var aabb = get_component("AABB")
	assert_eq(typeof(aabb), TYPE_AABB, "AABB type")
	assert_eq(aabb.position, Vector3(0, 0, 0), "AABB position")
	assert_eq(aabb.size, Vector3(10, 10, 10), "AABB size")

	# ── Projection ────────────────────────────────────────────────────────────
	var proj = Projection.IDENTITY
	set_component("Projection", proj)
	var got_proj = get_component("Projection")
	assert_eq(typeof(got_proj), TYPE_PROJECTION, "Projection type")

	# ── Array ─────────────────────────────────────────────────────────────────
	set_component("Array", [1, "two", 3.0])
	var arr = get_component("Array")
	assert_eq(typeof(arr), TYPE_ARRAY, "Array type")
	assert_eq(arr.size(), 3, "Array size")
	assert_eq(arr[1], "two", "Array[1]")

	# ── Dictionary ────────────────────────────────────────────────────────────
	set_component("Dictionary", {"a": 1, "b": 2})
	var dict = get_component("Dictionary")
	assert_eq(typeof(dict), TYPE_DICTIONARY, "Dictionary type")
	assert_eq(dict["a"], 1, "Dictionary[a]")

	# ── String ────────────────────────────────────────────────────────────────
	set_component("String", "test_string_value")
	assert_eq(get_component("String"), "test_string_value", "String value")

	# ── StringName ────────────────────────────────────────────────────────────
	set_component("StringName", &"test_string_name")
	var sn = get_component("StringName")
	assert_eq(str(sn), "test_string_name", "StringName value")

	# ── NodePath ──────────────────────────────────────────────────────────────
	set_component("NodePath", NodePath("some/node/path"))
	var np = get_component("NodePath")
	assert_eq(typeof(np), TYPE_NODE_PATH, "NodePath type")
	assert_eq(str(np), "some/node/path", "NodePath value")

	print("All built-in Godot Variant tests passed!")
	get_tree().quit(0)


# ── Assertion helpers ─────────────────────────────────────────────────────────

func assert_eq(actual, expected, label: String) -> void:
	if actual != expected:
		_fail("%s: expected %s, got %s" % [label, str(expected), str(actual)])
	else:
		print("  PASS: %s" % label)

func assert_approx(actual: float, expected: float, label: String, epsilon: float = 0.01) -> void:
	if abs(actual - expected) > epsilon:
		_fail("%s: expected ~%s, got %s" % [label, str(expected), str(actual)])
	else:
		print("  PASS: %s" % label)

func _fail(msg: String) -> void:
	print("FAIL: %s" % msg)
	get_tree().quit(1)
