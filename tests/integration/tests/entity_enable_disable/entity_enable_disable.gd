extends FlecsWorld

## Tests that enable_entity(false) excludes entities from ECS queries,
## and enable_entity(true) re-includes them.
## Uses the "Count Rendered Entities" on-demand system (which uses a proper
## query_builder with flecs::IsA) to verify query-level effects.

const PREFAB := "stagehand_tests::TestEntity2D"

func _ready() -> void:
	print("Test: Entity enable/disable observable effects")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Create entities and verify count ─────────────────────────
	print("\n=== Test 1: Baseline count of 3 entities ===")
	var id_a = instantiate_prefab(PREFAB, {"EntityValue": 10.0})
	var id_b = instantiate_prefab(PREFAB, {"EntityValue": 20.0})
	var id_c = instantiate_prefab(PREFAB, {"EntityValue": 30.0})
	progress(0.016)

	var count = _count_entities()
	assert_eq(count, 3, "Baseline count: 3 entities")

	# ── Test 2: Disable entity B and verify exclusion ────────────────────
	print("\n=== Test 2: Disable entity B → count excludes it ===")
	var ok = enable_entity(id_b, false)
	assert_true(ok, "enable_entity(id_b, false) returns true")
	progress(0.016)

	count = _count_entities()
	assert_eq(count, 2, "Count after disabling B: 2")

	# ── Test 3: Disabled entity is still alive ───────────────────────────
	print("\n=== Test 3: Disabled entity is still alive ===")
	assert_true(is_alive(id_b), "Disabled entity is_alive returns true")

	# ── Test 4: Disabled entity components still readable ────────────────
	print("\n=== Test 4: Disabled entity components are readable ===")
	var val_b = get_component("EntityValue", id_b)
	assert_eq(val_b, 20.0, "EntityValue on disabled entity returns 20.0")

	# ── Test 5: Re-enable entity B ───────────────────────────────────────
	print("\n=== Test 5: Re-enable entity B → count includes it ===")
	ok = enable_entity(id_b, true)
	assert_true(ok, "enable_entity(id_b, true) returns true")
	progress(0.016)

	count = _count_entities()
	assert_eq(count, 3, "Count after re-enabling B: 3")

	# ── Test 6: Disable multiple entities ────────────────────────────────
	print("\n=== Test 6: Disable A and C simultaneously ===")
	enable_entity(id_a, false)
	enable_entity(id_c, false)
	progress(0.016)

	count = _count_entities()
	assert_eq(count, 1, "Count with only B active: 1")

	# ── Test 7: Re-enable all ────────────────────────────────────────────
	print("\n=== Test 7: Re-enable all entities ===")
	enable_entity(id_a, true)
	enable_entity(id_c, true)
	progress(0.016)

	count = _count_entities()
	assert_eq(count, 3, "Count with all re-enabled: 3")

	# ── Test 8: Disable is idempotent ────────────────────────────────────
	print("\n=== Test 8: Disable is idempotent ===")
	enable_entity(id_a, false)
	enable_entity(id_a, false) # second disable should be harmless
	progress(0.016)

	count = _count_entities()
	assert_eq(count, 2, "Count after double-disable: 2")

	# ── Test 9: Enable is idempotent ─────────────────────────────────────
	print("\n=== Test 9: Enable on already-enabled entity ===")
	enable_entity(id_b, true)
	enable_entity(id_b, true) # should be harmless
	progress(0.016)

	count = _count_entities()
	assert_eq(count, 2, "Count unchanged by double-enable on B: 2")

	# Re-enable A for cleanup
	enable_entity(id_a, true)

	print("\nAll entity enable/disable tests passed!")
	get_tree().quit(0)


# ── Helpers ───────────────────────────────────────────────────────────────────

func _count_entities() -> int:
	run_system("stagehand_tests::Count Rendered Entities", {"prefab": PREFAB})
	progress(0.016)
	return get_component("AccumulatorValue")


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
