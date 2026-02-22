extends FlecsWorld

## Tests MultiMesh 3D rendering with both Color and CustomData, plus
## multiple prefab types rendered by a single MultiMesh:
## - Combined Color + CustomData buffer layout in 3D
## - Multiple 3D prefab types in prefabs_rendered
## - Draw order with Z-axis sorted mixed entities
## - Large mixed batch

func _ready() -> void:
	print("Test: Entity Rendering MultiMesh 3D – Full (Color+CustomData+MultiPrefab)")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Verify full MultiMesh 3D configuration ──────────────────────
	var renderer = get_node_or_null("FullRenderer3D")
	assert_true(renderer != null, "FullRenderer3D child node exists")

	var multimesh = renderer.get_multimesh()
	assert_true(multimesh != null, "MultiMesh resource assigned")
	assert_true(multimesh.is_using_colors(), "MultiMesh use_colors is true")
	assert_true(multimesh.is_using_custom_data(), "MultiMesh use_custom_data is true")
	assert_eq(multimesh.get_transform_format(), MultiMesh.TRANSFORM_3D, "Transform format is 3D")

	# ── Test 2: Verify multiple 3D prefabs registered ────────────────────────
	var prefabs = renderer.get_prefabs_rendered()
	assert_eq(prefabs.size(), 2, "Two prefabs registered")
	assert_eq(prefabs[0], "stagehand_tests::FullEntity3D", "First prefab: FullEntity3D")
	assert_eq(prefabs[1], "stagehand_tests::ColoredEntity3D", "Second prefab: ColoredEntity3D")

	# ── Test 3: Zero entities — no crash ─────────────────────────────────────
	progress(0.016)
	print("  PASS: Progress with zero full 3D entities stable")

	# ── Test 4: Instantiate full 3D entities (Color + CustomData) ────────────
	for i in range(5):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::FullEntity3D",
			"components": {
				"EntityValue": float(i + 1),
				"Position3D": Vector3(i * 2.0, i * 1.0, i * -3.0),
				"Color": Color(float(i) / 5.0, 0.8, 0.2, 1.0),
				"CustomData": Vector4(float(i), float(i) * 0.5, 0, 1)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::FullEntity3D"})
	progress(0.016)
	var count = get_component("AccumulatorValue")
	assert_eq(count, 5, "5 FullEntity3D instances")

	# ── Test 5: Verify full entity data ──────────────────────────────────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::FullEntity3D",
		"dimension": "3d"
	})
	progress(0.016)
	var result = get_component("SceneChildrenResult")
	assert_eq(result["count"], 5, "Query returned 5 full 3D entities")
	assert_eq(result["has_colors"], true, "Full 3D entities have Color")
	assert_eq(result["has_custom_data"], true, "Full 3D entities have CustomData")

	# ── Test 6: Instantiate colored-only 3D entities (second prefab) ─────────
	for i in range(4):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::ColoredEntity3D",
			"components": {
				"EntityValue": float(i + 100),
				"Position3D": Vector3(i * 5.0 + 20.0, 0, i * -5.0),
				"Color": Color(0, 0, 1.0, 0.9)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::ColoredEntity3D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 4, "4 ColoredEntity3D instances")

	# ── Test 7: Both prefab types render together in 3D ──────────────────────
	for i in range(5):
		progress(0.016)
	print("  PASS: Multi-prefab 3D rendering stable across 5 progress cycles")

	# ── Test 8: Z-axis draw order with mixed entities ────────────────────────
	renderer.set_draw_order(MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Z)
	assert_eq(renderer.get_draw_order(), MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Z, "Draw order set to Z")
	progress(0.016)
	progress(0.016)
	print("  PASS: Z draw order with mixed 3D entities stable")

	# ── Test 9: Y-axis draw order with mixed entities ────────────────────────
	renderer.set_draw_order(MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Y)
	assert_eq(renderer.get_draw_order(), MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Y, "Draw order set to Y")
	progress(0.016)
	print("  PASS: Y draw order with mixed 3D entities stable")
	renderer.set_draw_order(MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_NONE)

	# ── Test 10: Large mixed batch of 3D entities ────────────────────────────
	for i in range(20):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::FullEntity3D",
			"components": {
				"EntityValue": float(i),
				"Position3D": Vector3(i * 1.0, 0, i * -1.0),
				"Color": Color(1, 1, 1, 1),
				"CustomData": Vector4(1, 0, 0, 0)
			}
		})
	for i in range(20):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::ColoredEntity3D",
			"components": {
				"EntityValue": float(i + 50),
				"Position3D": Vector3(i * 1.0 + 50, 0, i * -1.0),
				"Color": Color(0, 1, 0, 1)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::FullEntity3D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 25, "25 total FullEntity3D instances")

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::ColoredEntity3D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 24, "24 total ColoredEntity3D instances")

	# ── Test 11: Sustained rendering with large entity count ─────────────────
	for i in range(10):
		progress(0.016)
	print("  PASS: 10 progress cycles with 49 mixed 3D entities stable")

	print("All full MultiMesh 3D rendering tests passed!")
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
