extends FlecsWorld

## Tests entity lifecycle edge cases and cross-boundary interactions:
## - Mixed prefab types in same world
## - Tag operations across C++/GDScript boundary
## - Entity component access by entity ID
## - Prefab instantiation with all component types
## - Component value mutation across progress cycles
## - Multiple prefab types interacting via shared queries

func _ready() -> void:
	print("Test: Entity lifecycle – advanced cross-boundary tests")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Mixed prefab instantiation (2D and 3D in same world) ─────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {
			"EntityValue": 10.0,
			"Position2D": Vector2(100, 100)
		}
	})
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity3D",
		"components": {
			"EntityValue": 20.0,
			"Position3D": Vector3(50, 50, 50)
		}
	})
	progress(0.016)

	# Sum should include both 2D and 3D entities
	run_system("stagehand_tests::Sum Query", {})
	progress(0.016)
	var total = get_component("AccumulatorValue")
	assert_eq(total, 30, "Sum of mixed 2D+3D entities (10+20)")

	# ── Test 2: Tag toggle system (cross-boundary tag mutation) ───────────────
	# Instantiate entities with MarkerB tag and toggle to MarkerA via C++ system
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {
			"EntityValue": 5.0,
			"MarkerB": true
		}
	})
	progress(0.016)

	# Run the toggle tag system (removes MarkerB, adds MarkerA)
	run_system("stagehand_tests::Toggle Tag", {})
	progress(0.016)
	print("  PASS: Tag toggle system ran without crash")

	# ── Test 3: Instantiate multiple entities rapidly ─────────────────────────
	for i in range(20):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::TestEntity2D",
			"components": {
				"EntityValue": float(i)
			}
		})
	progress(0.016)

	# Sum query to verify all entities exist: 10 + 20 + 5 + sum(0..19) = 225
	run_system("stagehand_tests::Sum Query", {})
	progress(0.016)
	total = get_component("AccumulatorValue")
	assert_eq(total, 225, "Sum after rapid batch (10+20+5+sum(0..19)=225)")

	# ── Test 4: Prefab instantiation with various component types ────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {
			"EntityValue": 0.0,
			"Position2D": Vector2.ZERO,
			"Rotation2D": 0.0,
			"Scale2D": Vector2.ONE
		}
	})
	progress(0.016)
	print("  PASS: Entity with all transform components instantiated")

	# ── Test 5: Invalid component name in instantiation (should warn) ────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {
			"EntityValue": 1.0,
			"TotallyFakeComponent": 42
		}
	})
	progress(0.016)
	print("  PASS: Invalid component name handled gracefully")

	# ── Test 6: Prefab instantiation with empty components dict ──────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D",
		"components": {}
	})
	progress(0.016)
	print("  PASS: Empty components dictionary handled gracefully")

	# ── Test 7: Prefab instantiation without components key at all ────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity2D"
	})
	progress(0.016)
	print("  PASS: No components key handled gracefully")

	# ── Test 8: Multiple progress cycles don't corrupt state ─────────────────
	var prev_total = 0
	run_system("stagehand_tests::Sum Query", {})
	progress(0.016)
	prev_total = get_component("AccumulatorValue")

	for i in range(10):
		progress(0.016)

	run_system("stagehand_tests::Sum Query", {})
	progress(0.016)
	total = get_component("AccumulatorValue")
	assert_eq(total, prev_total, "Sum unchanged after 10 progress cycles (no new entities)")

	# ── Test 9: Instantiate 3D entities with extreme positions ────────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::TestEntity3D",
		"components": {
			"EntityValue": 0.0,
			"Position3D": Vector3(99999, -99999, 0.001)
		}
	})
	progress(0.016)
	print("  PASS: Entity with extreme position values instantiated")

	# ── Test 10: Verify singleton component access still works ────────────────
	set_component("TickCount", 0)
	progress(0.016)
	var ticks = get_component("TickCount")
	# TickCount system increments each progress, so it should be 1 now
	assert_eq(ticks, 1, "TickCount incremented during progress")

	print("All advanced entity lifecycle tests passed!")
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
