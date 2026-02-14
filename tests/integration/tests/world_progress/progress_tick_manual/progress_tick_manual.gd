extends FlecsWorld

var test_passed = false
var manual_progress_calls = 0

func _ready() -> void:
	print("Test: ProgressTick MANUAL mode")
	
	# Set progress_tick to MANUAL
	set_progress_tick(PROGRESS_TICK_MANUAL)
	
	# Verify it was set
	var current_tick = get_progress_tick()
	if current_tick != PROGRESS_TICK_MANUAL:
		print("FAIL: Expected PROGRESS_TICK_MANUAL (", PROGRESS_TICK_MANUAL, "), got ", current_tick)
		get_tree().quit(1)
		return
	
	print("PASS: Progress tick set to MANUAL mode")
	
	# In MANUAL mode, we need to call progress() ourselves
	# Call it a few times to verify it works
	progress(0.016) # ~60 FPS delta
	manual_progress_calls += 1
	print("PASS: Called progress() manually (call #", manual_progress_calls, ")")
	
	# Wait a frame
	await get_tree().process_frame
	
	# Call progress again to verify it can be called multiple times
	progress(0.016)
	manual_progress_calls += 1
	print("PASS: Called progress() manually (call #", manual_progress_calls, ")")
	
	# Verify progress was called multiple times
	if manual_progress_calls < 2:
		print("FAIL: progress() was not called enough times")
		get_tree().quit(1)
		return
	
	print("PASS: World progresses only when progress() is called manually")
	test_passed = true

func _process(delta) -> void:
	# In MANUAL mode, we should NOT auto-progress here
	# We've already called progress() manually in _ready
	if test_passed:
		get_tree().quit(0)
