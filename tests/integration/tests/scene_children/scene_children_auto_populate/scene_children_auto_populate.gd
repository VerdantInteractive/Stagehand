extends FlecsWorld

## Tests the C++ populate_scene_children_singleton() code path.
##
## Unlike scene_children.gd (which manually populates SceneChildren from
## GDScript), this test relies on the C++ _notification(NOTIFICATION_READY)
## handler to populate the singleton automatically.
##
## GDExtension notification ordering means the C++ handler executes AFTER
## GDScript's _ready(). By awaiting one frame we ensure the C++ code has
## already run, and then we read SceneChildren without setting it manually.

func _ready() -> void:
	print("Test: SceneChildren auto-populated by C++ (populate_scene_children_singleton)")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# Wait one frame so that the C++ _notification(NOTIFICATION_READY)
	# has executed and populated the SceneChildren singleton.
	await get_tree().process_frame

	# ── Test 1: SceneChildren singleton should be auto-populated ─────────────
	var sc = get_component("SceneChildren")
	assert_eq(typeof(sc), TYPE_DICTIONARY, "SceneChildren is Dictionary")
	assert_true(not sc.is_empty(), "SceneChildren is not empty (auto-populated by C++)")

	# ── Test 2: Verify expected children are present ─────────────────────────
	assert_true(sc.has("AutoChildA"), "SceneChildren has 'AutoChildA'")
	assert_true(sc.has("AutoChildB"), "SceneChildren has 'AutoChildB'")
	assert_true(sc.has("AutoChildC"), "SceneChildren has 'AutoChildC'")
	assert_eq(sc.size(), 3, "Exactly 3 children")

	# ── Test 3: Values are actual node references ────────────────────────────
	var child_a = sc["AutoChildA"]
	assert_true(child_a is Node, "AutoChildA is a Node")
	assert_eq(str(child_a.name), "AutoChildA", "AutoChildA name matches")

	# ── Test 4: C++ Read Scene Children system can use the auto-populated data
	var ok = run_system("stagehand_tests::Read Scene Children", {})
	assert_true(ok, "run_system Read Scene Children returned true")
	progress(0.016)

	var result = get_component("SceneChildrenResult")
	assert_eq(typeof(result), TYPE_DICTIONARY, "SceneChildrenResult is Dictionary")
	assert_eq(result["count"], 3, "C++ system sees 3 children")

	var names: Array = result["names"]
	assert_true("AutoChildA" in names, "C++ sees AutoChildA")
	assert_true("AutoChildB" in names, "C++ sees AutoChildB")
	assert_true("AutoChildC" in names, "C++ sees AutoChildC")

	print("All SceneChildren auto-populate tests passed!")
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
