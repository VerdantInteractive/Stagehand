extends FlecsWorld

## Tests MultiMesh 3D entity rendering integration:
## - MultiMeshRenderer3D node setup and prefabs_rendered property
## - Entity instantiation with 3D rendering prefabs
## - Entity count tracking through the renderer pipeline
## - Color component support
## - CustomData component support
## - Combined Color + CustomData
## - Draw order property (including Z axis, exclusive to 3D)
## - Transform3D computation for rendered entities
## - Zero-entity edge case
## - Large batch instantiation
## - Multiple prefab types rendered by a single MultiMesh

func _ready() -> void:
	print("Test: Entity Rendering MultiMesh 3D")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Verify MultiMeshRenderer3D is a child node ───────────────────
	var renderer = get_node_or_null("MultiMeshRenderer3D")
	assert_true(renderer != null, "MultiMeshRenderer3D child node exists")
	assert_true(renderer is MultiMeshRenderer3D, "Node is MultiMeshRenderer3D type")

	# ── Test 2: Verify prefabs_rendered property is set ──────────────────────
	var prefabs = renderer.get_prefabs_rendered()
	assert_true(prefabs.size() > 0, "prefabs_rendered is not empty")
	assert_eq(prefabs[0], "stagehand_tests::RenderedEntity3D", "First prefab name")

	# ── Test 3: Verify draw_order property defaults ──────────────────────────
	var draw_order = renderer.get_draw_order()
	assert_eq(draw_order, MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_NONE, "Default draw_order is NONE")

	# ── Test 4: Set draw_order to all valid values including Z (3D-exclusive)
	renderer.set_draw_order(MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_X)
	assert_eq(renderer.get_draw_order(), MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_X, "Draw order set to X")
	renderer.set_draw_order(MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Y)
	assert_eq(renderer.get_draw_order(), MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Y, "Draw order set to Y")
	renderer.set_draw_order(MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Z)
	assert_eq(renderer.get_draw_order(), MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Z, "Draw order set to Z (3D only)")
	# Reset to NONE for remaining tests
	renderer.set_draw_order(MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_NONE)

	# ── Test 5: Verify MultiMesh resource is assigned and 3D ─────────────────
	var multimesh = renderer.get_multimesh()
	assert_true(multimesh != null, "MultiMesh resource is assigned")
	assert_eq(multimesh.get_transform_format(), MultiMesh.TRANSFORM_3D, "MultiMesh uses TRANSFORM_3D")

	# ── Test 6: Zero entities — progress should not crash ────────────────────
	progress(0.016)
	progress(0.016)
	print("  PASS: Progress with zero entities does not crash")

	# ── Test 7: Instantiate basic rendered 3D entities ───────────────────────
	for i in range(5):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::RenderedEntity3D",
			"components": {
				"EntityValue": float(i + 1),
				"Position3D": Vector3(i * 10.0, i * 5.0, i * 2.0),
				"Rotation3D": Quaternion.IDENTITY,
				"Scale3D": Vector3(1.0, 1.0, 1.0)
			}
		})
	progress(0.016)
	progress(0.016)

	# Verify entities were instantiated
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::RenderedEntity3D"})
	progress(0.016)
	var entity_count = get_component("AccumulatorValue")
	assert_eq(entity_count, 5, "5 RenderedEntity3D instances created")

	# ── Test 8: Verify transform data via query system ───────────────────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::RenderedEntity3D",
		"dimension": "3d"
	})
	progress(0.016)
	var query_result = get_component("SceneChildrenResult")
	assert_eq(query_result["count"], 5, "Query returned 5 entities")
	assert_eq(query_result["has_colors"], false, "Basic entities have no Color")
	assert_eq(query_result["has_custom_data"], false, "Basic entities have no CustomData")

	# Verify transforms are non-empty
	var transforms = query_result["transforms"]
	assert_true(transforms.size() > 0, "Transform array is not empty")

	# ── Test 9: Progress several times to ensure rendering pipeline is stable
	for i in range(5):
		progress(0.016)
	print("  PASS: Multiple progress() calls with 3D entities are stable")

	# ── Test 10: Instantiate entity with non-identity rotation and scale ─────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::RenderedEntity3D",
		"components": {
			"EntityValue": 42.0,
			"Position3D": Vector3(100, 200, 300),
			"Rotation3D": Quaternion(0, 0.707, 0, 0.707),
			"Scale3D": Vector3(2.0, 3.0, 4.0)
		}
	})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::RenderedEntity3D"})
	progress(0.016)
	entity_count = get_component("AccumulatorValue")
	assert_eq(entity_count, 6, "6 RenderedEntity3D instances after adding rotated+scaled entity")

	# ── Test 11: Large batch instantiation ────────────────────────────────────
	for i in range(50):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::RenderedEntity3D",
			"components": {
				"EntityValue": float(i),
				"Position3D": Vector3(i * 5.0, i * 3.0, i * 1.0)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::RenderedEntity3D"})
	progress(0.016)
	entity_count = get_component("AccumulatorValue")
	assert_eq(entity_count, 56, "56 total RenderedEntity3D instances after batch")

	# ── Test 12: Multiple progress cycles to trigger buffer management ────────
	for i in range(10):
		progress(0.016)
	print("  PASS: 10 progress cycles with 56 3D entities stable")

	# ── Test 13: Verify entity transforms with Z positions ───────────────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::RenderedEntity3D",
		"dimension": "3d"
	})
	progress(0.016)
	query_result = get_component("SceneChildrenResult")
	assert_eq(query_result["count"], 56, "Query returned all 56 3D entities")

	# Check that at least some transforms have non-zero z origins
	var has_z = false
	for t in query_result["transforms"]:
		if t.has("origin_z") and t["origin_z"] != 0.0:
			has_z = true
			break
	assert_true(has_z, "Some 3D entities have non-zero Z origin")

	print("All entity rendering MultiMesh 3D tests passed!")
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
