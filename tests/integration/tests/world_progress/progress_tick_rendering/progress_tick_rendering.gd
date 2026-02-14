extends FlecsWorld

func _ready() -> void:
	print("Test: ProgressTick RENDERING mode")
	
	# Set progress_tick to RENDERING
	set_progress_tick(PROGRESS_TICK_RENDERING)
	
	# Verify it was set
	var current_tick = get_progress_tick()
	if current_tick != PROGRESS_TICK_RENDERING:
		print("FAIL: Expected PROGRESS_TICK_RENDERING (", PROGRESS_TICK_RENDERING, "), got ", current_tick)
		get_tree().quit(1)
		return
	
	print("PASS: Progress tick set to RENDERING mode")
	
	# In RENDERING mode, the world will automatically progress in the _process callback
	# We verify this by confirming the mode was set correctly.
	print("PASS: World configured to progress in rendering tick (_process)")
	
	# Test that we can call progress manually even in RENDERING mode
	progress(0.016)
	print("PASS: progress() called successfully in RENDERING mode")
	
	# Test that we can switch to another mode
	set_progress_tick(PROGRESS_TICK_MANUAL)
	var tick_after_switch = get_progress_tick()
	if tick_after_switch != PROGRESS_TICK_MANUAL:
		print("FAIL: Could not switch to MANUAL mode")
		get_tree().quit(1)
		return
	
	print("PASS: Progress tick mode can be switched to MANUAL")
	get_tree().quit(0)
