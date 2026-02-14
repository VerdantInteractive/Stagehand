extends FlecsWorld

var test_passed = false

func _ready() -> void:
	print("Test: WorldConfiguration basic functionality")
	
	# Test 1: Check that world_configuration can be set with string keys
	var config = {
		"max_health": 100,
		"player_name": "Hero",
		"position": Vector2(10, 20),
		"score": 42,
		"active": true
	}
	set_component("WorldConfiguration", config)
	
	var retrieved_config = get_component("WorldConfiguration")
	if retrieved_config.is_empty():
		print("FAIL: Retrieved configuration is empty")
		get_tree().quit(1)
		return
	print("PASS: World configuration can be set and retrieved")
	
	# Test 2: Check that individual values are correct
	if not retrieved_config.has("max_health"):
		print("FAIL: max_health not found in configuration")
		get_tree().quit(1)
		return
	
	if retrieved_config["max_health"] != 100:
		print("FAIL: max_health is ", retrieved_config["max_health"], ", expected 100")
		get_tree().quit(1)
		return
	
	if retrieved_config["player_name"] != "Hero":
		print("FAIL: player_name is incorrect")
		get_tree().quit(1)
		return
	print("PASS: Configuration values are correct")
	
	# Test 3: Check that various Variant types work as values
	var variant_config = {
		"int_value": 123,
		"float_value": 3.14,
		"string_value": "test",
		"vector_value": Vector3(1, 2, 3),
		"bool_value": false,
		"color_value": Color(1.0, 0.5, 0.0),
		"array_value": [1, 2, 3],
		"dict_value": {"nested": "data"}
	}
	set_component("WorldConfiguration", variant_config)
	
	await get_tree().process_frame
	
	var registry = get_component("WorldConfiguration")
	if typeof(registry) != TYPE_DICTIONARY:
		print("FAIL: Registry is not a dictionary, got type: ", typeof(registry))
		get_tree().quit(1)
		return

	if registry["int_value"] != 123:
		print("FAIL: Int value not preserved correctly")
		get_tree().quit(1)
		return

	if registry["float_value"] != 3.14:
		print("FAIL: Float value not preserved correctly")
		get_tree().quit(1)
		return

	if registry["string_value"] != "test":
		print("FAIL: String value not preserved correctly")
		get_tree().quit(1)
		return

	if registry["vector_value"] != Vector3(1, 2, 3):
		print("FAIL: Vector value not preserved correctly")
		get_tree().quit(1)
		return
	
	if registry["bool_value"] != false:
		print("FAIL: Bool value not preserved correctly")
		get_tree().quit(1)
		return

	if registry["color_value"] != Color(1.0, 0.5, 0.0):
		print("FAIL: Color value not preserved correctly")
		get_tree().quit(1)
		return

	if registry["array_value"] != [1, 2, 3]:
		print("FAIL: Array value not preserved correctly")
		get_tree().quit(1)
		return

	if registry["dict_value"] != {"nested": "data"}:
		print("FAIL: Dict value not preserved correctly")
		get_tree().quit(1)
		return
	print("PASS: Various Variant types work as values")
	
	# Test 4: Nested dictionaries keep their own key flexibility
	var nested_config = {
		"nested": {
			1: "integer key",
			StringName("named"): "string name key",
			Vector2i(3, 4): "vector key"
		}
	}
	set_component("WorldConfiguration", nested_config)

	await get_tree().process_frame

	registry = get_component("WorldConfiguration")
	if not registry.has("nested"):
		print("FAIL: nested dictionary missing from registry")
		get_tree().quit(1)
		return

	var nested_registry = registry["nested"]
	if typeof(nested_registry) != TYPE_DICTIONARY:
		print("FAIL: nested value is not a dictionary")
		get_tree().quit(1)
		return

	if not nested_registry.has(1):
		print("FAIL: nested integer key missing")
		get_tree().quit(1)
		return

	if not nested_registry.has(StringName("named")):
		print("FAIL: nested StringName key missing")
		get_tree().quit(1)
		return

	if not nested_registry.has(Vector2i(3, 4)):
		print("FAIL: nested Vector2i key missing")
		get_tree().quit(1)
		return
	print("PASS: Nested dictionaries preserve non-String key types")

	# Test 5: Modify the configuration. This time, also pass the 0 entity_id explicitly.
	var update_config = {
		"max_health": 75,
		"player_name": "Updated Hero"
	}
	set_component("WorldConfiguration", update_config, 0)
	
	await get_tree().process_frame
	
	# Check that the change was synced. This time, also pass the 0 entity_id explicitly.
	registry = get_component("WorldConfiguration", 0)
	if registry["max_health"] != 75:
		print("FAIL: Updated max_health is ", registry["max_health"], ", expected 75")
		get_tree().quit(1)
		return
	print("PASS: Configuration updates are synced to world")
	
	print("All tests passed!")
	test_passed = true

func _process(delta) -> void:
	if test_passed:
		progress(delta)
		get_tree().quit(0)
		# Quit after first successful frame
		get_tree().quit(0)
