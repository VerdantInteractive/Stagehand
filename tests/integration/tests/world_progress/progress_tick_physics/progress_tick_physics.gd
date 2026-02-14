extends FlecsWorld

var test_passed = false

func _ready() -> void:
	print("Test: ProgressTick PHYSICS mode")
	
	# Set progress_tick to PHYSICS
	set_progress_tick(PROGRESS_TICK_PHYSICS)
	
	# Verify it was set
	var current_tick = get_progress_tick()
	if current_tick != PROGRESS_TICK_PHYSICS:
		print("FAIL: Expected PROGRESS_TICK_PHYSICS (", PROGRESS_TICK_PHYSICS, "), got ", current_tick)
		get_tree().quit(1)
		return
	
	print("PASS: Progress tick set to PHYSICS mode")
	
	# In PHYSICS mode, the world will automatically progress in the physics callback
	# We verify this by confirming the mode was set correctly.
	# The actual physics progression will happen when _physics_process is called by the engine.
	print("PASS: World configured to progress in physics tick (_physics_process)")
	
	# Test that we can switch back to verify toggle functionality
	set_progress_tick(PROGRESS_TICK_RENDERING)
	var tick_after_switch = get_progress_tick()
	if tick_after_switch != PROGRESS_TICK_RENDERING:
		print("FAIL: Could not switch back to RENDERING mode")
		get_tree().quit(1)
		return
	
	print("PASS: Progress tick mode can be switched back to RENDERING")
	test_passed = true

func _process(delta) -> void:
	if test_passed:
		get_tree().quit(0)
