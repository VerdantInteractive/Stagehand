extends FlecsWorld

## Tests MultiMesh 3D rendering with Color-enabled entities:
## - 3D entities with Color component
## - use_colors MultiMesh configuration in 3D
## - Color data query verification
## - Batch coloring

func _ready() -> void:
	print("Test: Entity Rendering MultiMesh 3D – Colored Entities")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Verify MultiMesh configuration ───────────────────────────────
	var renderer = get_node_or_null("ColoredRenderer3D")
	assert_true(renderer != null, "ColoredRenderer3D child node exists")

	var multimesh = renderer.get_multimesh()
	assert_true(multimesh != null, "MultiMesh resource assigned")
	assert_true(multimesh.is_using_colors(), "MultiMesh use_colors is true")
	assert_eq(multimesh.get_transform_format(), MultiMesh.TRANSFORM_3D, "Transform format is 3D")

	# ── Test 2: Verify prefab setup ──────────────────────────────────────────
	var prefabs = renderer.get_prefabs_rendered()
	assert_eq(prefabs.size(), 1, "Single prefab registered")
	assert_eq(prefabs[0], "stagehand_tests::ColoredEntity3D", "Prefab is ColoredEntity3D")

	# ── Test 3: Zero entities — no crash ─────────────────────────────────────
	progress(0.016)
	print("  PASS: Progress with zero colored 3D entities stable")

	# ── Test 4: Instantiate coloured 3D entities ─────────────────────────────
	var colors_to_test: Array[Color] = [
		Color.RED,
		Color.GREEN,
		Color.BLUE,
		Color(1, 1, 0, 1), # yellow
		Color(0, 1, 1, 0.5) # cyan semi-transparent
	]
	for i in range(colors_to_test.size()):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::ColoredEntity3D",
			"components": {
				"EntityValue": float(i + 1),
				"Position3D": Vector3(i * 2.0, 0, i * -3.0),
				"Color": colors_to_test[i]
			}
		})
	progress(0.016)
	progress(0.016)

	# ── Test 5: Verify count ─────────────────────────────────────────────────
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::ColoredEntity3D"})
	progress(0.016)
	var count = get_component("AccumulatorValue")
	assert_eq(count, 5, "5 ColoredEntity3D instances")

	# ── Test 6: Query color data ─────────────────────────────────────────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::ColoredEntity3D",
		"dimension": "3d"
	})
	progress(0.016)
	var result = get_component("SceneChildrenResult")
	assert_eq(result["count"], 5, "Query returned 5 colored 3D entities")
	assert_eq(result["has_colors"], true, "Colored 3D entities have Color component")

	var colors_array = result.get("colors", [])
	assert_eq(colors_array.size(), 5, "5 colors returned in query")

	# ── Test 7: Draw order with colored 3D entities ──────────────────────────
	renderer.set_draw_order(MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Z)
	assert_eq(renderer.get_draw_order(), MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_Z, "Draw order set to Z")
	progress(0.016)
	progress(0.016)
	print("  PASS: Z draw order with colored 3D entities stable")
	renderer.set_draw_order(MultiMeshRenderer3D.MULTIMESH_DRAW_ORDER_NONE)

	# ── Test 8: Batch colored 3D entities ────────────────────────────────────
	for i in range(30):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::ColoredEntity3D",
			"components": {
				"EntityValue": float(i + 100),
				"Position3D": Vector3(i * 1.0, i * 0.5, i * -1.0),
				"Color": Color(float(i) / 30.0, 1.0 - float(i) / 30.0, 0.5, 1)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::ColoredEntity3D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 35, "35 total ColoredEntity3D instances")

	# ── Test 9: Sustained rendering ──────────────────────────────────────────
	for i in range(10):
		progress(0.016)
	print("  PASS: 10 progress cycles with 35 colored 3D entities stable")

	print("All colored MultiMesh 3D rendering tests passed!")
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
