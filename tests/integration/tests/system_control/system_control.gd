extends FlecsWorld

## Tests system enable/disable, on-demand system execution with parameters,
## and the Accumulator system.

var frame_count := 0

func _ready() -> void:
	print("Test: System control – enable, disable and run_system")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: Tick Counter system runs on each progress() ──────────────────
	progress(0.016)
	progress(0.016)
	progress(0.016)
	var ticks = get_component("TickCount")
	assert_eq(ticks, 3, "TickCount after 3 progress() calls")

	# ── Test 2: Disable Tick Counter system ──────────────────────────────────
	var ok = enable_system("stagehand_tests::Tick Counter", false)
	assert_true(ok, "enable_system(false) returned true")

	progress(0.016)
	progress(0.016)
	ticks = get_component("TickCount")
	assert_eq(ticks, 3, "TickCount unchanged after disable + 2 progress()")

	# ── Test 3: Re-enable Tick Counter system ────────────────────────────────
	ok = enable_system("stagehand_tests::Tick Counter", true)
	assert_true(ok, "enable_system(true) returned true")

	progress(0.016)
	ticks = get_component("TickCount")
	assert_eq(ticks, 4, "TickCount incremented after re-enable")

	# ── Test 4: Disable/enable a non-existent system (should return false) ───
	ok = enable_system("NonExistentSystem", false)
	assert_true(not ok, "enable_system on non-existent system returns false")

	# ── Test 5: Accumulator on-demand system with parameters ─────────────────
	# Reset accumulator
	set_component("AccumulatorValue", 0)
	progress(0.016)

	run_system("stagehand_tests::Accumulator", {"amount": 5})
	progress(0.016)
	var acc = get_component("AccumulatorValue")
	assert_eq(acc, 5, "Accumulator after adding 5")

	run_system("stagehand_tests::Accumulator", {"amount": 15})
	progress(0.016)
	acc = get_component("AccumulatorValue")
	assert_eq(acc, 20, "Accumulator after adding 15 more")

	# ── Test 6: Accumulator without 'amount' parameter (should warn) ─────────
	run_system("stagehand_tests::Accumulator", {})
	progress(0.016)
	acc = get_component("AccumulatorValue")
	assert_eq(acc, 20, "Accumulator unchanged after missing 'amount'")

	# ── Test 7: run_system on non-existent system returns false ──────────────
	ok = run_system("NonExistentSystem", {})
	assert_true(not ok, "run_system on non-existent system returns false")

	# ── Test 8: run_system with empty Dictionary for parameters ──────────────
	# The Tick Counter is not on-demand, but calling run_system on a valid
	# system entity should work (even if it's a pipeline system).
	ok = run_system("stagehand_tests::Tick Counter", {})
	assert_true(ok, "run_system on pipeline system works")

	print("All system control tests passed!")
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
