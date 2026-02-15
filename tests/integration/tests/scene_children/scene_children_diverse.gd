extends FlecsWorld

## Tests scene children behavior with various node types and configurations:
## - Multiple child types (Node2D, Node3D, Control, etc.)
## - Children co-existing with MultiMeshRenderer nodes
## - Scene children populated correctly with diverse node tree

func _ready() -> void:
	print("Test: Scene children – diverse node types")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# Populate SceneChildren singleton from GDScript (mirrors what
	# populate_scene_children_singleton does in C++, but works regardless
	# of the GDExtension notification ordering).
	var godot_children = get_children()
	var children_dict = {}
	for child in godot_children:
		children_dict[child.name] = child
	set_component("SceneChildren", children_dict)
	progress(0.016)

	# ── Test 1: Verify all child nodes are accessible ────────────────────────
	run_system("stagehand_tests::Read Scene Children", {})
	progress(0.016)

	var result = get_component("SceneChildrenResult")
	assert_true(result is Dictionary, "SceneChildrenResult is Dictionary")
	assert_eq(result["count"], 4, "4 child nodes detected")

	# ── Test 2: Verify child names ───────────────────────────────────────────
	var names = result["names"]
	assert_true(names is Array, "names is Array")
	assert_true("Sprite" in names, "Sprite child found")
	assert_true("Camera" in names, "Camera child found")
	assert_true("Label" in names, "Label child found")
	assert_true("Timer" in names, "Timer child found")

	# ── Test 3: Verify get_children returns expected types ───────────────────
	var sprite = get_node_or_null("Sprite")
	assert_true(sprite != null, "Sprite node exists")
	assert_true(sprite is Node2D, "Sprite is Node2D")

	var camera = get_node_or_null("Camera")
	assert_true(camera != null, "Camera node exists")
	assert_true(camera is Camera3D, "Camera is Camera3D")

	var label = get_node_or_null("Label")
	assert_true(label != null, "Label node exists")
	assert_true(label is Label, "Label is Label")

	var timer = get_node_or_null("Timer")
	assert_true(timer != null, "Timer node exists")
	assert_true(timer is Timer, "Timer is Timer")

	# ── Test 4: Progress with diverse children — no crashes ──────────────────
	for i in range(5):
		progress(0.016)
	print("  PASS: Progress with diverse child nodes stable")

	# ── Test 5: ECS world functions still work with child nodes present ──────
	set_component("TickCount", 0)
	progress(0.016)
	var ticks = get_component("TickCount")
	assert_eq(ticks, 1, "TickCount works with diverse child nodes")

	print("All diverse scene children tests passed!")
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
