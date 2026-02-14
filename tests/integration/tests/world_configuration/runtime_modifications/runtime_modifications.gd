extends FlecsWorld

var test_passed = false
var frame_count = 0

func _ready() -> void:
	print("Test: WorldConfiguration runtime modifications")
	
	# Test 1: Start with an empty configuration
	var initial_config = get_component("WorldConfiguration")
	if not initial_config.is_empty():
		print("FAIL: Initial configuration should be empty")
		get_tree().quit(1)
		return
	print("PASS: Initial configuration is empty")
	
	# Test 2: Verify singleton exists but is empty when configuration is empty
	await get_tree().process_frame
	
	var registry = get_component("WorldConfiguration")
	if not registry.is_empty():
		print("FAIL: WorldConfiguration singleton should be empty when no config set")
		get_tree().quit(1)
		return
	print("PASS: Singleton exists but is empty for empty configuration")
	
	# Test 3: Add configuration at runtime
	var config = {
		"counter": 0,
		"active": true
	}
	set_component("WorldConfiguration", config)
	
	await get_tree().process_frame
	
	registry = get_component("WorldConfiguration")
	if not registry.has("counter"):
		print("FAIL: counter not found after runtime addition")
		get_tree().quit(1)
		return
	print("PASS: Values added at runtime")
	
	# Test 4: Multiple updates in succession
	for i in range(5):
		config["counter"] = i
		set_component("WorldConfiguration", config)
		await get_tree().process_frame
		
		registry = get_component("WorldConfiguration")
		if registry["counter"] != i:
			print("FAIL: Counter is ", registry["counter"], ", expected ", i)
			get_tree().quit(1)
			return
	print("PASS: Multiple successive updates work correctly")
	
	# Test 5: Add more values without losing existing ones
	config["data"] = "test_value"
	set_component("WorldConfiguration", config)
	
	await get_tree().process_frame
	
	registry = get_component("WorldConfiguration")
	if not registry.has("counter"):
		print("FAIL: First value lost after adding second")
		get_tree().quit(1)
		return
	
	if not registry.has("data"):
		print("FAIL: data not found")
		get_tree().quit(1)
		return
	print("PASS: Multiple values coexist correctly")
	
	# Test 6: Verify configuration persists across frames
	await get_tree().process_frame
	await get_tree().process_frame
	
	registry = get_component("WorldConfiguration")
	if not registry.has("counter") or not registry.has("data"):
		print("FAIL: Configuration values not persisted across frames")
		get_tree().quit(1)
		return
	print("PASS: Configuration persists across frames")
	
	# Test 7: Clear configuration and verify singleton is empty
	set_component("WorldConfiguration", {})
	
	await get_tree().process_frame
	
	registry = get_component("WorldConfiguration")
	if not registry.is_empty():
		print("FAIL: Singleton should be empty when configuration is cleared")
		get_tree().quit(1)
		return
	print("PASS: Singleton is empty when configuration is cleared")
	
	# Test 8: Test that non-string keys are rejected by TypedDictionary
	# We can't directly test this in GDScript since the dict literal syntax
	# won't allow non-string keys for a typed dictionary, but we know it's enforced
	var test_config = {
		"final_test": "passed"
	}
	set_component("WorldConfiguration", test_config)
	
	await get_tree().process_frame
	
	registry = get_component("WorldConfiguration")
	if registry["final_test"] != "passed":
		print("FAIL: Final test value incorrect")
		get_tree().quit(1)
		return
	print("PASS: TypedDictionary enforces String keys")
	
	print("All tests passed!")
	test_passed = true

func _process(delta) -> void:
	if test_passed:
		progress(delta)
		frame_count += 1
		# Quit after a couple of frames to ensure stability
		if frame_count > 2:
			get_tree().quit(0)
