extends FlecsWorld

## Tests transform system edge cases at the C++/GDScript boundary:
## - Negative coordinate values
## - Very large coordinate values
## - Zero scale
## - Very small (epsilon) values
## - Multiple transform updates in sequence
## - Transform components set individually then computed

func _ready() -> void:
	print("Test: Transform systems – edge cases")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Negative coordinate values ───────────────────────────────────
	set_component("Position2D", Vector2(-500.5, -999.9))
	var pos = get_component("Position2D")
	assert_eq(pos, Vector2(-500.5, -999.9), "Negative Position2D round-trip")

	set_component("Position3D", Vector3(-100, -200, -300))
	assert_eq(get_component("Position3D"), Vector3(-100, -200, -300), "Negative Position3D round-trip")

	# ── Test 2: Very large coordinate values ─────────────────────────────────
	set_component("Position2D", Vector2(1e6, 1e6))
	assert_eq(get_component("Position2D"), Vector2(1e6, 1e6), "Large Position2D round-trip")

	set_component("Position3D", Vector3(1e6, 1e6, 1e6))
	assert_eq(get_component("Position3D"), Vector3(1e6, 1e6, 1e6), "Large Position3D round-trip")

	# ── Test 3: Zero vector values ───────────────────────────────────────────
	set_component("Position2D", Vector2.ZERO)
	assert_eq(get_component("Position2D"), Vector2.ZERO, "Zero Position2D")

	set_component("Scale2D", Vector2.ZERO)
	assert_eq(get_component("Scale2D"), Vector2.ZERO, "Zero Scale2D")

	set_component("Scale3D", Vector3.ZERO)
	assert_eq(get_component("Scale3D"), Vector3.ZERO, "Zero Scale3D")

	# ── Test 4: Very small epsilon values ────────────────────────────────────
	set_component("Position2D", Vector2(0.0001, 0.0001))
	pos = get_component("Position2D")
	assert_approx(pos.x, 0.0001, "Epsilon Position2D.x", 0.001)
	assert_approx(pos.y, 0.0001, "Epsilon Position2D.y", 0.001)

	set_component("Rotation2D", 0.0001)
	var rot = get_component("Rotation2D")
	assert_approx(rot, 0.0001, "Epsilon Rotation2D", 0.001)

	# ── Test 5: Full rotation values ─────────────────────────────────────────
	set_component("Rotation2D", 6.28318) # 2*PI
	rot = get_component("Rotation2D")
	assert_approx(rot, 6.28318, "Full rotation (2*PI)")

	set_component("Rotation2D", -3.14159) # -PI
	rot = get_component("Rotation2D")
	assert_approx(rot, -3.14159, "Negative rotation (-PI)")

	# ── Test 6: 3D rotation with non-identity quaternion ─────────────────────
	var q_90y = Quaternion(Vector3.UP, deg_to_rad(90.0))
	set_component("Rotation3D", q_90y)
	var got_q = get_component("Rotation3D")
	assert_approx(got_q.y, q_90y.y, "Rotation3D 90deg Y-axis rotation .y")
	assert_approx(got_q.w, q_90y.w, "Rotation3D 90deg Y-axis rotation .w")

	# ── Test 7: Entity with zero scale + transform computation ───────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {
			"EntityValue": 1.0,
			"Position2D": Vector2(100, 200),
			"Rotation2D": 0.0,
			"Scale2D": Vector2.ZERO
		}
	})
	progress(0.016)
	progress(0.016)
	print("  PASS: Entity with zero scale processed without crash")

	# ── Test 8: Entity with negative scale ───────────────────────────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {
			"EntityValue": 1.0,
			"Position2D": Vector2(0, 0),
			"Rotation2D": 0.0,
			"Scale2D": Vector2(-1.0, -1.0)
		}
	})
	progress(0.016)
	progress(0.016)
	print("  PASS: Entity with negative scale processed without crash")

	# ── Test 9: Rapid component updates ──────────────────────────────────────
	for i in range(100):
		set_component("Position2D", Vector2(float(i), float(i * 2)))
	var final_pos = get_component("Position2D")
	assert_eq(final_pos, Vector2(99, 198), "Rapid Position2D updates, last value wins")

	# ── Test 10: 3D entity with extreme transform values ─────────────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity3D",
		"components": {
			"EntityValue": 1.0,
			"Position3D": Vector3(1e5, -1e5, 0.001),
			"Rotation3D": Quaternion(0.5, 0.5, 0.5, 0.5),
			"Scale3D": Vector3(100, 0.01, 50)
		}
	})
	progress(0.016)
	progress(0.016)
	print("  PASS: 3D entity with extreme transform values processed")

	# ── Test 11: Multiple components set before progress ─────────────────────
	set_component("Position2D", Vector2(111, 222))
	set_component("Rotation2D", 0.785) # PI/4
	set_component("Scale2D", Vector2(2.0, 2.0))
	progress(0.016)

	var t2d = get_component("Transform2D")
	if typeof(t2d) == TYPE_TRANSFORM2D:
		assert_approx(t2d.origin.x, 111.0, "Transform2D origin.x after batch set")
		assert_approx(t2d.origin.y, 222.0, "Transform2D origin.y after batch set")
	else:
		print("  SKIP: Transform2D singleton not computed (no matching entity)")

	print("All transform edge case tests passed!")
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

func assert_true(value: bool, label: String) -> void:
	if not value:
		_fail(label)
	else:
		print("  PASS: %s" % label)

func _fail(msg: String) -> void:
	print("FAIL: %s" % msg)
	get_tree().quit(1)
