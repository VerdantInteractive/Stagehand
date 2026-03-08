extends FlecsWorld

## Tests the GodotSignal system: emits a signal from a C++ on-demand system
## and verifies it is received in GDScript via the "stagehand_signal_emitted" signal.

var signals_received: Array[Dictionary] = []
var test_complete := false

func _ready() -> void:
	print("Test: GodotSignal – C++ to GDScript signal emission")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# Connect to the built-in signal
	stagehand_signal_emitted.connect(_on_flecs_signal_emitted)

	# ── Test 1: Emit a simple test signal ────────────────────────────────────
	run_system("stagehand_tests::Emit Test Signal", {
		"signal_name": "test_ping",
		"signal_data": {"message": "hello from C++", "value": 42}
	})
	# Progress to flush deferred operations and trigger the observer
	progress(0.016)

	# The signal is deferred, so we need to wait a frame
	await get_tree().process_frame

	# Progress again to ensure deferred signal emission completes
	progress(0.016)
	await get_tree().process_frame

func _on_flecs_signal_emitted(signal_name: StringName, data: Dictionary) -> void:
	signals_received.append({"name": signal_name, "data": data})

func _process(delta: float) -> void:
	if test_complete:
		get_tree().quit(0)
		return

	if signals_received.size() > 0:
		# Validate received signals
		var found_ping := false
		for sig in signals_received:
			if sig["name"] == &"test_ping":
				found_ping = true
				var data: Dictionary = sig["data"]
				if data.get("message") != "hello from C++":
					_fail("Signal data 'message' mismatch: %s" % str(data.get("message")))
					return
				if data.get("value") != 42:
					_fail("Signal data 'value' mismatch: %s" % str(data.get("value")))
					return
				print("  PASS: Received 'test_ping' signal with correct data")

		if not found_ping:
			_fail("Did not receive 'test_ping' signal")
			return

		# ── Test 2: Emit a signal without data ───────────────────────────────
		signals_received.clear()
		run_system("stagehand_tests::Emit Test Signal", {
			"signal_name": "empty_signal"
		})
		progress(0.016)
		await get_tree().process_frame
		progress(0.016)
		await get_tree().process_frame

		var found_empty := false
		for sig in signals_received:
			if sig["name"] == &"empty_signal":
				found_empty = true
				print("  PASS: Received 'empty_signal' with empty data")

		if not found_empty:
			# Give one more frame for deferred signals
			progress(0.016)
			await get_tree().process_frame
			for sig in signals_received:
				if sig["name"] == &"empty_signal":
					found_empty = true

		if not found_empty:
			_fail("Did not receive 'empty_signal' signal")
			return

		print("All GodotSignal tests passed!")
		test_complete = true

func _fail(msg: String) -> void:
	print("FAIL: %s" % msg)
	get_tree().quit(1)
