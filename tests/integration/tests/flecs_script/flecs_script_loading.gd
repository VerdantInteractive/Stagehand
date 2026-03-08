extends FlecsWorld

## Tests that Flecs scripts (.flecs files) are loaded automatically and
## that the entities they define actually exist in the ECS world.
## Also validates hierarchical loading order (directory depth, then
## alphabetical) by checking ChildOf relationships.

func _ready() -> void:
	print("Test: Flecs script loading and entity creation")

	set_progress_tick(PROGRESS_TICK_MANUAL)
	progress(0.016)

	# ── Test 1: Verify entities created by test_entities.flecs ───────────────
	# test_entities.flecs creates TestEntity0 through TestEntity9 via a loop.
	var entity_names: Array = []
	for i in range(10):
		entity_names.append("TestEntity%d" % i)

	run_system("stagehand_tests::Lookup Entities", {"names": entity_names})
	progress(0.016)

	var result = get_component("SceneChildrenResult")
	assert_eq(typeof(result), TYPE_DICTIONARY, "Lookup result is Dictionary")

	var found: Array = result.get("found", [])
	var missing: Array = result.get("missing", [])

	assert_eq(missing.size(), 0,
		"All TestEntity0..9 found (missing: %s)" % str(missing))
	assert_eq(found.size(), 10, "Exactly 10 test entities found")

	# ── Test 2: Verify hierarchical entities exist ───────────────────────────
	# The exec_order_respects_hierarchy_and_alphabetical_order/ directory
	# creates: GrandParent, Parent (ChildOf GrandParent),
	#          Child (ChildOf Parent), GrandChild (ChildOf Child).
	# If loading order were wrong, ChildOf references would have failed
	# during world init. We verify the entities actually exist.
	var hierarchy_names = ["GrandParent", "GrandParent.Parent",
		"GrandParent.Parent.Child", "GrandParent.Parent.Child.GrandChild"]
	run_system("stagehand_tests::Lookup Entities", {"names": hierarchy_names})
	progress(0.016)

	result = get_component("SceneChildrenResult")
	found = result.get("found", [])
	missing = result.get("missing", [])

	assert_eq(missing.size(), 0,
		"All hierarchy entities found (missing: %s)" % str(missing))
	assert_eq(found.size(), 4, "All 4 hierarchy entities found")

	# ── Test 3: Non-existent entity is correctly reported missing ────────────
	run_system("stagehand_tests::Lookup Entities", {
		"names": ["DoesNotExist", "AlsoMissing"]
	})
	progress(0.016)

	result = get_component("SceneChildrenResult")
	found = result.get("found", [])
	missing = result.get("missing", [])
	assert_eq(found.size(), 0, "Non-existent entities not found")
	assert_eq(missing.size(), 2, "Both missing entities reported")

	# ── Test 4: WorldConfiguration accessible ────────────────────────────────
	var config = get_component("WorldConfiguration")
	assert_eq(typeof(config), TYPE_DICTIONARY,
		"WorldConfiguration is accessible as Dictionary")

	print("All Flecs script tests passed!")
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
