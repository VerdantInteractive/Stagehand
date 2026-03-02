extends FlecsWorld

## Tests physics body creation, space assignment, and basic lifecycle.
## Verifies that PhysicsBodyType → PhysicsBodyRID hooks work,
## bodies are assigned to spaces, and cleanup happens on destruction.

func _ready() -> void:
	print("Test: Physics body lifecycle and space assignment")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Create a 3D static physics body ─────────────────────────
	var entity_id = instantiate_prefab("stagehand_tests::PhysicsStatic3D", {
		"Position3D": Vector3(10, 20, 30),
	})
	assert_true(entity_id > 0, "Static3D entity created")
	assert_true(is_alive(entity_id), "Static3D entity is alive")

	# Progress to run body space assignment system
	progress(0.016)
	progress(0.016)

	# Query physics bodies to verify state
	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsStatic3D"
	})
	var bodies_result = get_component("SceneChildrenResult")
	assert_true(bodies_result is Dictionary, "Query Physics Bodies returned Dictionary")
	assert_eq(int(bodies_result.get("count", 0)), 1, "One static 3D body exists")

	var bodies = bodies_result.get("bodies", [])
	if bodies.size() > 0:
		var body = bodies[0]
		assert_true(body.get("rid_valid", false), "Static3D body has valid RID")
		assert_true(body.get("in_space", false), "Static3D body is in space")
		assert_eq(int(body.get("body_type", -1)), 4, "Body type is Static3D (4)")

	# ── Test 2: Verify physics space was created ─────────────────────────
	run_system("stagehand_tests::Query Physics Spaces", {})
	var spaces_result = get_component("SceneChildrenResult")
	assert_true(spaces_result.get("has_space_3d", false), "PhysicsSpace3D singleton exists")
	assert_true(spaces_result.get("space_3d_valid", false), "PhysicsSpace3D has valid RID")
	assert_true(spaces_result.get("space_3d_owned", false), "PhysicsSpace3D is owned (auto-created)")

	# ── Test 3: Create a 3D rigid body ───────────────────────────────────
	var rigid_id = instantiate_prefab("stagehand_tests::PhysicsRigid3D", {
		"Position3D": Vector3(0, 50, 0),
	})
	assert_true(rigid_id > 0, "Rigid3D entity created")
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsRigid3D"
	})
	bodies_result = get_component("SceneChildrenResult")
	assert_eq(int(bodies_result.get("count", 0)), 1, "One rigid 3D body exists")
	bodies = bodies_result.get("bodies", [])
	if bodies.size() > 0:
		assert_true(bodies[0].get("rid_valid", false), "Rigid3D body has valid RID")
		assert_true(bodies[0].get("in_space", false), "Rigid3D body is in space")
		assert_eq(int(bodies[0].get("body_type", -1)), 6, "Body type is Rigid3D (6)")
		assert_true(bodies[0].get("has_collision_layer", false), "Rigid3D has collision layer")
		assert_true(bodies[0].get("has_collision_mask", false), "Rigid3D has collision mask")
		assert_eq(int(bodies[0].get("collision_layer", 0)), 1, "Default collision layer is 1")
		assert_eq(int(bodies[0].get("collision_mask", 0)), 1, "Default collision mask is 1")

	# ── Test 4: Create a 3D kinematic body ───────────────────────────────
	var kinematic_id = instantiate_prefab("stagehand_tests::PhysicsKinematic3D", {
		"Position3D": Vector3(100, 0, 0),
	})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsKinematic3D"
	})
	bodies_result = get_component("SceneChildrenResult")
	assert_eq(int(bodies_result.get("count", 0)), 1, "One kinematic 3D body exists")
	bodies = bodies_result.get("bodies", [])
	if bodies.size() > 0:
		assert_true(bodies[0].get("rid_valid", false), "Kinematic3D body has valid RID")
		assert_true(bodies[0].get("in_space", false), "Kinematic3D body is in space")
		assert_eq(int(bodies[0].get("body_type", -1)), 5, "Body type is Kinematic3D (5)")

	# ── Test 5: Destroy entity and verify cleanup ────────────────────────
	destroy_entity(entity_id)
	progress(0.016)

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsStatic3D"
	})
	bodies_result = get_component("SceneChildrenResult")
	assert_eq(int(bodies_result.get("count", 0)), 0, "Static 3D body destroyed")

	# ── Test 6: Multiple physics bodies of same type ─────────────────────
	for i in range(5):
		instantiate_prefab("stagehand_tests::PhysicsStatic3D", {
			"Position3D": Vector3(float(i) * 10, 0, 0),
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsStatic3D"
	})
	bodies_result = get_component("SceneChildrenResult")
	assert_eq(int(bodies_result.get("count", 0)), 5, "Five static 3D bodies exist")

	# Verify all have valid RIDs and are in space
	var all_valid = true
	var all_in_space = true
	for body in bodies_result.get("bodies", []):
		if not body.get("rid_valid", false):
			all_valid = false
		if not body.get("in_space", false):
			all_in_space = false
	assert_true(all_valid, "All 5 static bodies have valid RIDs")
	assert_true(all_in_space, "All 5 static bodies are in space")

	print("All physics body lifecycle tests passed!")
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

func assert_approx(actual: float, expected: float, label: String, epsilon: float = 0.01) -> void:
	if abs(actual - expected) > epsilon:
		_fail("%s: expected ~%s, got %s" % [label, str(expected), str(actual)])
	else:
		print("  PASS: %s" % label)

func _fail(msg: String) -> void:
	print("FAIL: %s" % msg)
	get_tree().quit(1)
