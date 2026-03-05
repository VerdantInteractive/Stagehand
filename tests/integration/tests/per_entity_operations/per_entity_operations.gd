extends FlecsWorld

## Tests per-entity component operations using the entity_id parameter.
## Covers: set_component, get_component, has_component with specific entity IDs.
## Note: TAG components (zero-size) are not exposed through the GDScript bridge;
## they don't have getters/setters, so add_component/remove_component for tags
## is not supported via the script API.

func _ready() -> void:
	print("Test: Per-entity component operations via entity_id parameter")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Create entity and set component by entity_id ─────────────
	print("\n=== Test 1: Set component on specific entity ===")
	var entity_id = instantiate_prefab("stagehand_tests::TestEntity2D", {
		"EntityValue": 5.0,
	})
	assert_true(entity_id != 0, "Prefab instantiation returned valid ID")
	progress(0.016)

	# Set EntityValue on the specific entity (not singleton)
	set_component("EntityValue", 99.0, entity_id)
	progress(0.016)

	# ── Test 2: Get component from specific entity ───────────────────────
	print("\n=== Test 2: Get component from specific entity ===")
	var value = get_component("EntityValue", entity_id)
	assert_eq(value, 99.0, "EntityValue read back from entity matches what was set")

	# ── Test 3: Set and get Position2D on specific entity ────────────────
	# Position2D is set AFTER progress so the decompose system (which reads
	# Transform2D and writes Position2D) does not overwrite it before we read.
	print("\n=== Test 3: Set and get Position2D ===")
	set_component("Position2D", Vector2(10, 20), entity_id)
	var pos: Vector2 = get_component("Position2D", entity_id)
	assert_true(pos.is_equal_approx(Vector2(10, 20)), "Position2D read back matches set value")

	# ── Test 4: Update Position2D on specific entity ─────────────────────
	print("\n=== Test 4: Update position on specific entity ===")
	set_component("Position2D", Vector2(300, 400), entity_id)
	pos = get_component("Position2D", entity_id)
	assert_true(pos.is_equal_approx(Vector2(300, 400)), "Position2D updated correctly")

	# ── Test 5: has_component on specific entity ─────────────────────────
	print("\n=== Test 5: has_component on specific entity ===")
	assert_true(has_component("EntityValue", entity_id), "has_component returns true for existing component")
	assert_true(has_component("Position2D", entity_id), "has_component returns true for Position2D")

	# ── Test 6: Multiple entities - independence ─────────────────────────
	print("\n=== Test 6: Multiple entity independence ===")
	var entity_a = instantiate_prefab("stagehand_tests::TestEntity2D", {"EntityValue": 100.0})
	var entity_b = instantiate_prefab("stagehand_tests::TestEntity2D", {"EntityValue": 200.0})
	progress(0.016)

	set_component("EntityValue", 111.0, entity_a)
	progress(0.016)

	var val_a = get_component("EntityValue", entity_a)
	var val_b = get_component("EntityValue", entity_b)
	assert_eq(val_a, 111.0, "Entity A value updated correctly")
	assert_eq(val_b, 200.0, "Entity B value unchanged (independent)")

	# ── Test 7: Singleton path still works (entity_id=0) ─────────────────
	print("\n=== Test 7: Singleton operations still work ===")
	set_component("AccumulatorValue", 42)
	progress(0.016)
	var acc = get_component("AccumulatorValue")
	assert_eq(acc, 42, "Singleton set/get works with default entity_id")

	# ── Test 8: get_component on non-existent component returns empty ────
	print("\n=== Test 8: Non-existent component returns empty ===")
	var empty = get_component("AccumulatorValue", entity_id)
	# AccumulatorValue is not part of TestEntity2D prefab, getter returns null
	assert_true(empty == null, "Non-existent component returns null")

	print("\nAll per-entity operation tests passed!")
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
