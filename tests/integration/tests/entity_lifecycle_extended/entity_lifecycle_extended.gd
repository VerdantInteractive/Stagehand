extends FlecsWorld

## Tests entity creation and destruction lifecycle:
##   - create_entity with name and anonymous
##   - is_alive before and after destroy
##   - get_entity_name for named and anonymous entities
##   - enable_entity on non-system entities (M8)

func _ready() -> void:
	print("Test: Entity creation, destruction, and lifecycle")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Create named entity ──────────────────────────────────────
	print("\n=== Test 1: create_entity with name ===")
	var named_id = create_entity("MyNamedEntity")
	assert_true(named_id != 0, "Named entity ID is non-zero")
	assert_true(is_alive(named_id), "Named entity is alive after creation")

	var name = get_entity_name(named_id)
	assert_eq(name, "MyNamedEntity", "Entity name matches what was passed to create_entity")

	# ── Test 2: Create anonymous entity ──────────────────────────────────
	print("\n=== Test 2: create_entity anonymous ===")
	var anon_id = create_entity("")
	assert_true(anon_id != 0, "Anonymous entity ID is non-zero")
	assert_true(is_alive(anon_id), "Anonymous entity is alive after creation")

	var anon_name = get_entity_name(anon_id)
	assert_eq(anon_name, "", "Anonymous entity has empty name")

	# ── Test 3: Destroy entity and verify is_alive returns false ─────────
	print("\n=== Test 3: destroy_entity + is_alive ===")
	var doomed_id = create_entity("DoomedEntity")
	assert_true(is_alive(doomed_id), "Entity alive before destruction")

	destroy_entity(doomed_id)
	progress(0.016)

	assert_true(not is_alive(doomed_id), "Entity NOT alive after destruction")

	# ── Test 4: lookup returns 0 for destroyed entity ────────────────────
	print("\n=== Test 4: lookup returns 0 after destroy ===")
	var look_id = lookup("DoomedEntity")
	assert_eq(look_id, 0, "lookup returns 0 for destroyed entity name")

	# ── Test 5: Named entity is findable via lookup ──────────────────────
	print("\n=== Test 5: lookup finds named entity ===")
	var found_id = lookup("MyNamedEntity")
	assert_eq(found_id, named_id, "lookup returns correct entity ID")

	# ── Test 6: Destroy prefab entity and verify is_alive ────────────────
	print("\n=== Test 6: Destroy prefab entity ===")
	var prefab_id = instantiate_prefab("stagehand_tests::TestEntity2D", {
		"EntityValue": 50.0,
		"Position2D": Vector2(1, 2)
	})
	progress(0.016)
	assert_true(is_alive(prefab_id), "Prefab entity alive after instantiation")

	destroy_entity(prefab_id)
	progress(0.016)
	assert_true(not is_alive(prefab_id), "Prefab entity NOT alive after destruction")

	# ── Test 7: is_alive with invalid ID ─────────────────────────────────
	print("\n=== Test 7: is_alive with invalid ID ===")
	assert_true(not is_alive(999999999), "is_alive(bogus ID) returns false")

	# ── Test 8: Multiple creates and destroys ────────────────────────────
	print("\n=== Test 8: Multiple create/destroy cycles ===")
	var ids = []
	for i in range(5):
		ids.append(create_entity("Batch_%d" % i))
	progress(0.016)

	for eid in ids:
		assert_true(is_alive(eid), "Batch entity alive after creation")

	# Destroy odd-indexed entities
	destroy_entity(ids[1])
	destroy_entity(ids[3])
	progress(0.016)

	assert_true(is_alive(ids[0]), "ids[0] still alive")
	assert_true(not is_alive(ids[1]), "ids[1] destroyed")
	assert_true(is_alive(ids[2]), "ids[2] still alive")
	assert_true(not is_alive(ids[3]), "ids[3] destroyed")
	assert_true(is_alive(ids[4]), "ids[4] still alive")

	# ── Test 9: enable_entity on non-system entity (M8) ─────────────────
	print("\n=== Test 9: enable_entity on regular entity ===")
	var ent_id = instantiate_prefab("stagehand_tests::TestEntity2D", {
		"EntityValue": 10.0
	})
	progress(0.016)

	# Disable the entity — should exclude it from queries
	var ok = enable_entity(ent_id, false)
	assert_true(ok, "enable_entity(false) on regular entity returns true")
	progress(0.016)

	# The entity should still be alive
	assert_true(is_alive(ent_id), "Disabled entity is still alive")

	# Re-enable
	ok = enable_entity(ent_id, true)
	assert_true(ok, "enable_entity(true) on regular entity returns true")
	progress(0.016)

	print("\nAll entity lifecycle extended tests passed!")
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
