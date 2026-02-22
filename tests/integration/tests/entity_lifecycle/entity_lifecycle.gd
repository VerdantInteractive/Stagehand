extends FlecsWorld

## Tests entity lifecycle: prefab instantiation via run_system, setting
## components on entities, reading them back, and querying entity values
## through on-demand systems.

func _ready() -> void:
	print("Test: Entity lifecycle – prefab instantiation and component access")

	# Ensure the world is ticking so deferred operations flush
	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Instantiate a 2D entity via the Prefab Instantiation system ──
	var result = run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {
			"EntityValue": 10.0,
			"Position2D": Vector2(100, 200)
		}
	})
	assert_true(result, "run_system for prefab instantiation returned true")
	progress(0.016)

	# ── Test 2: Instantiate a second 2D entity with different values ─────────
	result = run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {
			"EntityValue": 20.0,
			"Position2D": Vector2(300, 400)
		}
	})
	assert_true(result, "Second prefab instantiation returned true")
	progress(0.016)

	# ── Test 3: Instantiate a 3D entity ──────────────────────────────────────
	result = run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity3D",
		"components": {
			"EntityValue": 30.0,
			"Position3D": Vector3(5, 10, 15)
		}
	})
	assert_true(result, "3D prefab instantiation returned true")
	progress(0.016)

	# ── Test 4: Use the Sum Query system to verify entity values ─────────────
	# The Sum Query system sums all EntityValue components and stores in AccumulatorValue.
	run_system("stagehand_tests::Sum Query", {})
	progress(0.016)

	var total = get_component("AccumulatorValue")
	assert_eq(total, 60, "Sum of EntityValue across all entities (10+20+30)")

	# ── Test 5: Instantiate with missing prefab (should warn, not crash) ─────
	result = run_system("stagehand::Prefab Instantiation", {
		"prefab": "NonExistentPrefab"
	})
	# The system should handle the missing prefab gracefully
	progress(0.016)
	print("  PASS: Missing prefab handled gracefully")

	# ── Test 6: Prefab Instantiation without 'prefab' key (should warn) ──────
	result = run_system("stagehand::Prefab Instantiation", {
		"components": {"EntityValue": 1.0}
	})
	assert_true(result, "System runs even without 'prefab' key")
	progress(0.016)
	print("  PASS: Missing 'prefab' key handled gracefully")

	# ── Test 7: Instantiate another entity and re-sum ────────────────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {
			"EntityValue": 40.0
		}
	})
	progress(0.016)

	run_system("stagehand_tests::Sum Query", {})
	progress(0.016)

	total = get_component("AccumulatorValue")
	assert_eq(total, 100, "Sum after adding 4th entity (10+20+30+40)")

	print("All entity lifecycle tests passed!")
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

func _fail(msg: String) -> void:
	print("FAIL: %s" % msg)
	get_tree().quit(1)
