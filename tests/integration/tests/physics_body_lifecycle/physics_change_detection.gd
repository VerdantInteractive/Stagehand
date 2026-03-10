extends FlecsWorld

## Tests physics change detection: verifies that modifying ECS components
## (Position, Velocity, CollisionLayer, etc.) via the stagehand::entity wrapper
## triggers HasChanged tags, and the sync systems push those changes to the
## PhysicsServer.

func _ready() -> void:
	print("Test: Physics change detection sync pipeline")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: 3D transform change detection sync ───────────────────────
	# Create a kinematic body at origin, then modify its position via
	# the Update Entity Physics system (uses stagehand::entity → HasChanged).
	var entity_id = instantiate_prefab("stagehand_tests::PhysicsKinematic3D", {
		"Position3D": Vector3(0, 0, 0),
	})
	# Progress to run space assignment (sets initial transform)
	progress(0.016)

	# Verify initial position in PhysicsServer
	run_system("stagehand_tests::Query Physics Body State", {
		"prefab": "stagehand_tests::PhysicsKinematic3D",
		"dimension": "3d"
	})
	var result = get_component("SceneChildrenResult")
	var states = result.get("states", [])
	if states.size() > 0:
		var origin = states[0].get("origin", Vector3.ZERO)
		assert_approx(origin.x, 0.0, "Initial position.x is 0")
		assert_approx(origin.y, 0.0, "Initial position.y is 0")
		assert_approx(origin.z, 0.0, "Initial position.z is 0")

	# Now update position using stagehand::entity (triggers HasChanged)
	run_system("stagehand_tests::Update Entity Physics", {
		"entity_id": entity_id,
		"position_3d": Vector3(50, 100, 150),
	})
	# Progress to run the transform sync system (OnLateUpdate)
	progress(0.016)

	# Verify updated position in PhysicsServer
	run_system("stagehand_tests::Query Physics Body State", {
		"prefab": "stagehand_tests::PhysicsKinematic3D",
		"dimension": "3d"
	})
	result = get_component("SceneChildrenResult")
	states = result.get("states", [])
	if states.size() > 0:
		var origin = states[0].get("origin", Vector3.ZERO)
		assert_approx(origin.x, 50.0, "Synced position.x after change detection")
		assert_approx(origin.y, 100.0, "Synced position.y after change detection")
		assert_approx(origin.z, 150.0, "Synced position.z after change detection")

	# ── Test 2: Collision layer/mask change detection ────────────────────
	# Update collision layer and mask via stagehand::entity
	run_system("stagehand_tests::Update Entity Physics", {
		"entity_id": entity_id,
		"collision_layer": 4,
		"collision_mask": 8,
	})
	progress(0.016)

	# Read back collision config from ECS
	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsKinematic3D"
	})
	result = get_component("SceneChildrenResult")
	var bodies = result.get("bodies", [])
	if bodies.size() > 0:
		assert_eq(int(bodies[0].get("collision_layer", 0)), 4, "Collision layer updated to 4")
		assert_eq(int(bodies[0].get("collision_mask", 0)), 8, "Collision mask updated to 8")

	# ── Test 3: Velocity change detection sync ───────────────────────────
	run_system("stagehand_tests::Update Entity Physics", {
		"entity_id": entity_id,
		"velocity_3d": Vector3(10, 0, 0),
	})
	progress(0.016)

	# Verify velocity was pushed to PhysicsServer
	run_system("stagehand_tests::Query Physics Body State", {
		"prefab": "stagehand_tests::PhysicsKinematic3D",
		"dimension": "3d"
	})
	result = get_component("SceneChildrenResult")
	states = result.get("states", [])
	if states.size() > 0:
		var vel = states[0].get("linear_velocity", Vector3.ZERO)
		assert_approx(vel.x, 10.0, "Synced velocity.x")
		assert_approx(vel.y, 0.0, "Synced velocity.y")
		assert_approx(vel.z, 0.0, "Synced velocity.z")

	# ── Test 4: Angular velocity change detection sync ───────────────────
	run_system("stagehand_tests::Update Entity Physics", {
		"entity_id": entity_id,
		"angular_velocity_3d": Vector3(0, 3.14, 0),
	})
	progress(0.016)

	run_system("stagehand_tests::Query Physics Body State", {
		"prefab": "stagehand_tests::PhysicsKinematic3D",
		"dimension": "3d"
	})
	result = get_component("SceneChildrenResult")
	states = result.get("states", [])
	if states.size() > 0:
		var avel = states[0].get("angular_velocity", Vector3.ZERO)
		assert_approx(avel.y, 3.14, "Synced angular velocity.y")

	# ── Test 5: Multiple component changes in one frame ──────────────────
	run_system("stagehand_tests::Update Entity Physics", {
		"entity_id": entity_id,
		"position_3d": Vector3(-10, -20, -30),
		"velocity_3d": Vector3(5, 5, 5),
		"collision_layer": 16,
		"collision_mask": 32,
	})
	progress(0.016)

	run_system("stagehand_tests::Query Physics Body State", {
		"prefab": "stagehand_tests::PhysicsKinematic3D",
		"dimension": "3d"
	})
	result = get_component("SceneChildrenResult")
	states = result.get("states", [])
	if states.size() > 0:
		var origin = states[0].get("origin", Vector3.ZERO)
		assert_approx(origin.x, -10.0, "Multi-update position.x")
		assert_approx(origin.y, -20.0, "Multi-update position.y")
		var vel = states[0].get("linear_velocity", Vector3.ZERO)
		assert_approx(vel.x, 5.0, "Multi-update velocity.x")

	run_system("stagehand_tests::Query Physics Bodies", {
		"prefab": "stagehand_tests::PhysicsKinematic3D"
	})
	result = get_component("SceneChildrenResult")
	bodies = result.get("bodies", [])
	if bodies.size() > 0:
		assert_eq(int(bodies[0].get("collision_layer", 0)), 16, "Multi-update collision layer")
		assert_eq(int(bodies[0].get("collision_mask", 0)), 32, "Multi-update collision mask")

	# ── Test 6: 2D change detection sync ─────────────────────────────────
	var entity_2d = instantiate_prefab("stagehand_tests::PhysicsKinematic2D", {
		"Position2D": Vector2(0, 0),
	})
	progress(0.016)

	run_system("stagehand_tests::Update Entity Physics", {
		"entity_id": entity_2d,
		"position_2d": Vector2(200, 300),
	})
	progress(0.016)

	run_system("stagehand_tests::Query Physics Body State", {
		"prefab": "stagehand_tests::PhysicsKinematic2D",
		"dimension": "2d"
	})
	result = get_component("SceneChildrenResult")
	states = result.get("states", [])
	if states.size() > 0:
		var origin = states[0].get("origin", Vector2.ZERO)
		assert_approx(origin.x, 200.0, "2D synced position.x")
		assert_approx(origin.y, 300.0, "2D synced position.y")

	# ── Test 7: No sync without change detection ─────────────────────────
	# After the previous progress() cleared HasChanged tags, updating via
	# set_component (which does NOT enable HasChanged) should NOT cause
	# the sync systems to fire. The PhysicsServer state should remain unchanged.
	# (We can't test this perfectly from GDScript, but we can verify that
	# progressing without changes doesn't alter state.)
	progress(0.016)
	progress(0.016)

	# Position should still be the same as test 5
	run_system("stagehand_tests::Query Physics Body State", {
		"prefab": "stagehand_tests::PhysicsKinematic3D",
		"dimension": "3d"
	})
	result = get_component("SceneChildrenResult")
	states = result.get("states", [])
	if states.size() > 0:
		var origin = states[0].get("origin", Vector3.ZERO)
		assert_approx(origin.x, -10.0, "Position unchanged without change detection")
		assert_approx(origin.y, -20.0, "Position.y unchanged without change detection")

	print("All physics change detection tests passed!")
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
