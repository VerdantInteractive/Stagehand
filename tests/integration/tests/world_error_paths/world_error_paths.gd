extends FlecsWorld

## Tests FlecsWorld warning / error paths and stability behaviours:
## - enable_system / run_system on non-existent names
## - enable_system / run_system on a valid entity that is not a system
## - run_system with unexpected parameter data
## - set_component / get_component with unknown component names
## - Rapid error calls don't crash
## - World remains functional after error spam

func _ready() -> void:
	print("Test: FlecsWorld error paths and stability")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: enable_entity with non-existent name ────────────────────────
	var id = lookup("CompletelyFakeSystem")
	var ok = enable_entity(id, false)
	assert_true(not ok, "enable_entity on non-existent entity returns false")

	id = lookup("CompletelyFakeSystem")
	ok = enable_entity(id, true)
	assert_true(not ok, "enable_entity(true) on non-existent entity returns false")

	# ── Test 2: run_system with non-existent name ────────────────────────────
	ok = run_system("CompletelyFakeSystem", {})
	assert_true(not ok, "run_system on non-existent system returns false")

	# ── Test 3: enable_entity with empty string ──────────────────────────────
	id = lookup("")
	ok = enable_entity(id, false)
	assert_true(not ok, "enable_entity with empty string returns false")

	# ── Test 4: run_system with empty string ─────────────────────────────────
	ok = run_system("", {})
	assert_true(not ok, "run_system with empty string returns false")

	# ── Test 5: enable_entity on a valid entity that is not a system ─────────
	var my_entity = create_entity("MyTestEntity")
	ok = enable_entity(my_entity, false)
	assert_true(ok, "enable_entity on valid entity returns true")

	# ── Test 6: run_system on a valid entity that is not a system ────────────
	ok = run_system(my_entity, {})
	assert_true(not ok, "run_system on non-system entity returns false")

	# ── Test 7: run_system with valid on-demand system but wrong param keys ──
	# The Accumulator system expects {"amount": int}; passing something else
	# should warn but not crash, and AccumulatorValue should be unchanged.
	set_component("AccumulatorValue", 0)
	progress(0.016)

	run_system("stagehand_tests::Accumulator", {"wrong_key": 99})
	progress(0.016)
	var acc = get_component("AccumulatorValue")
	assert_eq(acc, 0, "Accumulator unchanged with wrong param key")

	# ── Test 8: run_system with empty dict on an on-demand system ────────────
	run_system("stagehand_tests::Accumulator", {})
	progress(0.016)
	acc = get_component("AccumulatorValue")
	assert_eq(acc, 0, "Accumulator unchanged with empty dict")

	# ── Test 9: set_component with unknown component name (warns) ────────────
	set_component("TotallyNonExistent", 42)
	# Should not crash; the warning is printed by the C++ side.
	print("  PASS: set_component with unknown name did not crash")

	# ── Test 10: get_component with unknown component name returns null ──────
	var val = get_component("TotallyNonExistent")
	assert_eq(val, null, "get_component unknown name returns null")

	# ── Test 11: Rapid error calls don't crash ───────────────────────────────
	for i in range(20):
		var bad_id = lookup("NoSuchSystem" + str(i))
		enable_entity(bad_id, false)
		run_system("NoSuchSystem" + str(i), {})
		get_component("NoSuchComponent" + str(i))
		set_component("NoSuchComponent" + str(i), i)
	print("  PASS: 20 rapid error calls did not crash")

	# ── Test 12: World still functional after error spam ─────────────────────
	set_component("TickCount", 0)
	progress(0.016)
	var ticks = get_component("TickCount")
	assert_eq(ticks, 1, "TickCount works after error spam")

	print("All FlecsWorld error path tests passed!")
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
