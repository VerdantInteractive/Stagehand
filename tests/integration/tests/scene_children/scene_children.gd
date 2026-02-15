extends FlecsWorld

## Tests that the SceneChildren singleton can be populated and read back from
## both GDScript and C++ systems. Verifies node references survive the ECS
## round-trip.
##
## NOTE: Due to GDExtension notification ordering, the C++ _notification handler
## (which calls populate_scene_children_singleton) runs AFTER GDScript's
## _ready(). Therefore the test populates SceneChildren from GDScript to
## exercise the same code path via set_component.

func _ready() -> void:
	print("Test: SceneChildren singleton population")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	var godot_children = get_children()

	# Populate SceneChildren singleton from GDScript (mirrors what
	# populate_scene_children_singleton does in C++, but works regardless
	# of the GDExtension notification ordering).
	var children_dict = {}
	for child in godot_children:
		children_dict[child.name] = child
	set_component("SceneChildren", children_dict)

	progress(0.016)

	# ── Test 1: C++ system reads the SceneChildren singleton ─────────────────
	var ok = run_system("stagehand_tests::Read Scene Children", {})
	assert_true(ok, "run_system Read Scene Children returned true")
	progress(0.016)

	var result = get_component("SceneChildrenResult")
	assert_eq(typeof(result), TYPE_DICTIONARY, "SceneChildrenResult is Dictionary")

	var child_count: int = result.get("count", -1)
	assert_eq(child_count, 2, "SceneChildren has 2 children")

	var child_names: Array = result.get("names", [])
	var has_child_a := false
	var has_child_b := false
	for child_name in child_names:
		if str(child_name) == "ChildA":
			has_child_a = true
		elif str(child_name) == "ChildB":
			has_child_b = true

	assert_true(has_child_a, "SceneChildren contains 'ChildA'")
	assert_true(has_child_b, "SceneChildren contains 'ChildB'")

	# ── Test 2: GDScript reads back the SceneChildren singleton ──────────────
	var sc_dict = get_component("SceneChildren")
	assert_eq(typeof(sc_dict), TYPE_DICTIONARY, "SceneChildren is Dictionary")
	assert_true(sc_dict.has("ChildA"), "SceneChildren dict has 'ChildA'")
	assert_true(sc_dict.has("ChildB"), "SceneChildren dict has 'ChildB'")

	# Verify the values are actual node references
	var child_a_node = sc_dict["ChildA"]
	assert_true(child_a_node is Node, "ChildA value is a Node")
	assert_eq(str(child_a_node.name), "ChildA", "ChildA node name matches")

	print("All SceneChildren tests passed!")
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
