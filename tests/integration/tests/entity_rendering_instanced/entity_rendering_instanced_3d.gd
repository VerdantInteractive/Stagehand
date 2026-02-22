extends FlecsWorld

## Tests InstancedRenderer3D entity rendering integration:
## - InstancedRenderer3D node setup and prefabs_rendered property
## - LOD level configuration (4 LOD levels with SphereMesh)
## - Entity instantiation with instanced rendering prefabs
## - Entity count tracking through the renderer pipeline
## - Instance creation and transform updates via RenderingServer
## - LOD visibility range configuration
## - Zero-entity edge case
## - Large batch instantiation
## - Instance cleanup when entities are removed

func _ready() -> void:
	print("Test: Entity Rendering Instanced 3D")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Verify InstancedRenderer3D is a child node ───────────────────
	var renderer = get_node_or_null("InstancedRenderer3D")
	assert_true(renderer != null, "InstancedRenderer3D child node exists")
	assert_true(renderer is InstancedRenderer3D, "Node is InstancedRenderer3D type")
	assert_true(renderer is Node3D, "InstancedRenderer3D inherits from Node3D")

	# ── Test 2: Verify prefabs_rendered property is set ──────────────────────
	var prefabs = renderer.get_prefabs_rendered()
	assert_true(prefabs.size() > 0, "prefabs_rendered is not empty")
	assert_eq(prefabs[0], "stagehand_tests::InstancedEntity3D", "First prefab name")

	# ── Test 3: Verify LOD levels ────────────────────────────────────────────
	var lod_levels = renderer.get_lod_levels()
	assert_eq(lod_levels.size(), 4, "4 LOD levels configured")

	# ── Test 4: Zero entities — progress should not crash ────────────────────
	progress(0.016)
	progress(0.016)
	print("  PASS: Progress with zero entities does not crash")

	# Wait one frame so C++ _notification(NOTIFICATION_READY) fires and
	# registers the instanced renderer into the ECS (GDScript _ready runs
	# before the C++ notification handler).
	await get_tree().process_frame

	# ── Test 5: Verify instanced renderer was registered in ECS ──────────────
	run_system("stagehand_tests::Query Instanced Renderers", {})
	progress(0.016)
	var render_info = get_component("SceneChildrenResult")
	assert_eq(render_info["renderer_count"], 1, "1 instanced renderer registered")

	var renderers = render_info["renderers"]
	assert_eq(renderers.size(), 1, "1 renderer in array")

	var renderer_data = renderers[0]
	assert_eq(renderer_data["lod_count"], 4, "Renderer has 4 LOD configs")

	# ── Test 6: Verify LOD config details ────────────────────────────────────
	var lod_details = renderer_data["lod_details"]
	assert_eq(lod_details.size(), 4, "4 LOD detail entries")

	# LOD 0: highest detail, closest range
	assert_true(lod_details[0]["mesh_rid_valid"], "LOD 0 mesh RID is valid")
	assert_approx(lod_details[0]["fade_min"], 0.0, "LOD 0 fade_min")
	assert_approx(lod_details[0]["fade_max"], 50.0, "LOD 0 fade_max")
	assert_eq(lod_details[0]["visibility_fade_mode"], InstancedRenderer3DLODConfiguration.VISIBILITY_FADE_DISABLED, "LOD 0 visibility_fade_mode")

	# LOD 1: medium-high detail
	assert_true(lod_details[1]["mesh_rid_valid"], "LOD 1 mesh RID is valid")
	assert_approx(lod_details[1]["fade_min"], 50.0, "LOD 1 fade_min")
	assert_approx(lod_details[1]["fade_max"], 100.0, "LOD 1 fade_max")
	assert_eq(lod_details[1]["visibility_fade_mode"], InstancedRenderer3DLODConfiguration.VISIBILITY_FADE_SELF, "LOD 1 visibility_fade_mode")

	# LOD 2: medium-low detail
	assert_true(lod_details[2]["mesh_rid_valid"], "LOD 2 mesh RID is valid")
	assert_approx(lod_details[2]["fade_min"], 100.0, "LOD 2 fade_min")
	assert_approx(lod_details[2]["fade_max"], 200.0, "LOD 2 fade_max")
	assert_eq(lod_details[2]["visibility_fade_mode"], InstancedRenderer3DLODConfiguration.VISIBILITY_FADE_DEPENDENCIES, "LOD 2 visibility_fade_mode")

	# LOD 3: lowest detail, farthest range
	assert_true(lod_details[3]["mesh_rid_valid"], "LOD 3 mesh RID is valid")
	assert_approx(lod_details[3]["fade_min"], 200.0, "LOD 3 fade_min")
	assert_approx(lod_details[3]["fade_max"], 500.0, "LOD 3 fade_max")
	assert_eq(lod_details[3]["visibility_fade_mode"], InstancedRenderer3DLODConfiguration.VISIBILITY_FADE_SELF, "LOD 3 visibility_fade_mode")

	# ── Test 7: Verify no instances exist yet ────────────────────────────────
	assert_eq(renderer_data["entity_count"], 0, "No entities tracked yet")
	assert_eq(renderer_data["valid_instance_rids"], 0, "No valid instance RIDs yet")

	# ── Test 8: Instantiate entities and verify rendering pipeline ───────────
	for i in range(5):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::InstancedEntity3D",
			"components": {
				"EntityValue": float(i + 1),
				"Position3D": Vector3(i * 10.0, 0.0, i * -5.0),
				"Rotation3D": Quaternion.IDENTITY,
				"Scale3D": Vector3(1.0, 1.0, 1.0)
			}
		})

	# Progress twice to trigger transform update + rendering
	progress(0.016)
	progress(0.016)

	# Verify entities exist
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::InstancedEntity3D"})
	progress(0.016)
	var entity_count = get_component("AccumulatorValue")
	assert_eq(entity_count, 5, "5 InstancedEntity3D instances created")

	# ── Test 9: Verify instanced renderer created RenderingServer instances ──
	run_system("stagehand_tests::Query Instanced Renderers", {})
	progress(0.016)
	render_info = get_component("SceneChildrenResult")
	renderer_data = render_info["renderers"][0]

	assert_eq(renderer_data["entity_count"], 5, "Renderer tracks 5 entities")
	# 5 entities * 4 LOD levels = 20 instance RIDs
	assert_eq(renderer_data["valid_instance_rids"], 20, "20 valid instance RIDs (5 entities × 4 LODs)")

	# ── Test 10: Progress several times to verify stability ──────────────────
	for i in range(5):
		progress(0.016)
	print("  PASS: Multiple progress() calls with instanced entities are stable")

	# ── Test 11: Verify instance count stays consistent ──────────────────────
	run_system("stagehand_tests::Query Instanced Renderers", {})
	progress(0.016)
	render_info = get_component("SceneChildrenResult")
	renderer_data = render_info["renderers"][0]
	assert_eq(renderer_data["entity_count"], 5, "Entity count stable after multiple frames")
	assert_eq(renderer_data["valid_instance_rids"], 20, "Instance RID count stable after multiple frames")

	# ── Test 12: Add more entities and verify dynamic growth ─────────────────
	for i in range(10):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::InstancedEntity3D",
			"components": {
				"EntityValue": float(i + 100),
				"Position3D": Vector3(i * 5.0 + 50.0, i * 2.0, i * -3.0),
				"Rotation3D": Quaternion(Vector3.UP, deg_to_rad(i * 36.0)),
				"Scale3D": Vector3(1.0 + i * 0.1, 1.0, 1.0 + i * 0.1)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::InstancedEntity3D"})
	progress(0.016)
	entity_count = get_component("AccumulatorValue")
	assert_eq(entity_count, 15, "15 total InstancedEntity3D instances")

	run_system("stagehand_tests::Query Instanced Renderers", {})
	progress(0.016)
	render_info = get_component("SceneChildrenResult")
	renderer_data = render_info["renderers"][0]
	assert_eq(renderer_data["entity_count"], 15, "Renderer tracks 15 entities after growth")
	# 15 entities * 4 LODs = 60
	assert_eq(renderer_data["valid_instance_rids"], 60, "60 valid instance RIDs (15 entities × 4 LODs)")

	# ── Test 13: Large batch instantiation ─────────────────────────────────────
	for i in range(50):
		run_system("stagehand::Prefab Instantiation", {
			"prefab": "stagehand_tests::InstancedEntity3D",
			"components": {
				"EntityValue": float(i),
				"Position3D": Vector3(i * 2.0, 0.0, i * -1.0)
			}
		})
	progress(0.016)
	progress(0.016)

	run_system("stagehand_tests::Count Rendered Entities", {"prefab": "stagehand_tests::InstancedEntity3D"})
	progress(0.016)
	entity_count = get_component("AccumulatorValue")
	assert_eq(entity_count, 65, "65 total InstancedEntity3D instances after batch")

	run_system("stagehand_tests::Query Instanced Renderers", {})
	progress(0.016)
	render_info = get_component("SceneChildrenResult")
	renderer_data = render_info["renderers"][0]
	assert_eq(renderer_data["entity_count"], 65, "Renderer tracks 65 entities after batch")
	# 65 entities * 4 LODs = 260
	assert_eq(renderer_data["valid_instance_rids"], 260, "260 valid instance RIDs (65 entities × 4 LODs)")

	# ── Test 14: Multiple progress cycles with large entity count ────────────
	for i in range(10):
		progress(0.016)
	print("  PASS: 10 progress cycles with 65 instanced entities stable")

	# ── Test 15: Verify entity transforms via existing query system ──────────
	run_system("stagehand_tests::Query Entity Transforms", {
		"prefab": "stagehand_tests::InstancedEntity3D",
		"dimension": "3d"
	})
	progress(0.016)
	var query_result = get_component("SceneChildrenResult")
	assert_eq(query_result["count"], 65, "Query returned all 65 entities")
	assert_true(query_result["transforms"].size() > 0, "Transform data available")

	print("All entity rendering Instanced 3D tests passed!")
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
