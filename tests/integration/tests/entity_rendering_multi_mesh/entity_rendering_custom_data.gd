extends FlecsWorld

## Tests MultiMesh 2D rendering with CustomData component:
## - Entity instantiation with CustomData (Vector4)
## - CustomData round-trip through ECS
## - Renderer with use_custom_data enabled

func _ready() -> void:
	print("Test: Entity Rendering MultiMesh 2D – CustomData")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Verify MultiMesh has use_custom_data enabled ─────────────────
	var renderer = get_node_or_null("CustomDataRenderer")
	assert_true(renderer != null, "CustomDataRenderer child node exists")

	var multimesh = renderer.get_multimesh()
	assert_true(multimesh != null, "MultiMesh resource assigned")
	assert_true(not multimesh.is_using_colors(), "MultiMesh use_colors is false")
	assert_true(multimesh.is_using_custom_data(), "MultiMesh use_custom_data is true")

	# ── Test 2: Verify prefabs_rendered ──────────────────────────────────────
	var prefabs = renderer.get_prefabs_rendered()
	assert_eq(prefabs[0], "stagehand_tests::CustomDataEntity2D", "CustomData prefab name")

	# ── Test 3: Zero entities — no crash ─────────────────────────────────────
	progress(0.016)
	print("  PASS: Progress with zero custom data entities stable")

	# ── Test 4: Instantiate entities with custom data ────────────────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::CustomDataEntity2D",
		"components": {
			"EntityValue": 1.0,
			"Position2D": Vector2(100, 200),
			"CustomData": Vector4(1.0, 2.0, 3.0, 4.0)
		}
	})
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::CustomDataEntity2D",
		"components": {
			"EntityValue": 2.0,
			"Position2D": Vector2(300, 400),
			"CustomData": Vector4(5.0, 6.0, 7.0, 8.0)
		}
	})
	progress(0.016)
	progress(0.016)

	# ── Test 5: Verify entity count ──────────────────────────────────────────
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::CustomDataEntity2D"})
	progress(0.016)
	var count = get_component("AccumulatorValue")
	assert_eq(count, 2, "2 CustomDataEntity2D instances")

	# ── Test 6: Verify custom data via query ─────────────────────────────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::CustomDataEntity2D",
		"dimension": "2d"
	})
	progress(0.016)
	var result = get_component("SceneChildrenResult")
	assert_eq(result["count"], 2, "Query returned 2 entities")
	assert_eq(result["has_custom_data"], true, "Entities have CustomData")

	var custom_data = result["custom_data"]
	assert_eq(custom_data.size(), 2, "2 custom data entries")

	# ── Test 7: Batch instantiation with varying custom data ─────────────────
	for i in range(30):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::CustomDataEntity2D",
			"components": {
				"EntityValue": float(i),
				"Position2D": Vector2(i * 10.0, 0),
				"CustomData": Vector4(float(i), float(i) * 0.5, 0.0, 1.0)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::CustomDataEntity2D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 32, "32 total CustomDataEntity2D")

	# ── Test 8: Multiple progress cycles ─────────────────────────────────────
	for i in range(5):
		progress(0.016)
	print("  PASS: 5 progress cycles with custom data entities stable")

	print("All custom data MultiMesh 2D rendering tests passed!")
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
