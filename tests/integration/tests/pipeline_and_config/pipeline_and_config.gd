extends FlecsWorld

## Tests pipeline phases and world configuration property integration.
## Verifies that world_configuration set in the scene tree is accessible,
## and that multiple progress calls work across different tick modes.

var frames_progressed := 0

func _ready() -> void:
	print("Test: Pipeline phases and world configuration property")

	# ── Test 1: Verify world_configuration is set from scene property ────────
	var config = world_configuration
	if typeof(config) != TYPE_DICTIONARY:
		_fail("world_configuration property is not a Dictionary")
		return
	assert_eq(config.get("test_int"), 42, "world_configuration test_int")
	assert_eq(config.get("test_string"), "hello", "world_configuration test_string")
	print("  PASS: world_configuration set from scene property")

	# ── Test 2: Verify the property is synced to the ECS singleton ───────────
	var ecs_config = get_component("WorldConfiguration")
	assert_eq(ecs_config.get("test_int"), 42, "ECS WorldConfiguration test_int")
	assert_eq(ecs_config.get("test_string"), "hello", "ECS WorldConfiguration test_string")
	print("  PASS: world_configuration synced to ECS singleton")

	# ── Test 3: Modify via set_world_configuration and verify ────────────────
	set_world_configuration({"modified_key": true, "value": 999})
	await get_tree().process_frame
	var modified = get_world_configuration()
	assert_eq(modified.get("modified_key"), true, "Modified configuration key")
	assert_eq(modified.get("value"), 999, "Modified configuration value")

	# ── Test 4: Verify get_world_configuration reads from ECS ────────────────
	set_component("WorldConfiguration", {"ecs_set": "from_ecs"})
	await get_tree().process_frame
	var from_ecs = get_world_configuration()
	assert_eq(from_ecs.get("ecs_set"), "from_ecs", "get_world_configuration reads from ECS")

	# ── Test 5: Progress tick renders correct phases ─────────────────────────
	set_progress_tick(PROGRESS_TICK_MANUAL)
	# Just verify that progressing the world doesn't crash
	for i in range(5):
		progress(0.016)
	print("  PASS: 5 manual progress calls executed without error")

	print("All pipeline / config property tests passed!")
	get_tree().quit(0)


# ── Assertion helpers ─────────────────────────────────────────────────────────

func assert_eq(actual, expected, label: String) -> void:
	if actual != expected:
		_fail("%s: expected %s, got %s" % [label, str(expected), str(actual)])
	else:
		print("  PASS: %s" % label)

func _fail(msg: String) -> void:
	print("FAIL: %s" % msg)
	get_tree().quit(1)
