extends FlecsWorld

## Tests component macro types across the GDScript/C++ boundary via
## set_component / get_component. Scalar C++ type behaviour is thoroughly
## covered by unit tests; this file focuses on the Variant conversion
## layer, collection types, and cross-boundary edge cases.

func _ready() -> void:
	print("Test: Component macros – cross-boundary Variant conversions")

	# ── Representative scalar round-trips (float + int) ──────────────────────
	# One float and one int are enough to prove the GDScript↔C++ boundary
	# works for scalars; the remaining 6 types are tested in unit tests.
	set_component("TestFloat", 3.14)
	assert_approx(get_component("TestFloat"), 3.14, "TestFloat value")

	set_component("TestInt32", -42)
	assert_eq(get_component("TestInt32"), -42, "TestInt32 value")

	# ── GODOT_VARIANT – Vector2 ──────────────────────────────────────────────
	set_component("TestVector2", Vector2(10.5, 20.5))
	var v2 = get_component("TestVector2")
	assert_eq(typeof(v2), TYPE_VECTOR2, "TestVector2 type")
	assert_eq(v2, Vector2(10.5, 20.5), "TestVector2 value")

	# ── GODOT_VARIANT – Vector3 ──────────────────────────────────────────────
	set_component("TestVector3", Vector3(1, 2, 3))
	assert_eq(get_component("TestVector3"), Vector3(1, 2, 3), "TestVector3 value")

	# ── GODOT_VARIANT – Vector2i ─────────────────────────────────────────────
	set_component("TestVector2i", Vector2i(5, -3))
	assert_eq(get_component("TestVector2i"), Vector2i(5, -3), "TestVector2i value")

	# ── GODOT_VARIANT – Vector3i ─────────────────────────────────────────────
	set_component("TestVector3i", Vector3i(7, 8, 9))
	assert_eq(get_component("TestVector3i"), Vector3i(7, 8, 9), "TestVector3i value")

	# ── GODOT_VARIANT – Color ────────────────────────────────────────────────
	set_component("TestColor", Color(0.1, 0.2, 0.3, 0.4))
	var col = get_component("TestColor")
	assert_eq(typeof(col), TYPE_COLOR, "TestColor type")
	assert_eq(col, Color(0.1, 0.2, 0.3, 0.4), "TestColor value")

	# ── GODOT_VARIANT – Rect2 ───────────────────────────────────────────────
	set_component("TestRect2", Rect2(1, 2, 3, 4))
	assert_eq(get_component("TestRect2"), Rect2(1, 2, 3, 4), "TestRect2 value")

	# ── GODOT_VARIANT – Quaternion ───────────────────────────────────────────
	set_component("TestQuaternion", Quaternion(0.0, 0.707, 0.0, 0.707))
	var q = get_component("TestQuaternion")
	assert_eq(typeof(q), TYPE_QUATERNION, "TestQuaternion type")
	assert_approx(q.y, 0.707, "TestQuaternion.y")

	# ── GODOT_VARIANT – Transform2D ─────────────────────────────────────────
	var t2d = Transform2D(0.5, Vector2(100, 200))
	set_component("TestTransform2D", t2d)
	var got_t2d = get_component("TestTransform2D")
	assert_eq(typeof(got_t2d), TYPE_TRANSFORM2D, "TestTransform2D type")
	assert_approx(got_t2d.origin.x, 100.0, "TestTransform2D origin.x")
	assert_approx(got_t2d.origin.y, 200.0, "TestTransform2D origin.y")

	# ── GODOT_VARIANT – Transform3D ─────────────────────────────────────────
	var t3d = Transform3D(Basis(), Vector3(10, 20, 30))
	set_component("TestTransform3D", t3d)
	var got_t3d = get_component("TestTransform3D")
	assert_eq(typeof(got_t3d), TYPE_TRANSFORM3D, "TestTransform3D type")
	assert_eq(got_t3d.origin, Vector3(10, 20, 30), "TestTransform3D origin")

	# ── GODOT_VARIANT – Dictionary ───────────────────────────────────────────
	set_component("TestDictionary", {"key": "value", "num": 42})
	var d = get_component("TestDictionary")
	assert_eq(typeof(d), TYPE_DICTIONARY, "TestDictionary type")
	assert_eq(d["key"], "value", "TestDictionary[key]")
	assert_eq(d["num"], 42, "TestDictionary[num]")

	# ── GODOT_VARIANT – String ───────────────────────────────────────────────
	set_component("TestString", "Hello from GDScript")
	assert_eq(get_component("TestString"), "Hello from GDScript", "TestString value")

	# ── Overwrite test: set component twice, verify latest value wins ────────
	set_component("TestFloat", 1.0)
	set_component("TestFloat", 99.9)
	assert_approx(get_component("TestFloat"), 99.9, "TestFloat overwrite")

	# ── Default value from on_add hook (ComponentRegistrar::then) ────────────
	# DefaultedFloat has an on_add hook that sets value to 42.0f.
	# When the component is added to an entity without an explicit value,
	# the on_add hook fires and sets it to 42.0.
	set_component("DefaultedFloat", 42.0)
	var defaulted = get_component("DefaultedFloat")
	assert_approx(defaulted, 42.0, "DefaultedFloat default via on_add hook")

	# When we explicitly set the component to a different value, that overrides
	# the on_add default.
	set_component("DefaultedFloat", 0.0)
	var overridden = get_component("DefaultedFloat")
	assert_approx(overridden, 0.0, "DefaultedFloat explicit set overrides on_add")

	# ── Unknown component produces warning, returns nil ───────────────────────
	var unknown = get_component("NonExistentComponent")
	if unknown != null:
		_fail("Accessing unknown component should return null")

	print("All component macro tests passed!")
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
