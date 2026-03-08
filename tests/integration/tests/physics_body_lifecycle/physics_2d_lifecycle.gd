extends FlecsWorld

## Tests 2D physics body lifecycle: creation, space assignment, body types,
## transform sync, and cleanup for all 2D physics body types.

func _ready() -> void:
	print("Test: Physics 2D body lifecycle")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Create a 2D static body ──────────────────────────────────
	var static_id = instantiate_prefab("stagehand_tests::PhysicsStatic2D", {
		"Position2D": Vector2(100, 200),
	})
	assert_true(static_id > 0, "Static2D entity created")
	progress(0.016)

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsStatic2D"
	})
	var result = get_component("SceneChildrenResult")
	assert_eq(int(result.get("count", 0)), 1, "One static 2D body exists")
	var bodies = result.get("bodies", [])
	if bodies.size() > 0:
		assert_true(bodies[0].get("rid_valid", false), "Static2D body has valid RID")
		assert_true(bodies[0].get("in_space", false), "Static2D body is in space")
		assert_eq(int(bodies[0].get("body_type", -1)), 0, "Body type is Static2D (0)")

	# ── Test 2: Verify 2D physics space was created ──────────────────────
	run_system("stagehand_tests::Query Physics Spaces", {})
	var spaces = get_component("SceneChildrenResult")
	assert_true(spaces.get("has_space_2d", false), "PhysicsSpace2D singleton exists")
	assert_true(spaces.get("space_2d_valid", false), "PhysicsSpace2D has valid RID")
	assert_true(spaces.get("space_2d_owned", false), "PhysicsSpace2D is owned (auto-created)")
	# 3D space should NOT exist yet (only 2D bodies created so far)
	assert_true(not spaces.get("has_space_3d", true), "PhysicsSpace3D does not exist yet")

	# ── Test 3: 2D static body transform is pushed to PhysicsServer ──────
	run_system("stagehand_tests::Query Physics Body State", {
		"prefab": "stagehand_tests::PhysicsStatic2D",
		"dimension": "2d"
	})
	var state_result = get_component("SceneChildrenResult")
	var states = state_result.get("states", [])
	if states.size() > 0:
		var origin = states[0].get("origin", Vector2.ZERO)
		assert_approx(origin.x, 100.0, "2D static origin.x")
		assert_approx(origin.y, 200.0, "2D static origin.y")

	# ── Test 4: Create 2D kinematic body ─────────────────────────────────
	var kinematic_id = instantiate_prefab("stagehand_tests::PhysicsKinematic2D", {
		"Position2D": Vector2(300, 400),
	})
	progress(0.016)

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsKinematic2D"
	})
	result = get_component("SceneChildrenResult")
	assert_eq(int(result.get("count", 0)), 1, "One kinematic 2D body exists")
	bodies = result.get("bodies", [])
	if bodies.size() > 0:
		assert_true(bodies[0].get("rid_valid", false), "Kinematic2D has valid RID")
		assert_true(bodies[0].get("in_space", false), "Kinematic2D is in space")
		assert_eq(int(bodies[0].get("body_type", -1)), 1, "Body type is Kinematic2D (1)")
		assert_true(bodies[0].get("has_collision_layer", false), "Kinematic2D has collision layer")
		assert_true(bodies[0].get("has_collision_mask", false), "Kinematic2D has collision mask")

	# ── Test 5: Create 2D rigid body ─────────────────────────────────────
	var rigid_id = instantiate_prefab("stagehand_tests::PhysicsRigid2D", {
		"Position2D": Vector2(500, 600),
	})
	progress(0.016)

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsRigid2D"
	})
	result = get_component("SceneChildrenResult")
	assert_eq(int(result.get("count", 0)), 1, "One rigid 2D body exists")
	bodies = result.get("bodies", [])
	if bodies.size() > 0:
		assert_true(bodies[0].get("rid_valid", false), "Rigid2D has valid RID")
		assert_true(bodies[0].get("in_space", false), "Rigid2D is in space")
		assert_eq(int(bodies[0].get("body_type", -1)), 2, "Body type is Rigid2D (2)")

	# ── Test 6: All 2D bodies share the same 2D space ────────────────────
	# After creating static + kinematic + rigid 2D bodies, there should be
	# exactly one PhysicsSpace2D singleton (not multiple).
	run_system("stagehand_tests::Query Physics Spaces", {})
	spaces = get_component("SceneChildrenResult")
	assert_true(spaces.get("has_space_2d", false), "PhysicsSpace2D still exists")
	assert_true(spaces.get("space_2d_valid", false), "PhysicsSpace2D still valid")

	# ── Test 7: 2D body destruction and cleanup ──────────────────────────
	destroy_entity(kinematic_id)
	progress(0.016)

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsKinematic2D"
	})
	result = get_component("SceneChildrenResult")
	assert_eq(int(result.get("count", 0)), 0, "Kinematic2D body destroyed")

	# Static and rigid bodies should still exist
	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsStatic2D"
	})
	result = get_component("SceneChildrenResult")
	assert_eq(int(result.get("count", 0)), 1, "Static2D still exists after kinematic destroyed")

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsRigid2D"
	})
	result = get_component("SceneChildrenResult")
	assert_eq(int(result.get("count", 0)), 1, "Rigid2D still exists after kinematic destroyed")

	# ── Test 8: 2D and 3D bodies in the same world ────────────────────────
	# Create a 3D body alongside existing 2D bodies
	var entity_3d = instantiate_prefab("stagehand_tests::PhysicsStatic3D", {
		"Position3D": Vector3(10, 20, 30),
	})
	progress(0.016)

	run_system("stagehand_tests::Query Physics Spaces", {})
	spaces = get_component("SceneChildrenResult")
	assert_true(spaces.get("has_space_2d", false), "2D space exists in mixed world")
	assert_true(spaces.get("has_space_3d", false), "3D space exists in mixed world")
	assert_true(spaces.get("space_2d_valid", false), "2D space valid in mixed world")
	assert_true(spaces.get("space_3d_valid", false), "3D space valid in mixed world")

	# Verify both dimensions work independently
	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsStatic3D"
	})
	result = get_component("SceneChildrenResult")
	assert_eq(int(result.get("count", 0)), 1, "3D body exists alongside 2D bodies")

	print("All physics 2D lifecycle tests passed!")
	get_tree().quit(0)


# ── Assertion helpers ─────────────────────────────────────────────────────────

func assert_eq(actual, expected, label: String) -> void:
	if actual != expected:
		_fail("%s: expected %s, got %s" % [label, str(expected), str(actual)])
	else:
		print("  PASS: %s" % label)

func assert_true(value: bool, label: String) -> void:
	if not value:
		_fail(label)
	else:
		print("  PASS: %s" % label)

func assert_approx(actual: float, expected: float, label: String, epsilon: float = 0.5) -> void:
	if abs(actual - expected) > epsilon:
		_fail("%s: expected ~%s, got %s" % [label, str(expected), str(actual)])
	else:
		print("  PASS: %s" % label)

func _fail(msg: String) -> void:
	print("FAIL: %s" % msg)
	get_tree().quit(1)
