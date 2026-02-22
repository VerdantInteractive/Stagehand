extends FlecsWorld

## Merged ProgressTick test covering all three modes (MANUAL, RENDERING, PHYSICS)
## with actual auto-progress assertions using the TickCount system.

func _ready() -> void:
	print("Test: Progress tick modes – manual, rendering and physics auto-progress")

	# ── Test 1: MANUAL mode — mode value and explicit progress() ─────────────
	set_progress_tick(PROGRESS_TICK_MANUAL)
	assert_eq(get_progress_tick(), PROGRESS_TICK_MANUAL, "Mode set to MANUAL")

	set_component("TickCount", 0)
	progress(0.016)
	progress(0.016)
	assert_eq(get_component("TickCount"), 2, "TickCount after 2 manual progress() calls")

	# Verify auto-progress does NOT happen in MANUAL mode
	set_component("TickCount", 0)
	await get_tree().process_frame
	await get_tree().process_frame
	assert_eq(get_component("TickCount"), 0, "TickCount unchanged in MANUAL mode after idle frames")

	# ── Test 2: RENDERING mode — auto-progress via _process ──────────────────
	set_component("TickCount", 0)
	set_progress_tick(PROGRESS_TICK_RENDERING)
	assert_eq(get_progress_tick(), PROGRESS_TICK_RENDERING, "Mode set to RENDERING")

	# Wait several frames; the C++ _notification(NOTIFICATION_PROCESS) should
	# call progress() automatically, causing TickCount to increment.
	await get_tree().process_frame
	await get_tree().process_frame
	await get_tree().process_frame

	var rendering_ticks = get_component("TickCount")
	assert_true(rendering_ticks >= 2,
		"TickCount auto-incremented in RENDERING mode (%d >= 2)" % rendering_ticks)

	# ── Test 3: Mode switching ───────────────────────────────────────────────
	set_progress_tick(PROGRESS_TICK_MANUAL)
	assert_eq(get_progress_tick(), PROGRESS_TICK_MANUAL, "Switched RENDERING → MANUAL")

	set_progress_tick(PROGRESS_TICK_PHYSICS)
	assert_eq(get_progress_tick(), PROGRESS_TICK_PHYSICS, "Switched MANUAL → PHYSICS")

	set_progress_tick(PROGRESS_TICK_RENDERING)
	assert_eq(get_progress_tick(), PROGRESS_TICK_RENDERING, "Switched PHYSICS → RENDERING")

	# ── Test 4: PHYSICS mode — auto-progress via _physics_process ────────────
	set_progress_tick(PROGRESS_TICK_PHYSICS)
	assert_eq(get_progress_tick(), PROGRESS_TICK_PHYSICS, "Mode set to PHYSICS")
	set_component("TickCount", 0)

	# Wait several idle frames; the C++ _notification(NOTIFICATION_PHYSICS_PROCESS)
	# fires on the physics tick which runs in the same main loop before idle,
	# so process_frame awaits give physics ticks enough time to fire.
	await get_tree().process_frame
	await get_tree().process_frame
	await get_tree().process_frame
	await get_tree().process_frame

	var physics_ticks = get_component("TickCount")
	assert_true(physics_ticks >= 2,
		"TickCount auto-incremented in PHYSICS mode (%d >= 2)" % physics_ticks)

	# ── Final: switch back to MANUAL and finish ──────────────────────────────
	set_progress_tick(PROGRESS_TICK_MANUAL)
	assert_eq(get_progress_tick(), PROGRESS_TICK_MANUAL, "Switched back to MANUAL from PHYSICS")

	print("All progress tick mode tests passed!")
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
