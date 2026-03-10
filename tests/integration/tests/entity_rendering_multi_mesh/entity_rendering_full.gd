extends FlecsWorld

## Tests MultiMesh 2D rendering with both Color and CustomData, plus
## multiple prefab types rendered by a single MultiMesh:
## - Combined Color + CustomData buffer layout
## - Multiple prefab types in prefabs_rendered
## - Draw order with entities from multiple prefabs

func _ready() -> void:
	print("Test: Entity Rendering MultiMesh 2D – Full (Color+CustomData+MultiPrefab)")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Verify full MultiMesh configuration ──────────────────────────
	var renderer = get_node_or_null("FullRenderer")
	assert_true(renderer != null, "FullRenderer child node exists")

	var multimesh = renderer.get_multimesh()
	assert_true(multimesh != null, "MultiMesh resource assigned")
	assert_true(multimesh.is_using_colors(), "MultiMesh use_colors is true")
	assert_true(multimesh.is_using_custom_data(), "MultiMesh use_custom_data is true")

	# ── Test 2: Verify multiple prefabs rendered ─────────────────────────────
	var prefabs = renderer.get_prefabs_rendered()
	assert_eq(prefabs.size(), 2, "Two prefabs registered")
	assert_eq(prefabs[0], "stagehand_tests::FullEntity2D", "First prefab: FullEntity2D")
	assert_eq(prefabs[1], "stagehand_tests::ColoredEntity2D", "Second prefab: ColoredEntity2D")

	# ── Test 3: Zero entities — no crash ─────────────────────────────────────
	progress(0.016)
	print("  PASS: Progress with zero full entities stable")

	# ── Test 4: Instantiate full entities (Color + CustomData) ───────────────
	for i in range(5):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::FullEntity2D",
			"components": {
				"EntityValue": float(i + 1),
				"Position2D": Vector2(i * 100.0, i * 50.0),
				"Color": Color(float(i) / 5.0, 0.5, 0.5, 1.0),
				"CustomData": Vector4(float(i), float(i) * 2, float(i) * 3, 1.0)
			}
		})
	progress(0.016)
	progress(0.016)

	# Verify full entities exist
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::FullEntity2D"})
	progress(0.016)
	var count = get_component("AccumulatorValue")
	assert_eq(count, 5, "5 FullEntity2D instances")

	# ── Test 5: Verify full entity data via query ────────────────────────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::FullEntity2D",
		"dimension": "2d"
	})
	progress(0.016)
	var result = get_component("SceneChildrenResult")
	assert_eq(result["count"], 5, "Query returned 5 full entities")
	assert_eq(result["has_colors"], true, "Full entities have Color")
	assert_eq(result["has_custom_data"], true, "Full entities have CustomData")

	# ── Test 6: Instantiate colored entities from second prefab ──────────────
	for i in range(3):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::ColoredEntity2D",
			"components": {
				"EntityValue": float(i + 100),
				"Position2D": Vector2(i * 200.0 + 500, 0),
				"Color": Color(0, 0, 1.0, 0.8)
			}
		})
	progress(0.016)
	progress(0.016)

	# Verify colored entities
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::ColoredEntity2D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 3, "3 ColoredEntity2D instances")

	# ── Test 7: Both prefabs render in same MultiMesh ────────────────────────
	# Progress several times to let the rendering system process both sets
	for i in range(5):
		progress(0.016)
	print("  PASS: Multi-prefab rendering stable across 5 progress cycles")

	# ── Test 8: Draw order with Y-axis sorting ───────────────────────────────
	renderer.set_draw_order(MultiMeshRenderer2D.MULTIMESH_DRAW_ORDER_Y)
	assert_eq(renderer.get_draw_order(), MultiMeshRenderer2D.MULTIMESH_DRAW_ORDER_Y, "Draw order set to Y")
	progress(0.016)
	progress(0.016)
	print("  PASS: Y-axis draw order with multi-prefab entities stable")

	# Reset draw order
	renderer.set_draw_order(MultiMeshRenderer2D.MULTIMESH_DRAW_ORDER_NONE)

	# ── Test 9: Large batch with mixed prefabs ───────────────────────────────
	for i in range(25):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::FullEntity2D",
			"components": {
				"EntityValue": float(i),
				"Position2D": Vector2(i * 10.0, i * 10.0),
				"Color": Color(1, 1, 1, 1),
				"CustomData": Vector4(1, 0, 0, 0)
			}
		})
	for i in range(25):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::ColoredEntity2D",
			"components": {
				"EntityValue": float(i + 50),
				"Position2D": Vector2(i * 10.0 + 500, i * 10.0),
				"Color": Color(0, 1, 0, 1)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::FullEntity2D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 30, "30 total FullEntity2D instances")

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::ColoredEntity2D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 28, "28 total ColoredEntity2D instances")

	# ── Test 10: Sustained rendering with large entity count ─────────────────
	for i in range(10):
		progress(0.016)
	print("  PASS: 10 progress cycles with 58 mixed entities stable")

	print("All full MultiMesh 2D rendering tests passed!")
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
