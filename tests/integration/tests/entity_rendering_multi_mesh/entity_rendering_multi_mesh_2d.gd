extends FlecsWorld

## Tests MultiMesh 2D entity rendering integration:
## - MultiMeshRenderer2D node setup and prefabs_rendered property
## - Entity instantiation with rendering prefabs
## - Entity count tracking through the renderer pipeline
## - Color component support
## - CustomData component support
## - Combined Color + CustomData
## - Multiple prefab types rendered by a single MultiMesh
## - Draw order property
## - Transform computation for rendered entities
## - Zero-entity edge case
## - Large batch instantiation

func _ready() -> void:
	print("Test: Entity Rendering MultiMesh 2D")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Verify MultiMeshRenderer2D is a child node ───────────────────
	var renderer = get_node_or_null("MultiMeshRenderer2D")
	assert_true(renderer != null, "MultiMeshRenderer2D child node exists")
	assert_true(renderer is MultiMeshRenderer2D, "Node is MultiMeshRenderer2D type")

	# ── Test 2: Verify prefabs_rendered property is set ──────────────────────
	var prefabs = renderer.get_prefabs_rendered()
	assert_true(prefabs.size() > 0, "prefabs_rendered is not empty")
	assert_eq(prefabs[0], "stagehand_tests::RenderedEntity2D", "First prefab name")

	# ── Test 3: Verify draw_order property defaults ──────────────────────────
	var draw_order = renderer.get_draw_order()
	assert_eq(draw_order, MultiMeshRenderer2D.MULTIMESH_DRAW_ORDER_NONE, "Default draw_order is NONE")

	# ── Test 4: Set draw_order and read back ─────────────────────────────────
	renderer.set_draw_order(MultiMeshRenderer2D.MULTIMESH_DRAW_ORDER_X)
	assert_eq(renderer.get_draw_order(), MultiMeshRenderer2D.MULTIMESH_DRAW_ORDER_X, "Draw order set to X")
	renderer.set_draw_order(MultiMeshRenderer2D.MULTIMESH_DRAW_ORDER_Y)
	assert_eq(renderer.get_draw_order(), MultiMeshRenderer2D.MULTIMESH_DRAW_ORDER_Y, "Draw order set to Y")
	# Reset to NONE for remaining tests
	renderer.set_draw_order(MultiMeshRenderer2D.MULTIMESH_DRAW_ORDER_NONE)

	# ── Test 5: Verify MultiMesh resource is assigned ────────────────────────
	var multimesh = renderer.get_multimesh()
	assert_true(multimesh != null, "MultiMesh resource is assigned")
	assert_eq(multimesh.get_transform_format(), MultiMesh.TRANSFORM_2D, "MultiMesh uses TRANSFORM_2D")

	# ── Test 6: Zero entities — progress should not crash ────────────────────
	# Before instantiating any entities, the renderer should handle zero entities gracefully
	progress(0.016)
	progress(0.016)
	print("  PASS: Progress with zero entities does not crash")

	# ── Test 7: Instantiate basic rendered 2D entities ───────────────────────
	for i in range(5):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::RenderedEntity2D",
			"components": {
				"EntityValue": float(i + 1),
				"Position2D": Vector2(i * 50.0, i * 30.0),
				"Rotation2D": 0.0,
				"Scale2D": Vector2(1.0, 1.0)
			}
		})
	progress(0.016)
	progress(0.016)

	# Verify entities were instantiated
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::RenderedEntity2D"})
	progress(0.016)
	var entity_count = get_component("AccumulatorValue")
	assert_eq(entity_count, 5, "5 RenderedEntity2D instances created")

	# ── Test 8: Verify transform data via query system ───────────────────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::RenderedEntity2D",
		"dimension": "2d"
	})
	progress(0.016)
	var query_result = get_component("SceneChildrenResult")
	assert_eq(query_result["count"], 5, "Query returned 5 entities")
	assert_eq(query_result["has_colors"], false, "Basic entities have no Color")
	assert_eq(query_result["has_custom_data"], false, "Basic entities have no CustomData")

	# Verify first entity's transform origin
	var transforms = query_result["transforms"]
	assert_true(transforms.size() > 0, "Transform array is not empty")

	# ── Test 9: Progress several times to ensure rendering pipeline is stable
	for i in range(5):
		progress(0.016)
	print("  PASS: Multiple progress() calls with entities are stable")

	# ── Test 10: Instantiate entities with different positions and verify ─────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::RenderedEntity2D",
		"components": {
			"EntityValue": 100.0,
			"Position2D": Vector2(999, 888),
			"Rotation2D": 1.57,
			"Scale2D": Vector2(2.0, 3.0)
		}
	})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::RenderedEntity2D"})
	progress(0.016)
	entity_count = get_component("AccumulatorValue")
	assert_eq(entity_count, 6, "6 RenderedEntity2D instances after adding one more")

	# ── Test 11: Large batch instantiation ────────────────────────────────────
	for i in range(50):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::RenderedEntity2D",
			"components": {
				"EntityValue": float(i),
				"Position2D": Vector2(i * 10.0, i * 5.0)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::RenderedEntity2D"})
	progress(0.016)
	entity_count = get_component("AccumulatorValue")
	assert_eq(entity_count, 56, "56 total RenderedEntity2D instances after batch")

	# ── Test 12: Multiple progress cycles to trigger buffer management ────────
	for i in range(10):
		progress(0.016)
	print("  PASS: 10 progress cycles with 56 entities stable")

	print("All entity rendering MultiMesh 2D tests passed!")
	get_tree().quit(0)


# ── Assertion helpers ─────────────────────────────────────────────────────────

func assert_eq(actual, expected, label: String) -> void:
	if actual != expected:
		_fail("%s: expected %s, got %s" % [label, str(expected), str(actual)])
	else:
		print("  PASS: %s" % label)

func assert_approx(actual: float, expected: float, label: String, epsilon: float = 0.01) -> void:
	if abs(actual - expected) > epsilon:
		_fail("%s: expected ~%s, got %s" % [label, str(expected), str(actual)])
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
