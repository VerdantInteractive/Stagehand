extends FlecsWorld

var test_passed = false

func _ready() -> void:
	print("Test: WorldConfiguration with empty state")
	
	# Test 1: Start with an empty configuration
	# Even when configuration is empty, the singleton should exist but be empty
	var empty_config = {}
	set_component("WorldConfiguration", empty_config)
	
	await get_tree().process_frame
	
	# Try to get the component - should return empty dictionary
	var registry = get_component("WorldConfiguration")
	if typeof(registry) != TYPE_DICTIONARY:
		print("FAIL: Registry should be a dictionary, got type: ", typeof(registry))
		get_tree().quit(1)
		return
	
	if not registry.is_empty():
		print("FAIL: Registry should be empty when no configuration is set")
		get_tree().quit(1)
		return
	print("PASS: Empty configuration creates empty singleton component")
	
	# Test 2: Set configuration with various values
	var config = {
		"value1": 42,
		"value2": "test",
		"value3": Vector2(10, 20)
	}
	set_component("WorldConfiguration", config)
	
	await get_tree().process_frame
	
	registry = get_component("WorldConfiguration")
	
	if not registry.has("value1"):
		print("FAIL: value1 not found in configuration data")
		get_tree().quit(1)
		return
	
	if registry["value1"] != 42:
		print("FAIL: value1 is incorrect")
		get_tree().quit(1)
		return
	
	if not registry.has("value2"):
		print("FAIL: value2 not found in configuration data")
		get_tree().quit(1)
		return
	print("PASS: Configuration values are set correctly")
	
	# Test 3: Modify values
	config["value1"] = 100
	set_component("WorldConfiguration", config)
	
	await get_tree().process_frame
	
	registry = get_component("WorldConfiguration")
	if registry["value1"] != 100:
		print("FAIL: value1 should be updated to 100")
		get_tree().quit(1)
		return
	print("PASS: Configuration values can be updated")
	
	# Test 4: Clear the entire configuration by setting empty dictionary
	var cleared_config = {}
	set_component("WorldConfiguration", cleared_config)
	
	await get_tree().process_frame
	
	registry = get_component("WorldConfiguration")
	if not registry.is_empty():
		print("FAIL: Registry should be empty after setting empty configuration")
		get_tree().quit(1)
		return
	print("PASS: Setting empty configuration keeps the singleton but empty")
	
	# Test 5: Verify we can set configuration again after clearing
	var new_config = {
		"restored_value": 999
	}
	set_component("WorldConfiguration", new_config)
	
	await get_tree().process_frame
	
	registry = get_component("WorldConfiguration")
	if not registry.has("restored_value"):
		print("FAIL: restored_value not found after restoring configuration")
		get_tree().quit(1)
		return
	
	if registry["restored_value"] != 999:
		print("FAIL: restored_value is incorrect")
		get_tree().quit(1)
		return
	print("PASS: Configuration can be set after clearing")
	
	print("All tests passed!")
	test_passed = true

func _process(delta) -> void:
	if test_passed:
		progress(delta)
		get_tree().quit(0)
