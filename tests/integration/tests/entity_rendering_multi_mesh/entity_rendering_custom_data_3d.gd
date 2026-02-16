extends FlecsWorld

## Tests MultiMesh 3D rendering with CustomData-enabled entities:
## - 3D entities with CustomData (Vector4) component
## - use_custom_data MultiMesh configuration in 3D
## - Custom data query verification
## - Batch with custom data

func _ready() -> void:
	print("Test: Entity Rendering MultiMesh 3D – CustomData Entities")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Verify MultiMesh configuration ───────────────────────────────
	var renderer = get_node_or_null("CustomDataRenderer3D")
	assert_true(renderer != null, "CustomDataRenderer3D child node exists")

	var multimesh = renderer.get_multimesh()
	assert_true(multimesh != null, "MultiMesh resource assigned")
	assert_true(multimesh.is_using_custom_data(), "MultiMesh use_custom_data is true")
	assert_eq(multimesh.get_transform_format(), MultiMesh.TRANSFORM_3D, "Transform format is 3D")

	# ── Test 2: Verify prefab setup ──────────────────────────────────────────
	var prefabs = renderer.get_prefabs_rendered()
	assert_eq(prefabs.size(), 1, "Single prefab registered")
	assert_eq(prefabs[0], "stagehand_tests::CustomDataEntity3D", "Prefab is CustomDataEntity3D")

	# ── Test 3: Zero entities — no crash ─────────────────────────────────────
	progress(0.016)
	print("  PASS: Progress with zero custom-data 3D entities stable")

	# ── Test 4: Instantiate custom-data 3D entities ──────────────────────────
	var custom_data_values: Array[Vector4] = [
		Vector4(1, 0, 0, 0), # material ID 1
		Vector4(0, 1, 0, 0.5), # material ID 2
		Vector4(0, 0, 1, 1.0), # material ID 3
		Vector4(1, 1, 0, 0.25), # combined
		Vector4(0.5, 0.5, 0.5, 0.75) # neutral
	]
	for i in range(custom_data_values.size()):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::CustomDataEntity3D",
			"components": {
				"EntityValue": float(i + 1),
				"Position3D": Vector3(i * 3.0, i * 1.0, i * -2.0),
				"CustomData": custom_data_values[i]
			}
		})
	progress(0.016)
	progress(0.016)

	# ── Test 5: Verify count ─────────────────────────────────────────────────
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::CustomDataEntity3D"})
	progress(0.016)
	var count = get_component("AccumulatorValue")
	assert_eq(count, 5, "5 CustomDataEntity3D instances")

	# ── Test 6: Query custom data ────────────────────────────────────────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::CustomDataEntity3D",
		"dimension": "3d"
	})
	progress(0.016)
	var result = get_component("SceneChildrenResult")
	assert_eq(result["count"], 5, "Query returned 5 custom-data 3D entities")
	assert_eq(result["has_custom_data"], true, "Custom data 3D entities have CustomData component")

	var custom_data_array = result.get("custom_data", [])
	assert_eq(custom_data_array.size(), 5, "5 custom data values returned in query")

	# ── Test 7: 3D positions with Z depth and custom data ────────────────────
	run_system("stagehand::Prefab Instantiation", {
		"prefab": "stagehand_tests::CustomDataEntity3D",
		"components": {
			"EntityValue": 99.0,
			"Position3D": Vector3(0, 100.0, -500.0),
			"CustomData": Vector4(0, 0, 0, 0)
		}
	})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::CustomDataEntity3D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 6, "6 total CustomDataEntity3D instances after deep-Z entity")

	# ── Test 8: Batch custom data 3D entities ────────────────────────────────
	for i in range(30):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::CustomDataEntity3D",
			"components": {
				"EntityValue": float(i + 200),
				"Position3D": Vector3(i * 1.0, 0, i * -0.5),
				"CustomData": Vector4(float(i) / 30.0, 0, 0, 1)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::CustomDataEntity3D"})
	progress(0.016)
	count = get_component("AccumulatorValue")
	assert_eq(count, 36, "36 total CustomDataEntity3D instances")

	# ── Test 9: Sustained rendering ──────────────────────────────────────────
	for i in range(10):
		progress(0.016)
	print("  PASS: 10 progress cycles with 36 custom-data 3D entities stable")

	print("All custom-data MultiMesh 3D rendering tests passed!")
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
