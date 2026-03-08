extends FlecsWorld

## Tests MultiMesh 2D rendering with Color component enabled:
## - Entity instantiation with Color data
## - Color data round-trip through ECS
## - Renderer with use_colors enabled
## - Multiple colored entities rendered correctly

func _ready() -> void:
	print("Test: Entity Rendering MultiMesh 2D – Color")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Verify MultiMesh has use_colors enabled ──────────────────────
	var renderer = get_node_or_null("ColoredRenderer")
	assert_true(renderer != null, "ColoredRenderer child node exists")

	var multimesh = renderer.get_multimesh()
	assert_true(multimesh != null, "MultiMesh resource assigned")
	assert_true(multimesh.is_using_colors(), "MultiMesh use_colors is true")
	assert_true(not multimesh.is_using_custom_data(), "MultiMesh use_custom_data is false")

	# ── Test 2: Verify prefabs_rendered is set ───────────────────────────────
	var prefabs = renderer.get_prefabs_rendered()
	assert_eq(prefabs.size(), 1, "One prefab registered")
	assert_eq(prefabs[0], "stagehand_tests::ColoredEntity2D", "Colored prefab name")

	# ── Test 3: Zero entities with colors — no crash ─────────────────────────
	progress(0.016)
	progress(0.016)
	print("  PASS: Progress with zero colored entities stable")

	# ── Test 4: Instantiate colored entities ─────────────────────────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::ColoredEntity2D",
		"components": {
			"EntityValue": 1.0,
			"Position2D": Vector2(10, 20),
			"Color": Color(1.0, 0.0, 0.0, 1.0)
		}
	})
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::ColoredEntity2D",
		"components": {
			"EntityValue": 2.0,
			"Position2D": Vector2(30, 40),
			"Color": Color(0.0, 1.0, 0.0, 0.5)
		}
	})
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::ColoredEntity2D",
		"components": {
			"EntityValue": 3.0,
			"Position2D": Vector2(50, 60),
			"Color": Color(0.0, 0.0, 1.0, 0.0)
		}
	})
	progress(0.016)
	progress(0.016)

	# ── Test 5: Verify entity count ──────────────────────────────────────────
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::ColoredEntity2D"})
	progress(0.016)
	var count = get_component("AccumulatorValue")
	assert_eq(count, 3, "3 ColoredEntity2D instances created")

	# ── Test 6: Verify colors via query system ───────────────────────────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::ColoredEntity2D",
		"dimension": "2d"
	})
	progress(0.016)
	var result = get_component("SceneChildrenResult")
	assert_eq(result["count"], 3, "Query returned 3 colored entities")
	assert_eq(result["has_colors"], true, "Entities have Color component")

	var colors = result["colors"]
	assert_eq(colors.size(), 3, "3 colors returned")

	# ── Test 7: Multiple progress cycles with colored entities ───────────────
	for i in range(10):
		progress(0.016)
	print("  PASS: 10 progress cycles with colored entities stable")

	# ── Test 8: Add more colored entities and verify ─────────────────────────
	for i in range(20):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::ColoredEntity2D",
			"components": {
				"EntityValue": float(i + 10),
				"Position2D": Vector2(i * 20.0, 0),
				"Color": Color(float(i) / 20.0, 0.5, 0.5, 1.0)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::ColoredEntity2D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 23, "23 total ColoredEntity2D after batch")

	print("All colored MultiMesh 2D rendering tests passed!")
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
