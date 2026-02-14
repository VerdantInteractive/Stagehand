extends FlecsWorld

var test_passed = false

func _ready() -> void:
	print("Test: WorldConfiguration edge cases and warning behaviors")
	
	# Test 1: Attempt to access WorldConfiguration with non-zero entity_id (should warn and return empty dict)
	print("Testing invalid entity_id access...")
	var result = get_component("WorldConfiguration", 123)
	if typeof(result) != TYPE_DICTIONARY:
		print("FAIL: Should return dictionary for invalid entity_id, got type: ", typeof(result))
		get_tree().quit(1)
		return
	
	if not result.is_empty():
		print("FAIL: Should return empty dictionary for invalid entity_id")
		get_tree().quit(1)
		return
	print("PASS: Invalid entity_id access returns empty dictionary")
	
	# Test 2: Attempt to set WorldConfiguration with non-zero entity_id (should warn and do nothing)
	print("Testing invalid entity_id set...")
	var test_config = {"invalid_entity_test": "value"}
	set_component("WorldConfiguration", test_config, 999)
	
	await get_tree().process_frame
	
	# Verify that entity 999 doesn't have the component (singleton should be unaffected)
	var singleton_result = get_component("WorldConfiguration")
	if singleton_result.has("invalid_entity_test"):
		print("FAIL: Setting with invalid entity_id should not affect singleton")
		get_tree().quit(1)
		return
	print("PASS: Invalid entity_id set does nothing (with warning)")
	
	# Test 3: Try to set WorldConfiguration with non-Dictionary type (should warn)
	print("Testing non-Dictionary type...")
	set_component("WorldConfiguration", "this_is_a_string")
	
	await get_tree().process_frame
	
	# The component should remain empty or unchanged
	result = get_component("WorldConfiguration")
	if result.has("this_is_a_string"):
		print("FAIL: Non-Dictionary type should not be set")
		get_tree().quit(1)
		return
	print("PASS: Non-Dictionary type rejected (with warning)")
	
	# Test 4: Verify TypedDictionary handles String keys correctly
	print("Testing String key handling...")
	var string_keys = {
		"valid_string_key": "value1",
		"another_key": "value2"
	}
	set_component("WorldConfiguration", string_keys)
	
	await get_tree().process_frame
	
	result = get_component("WorldConfiguration")
	
	if not result.has("valid_string_key"):
		print("FAIL: Valid string key should be present")
		get_tree().quit(1)
		return
	
	if result.size() != 2:
		print("FAIL: Expected exactly 2 keys, got ", result.size())
		get_tree().quit(1)
		return
	print("PASS: String keys handled correctly")
	
	# Test 5: Test direct set_component with valid Dictionary
	print("Testing direct set_component usage...")
	var direct_config = {
		"direct_key1": 100,
		"direct_key2": "test_value"
	}
	set_component("WorldConfiguration", direct_config)
	
	await get_tree().process_frame
	
	result = get_component("WorldConfiguration")
	if not result.has("direct_key1") or result["direct_key1"] != 100:
		print("FAIL: Direct set_component should work with valid data")
		get_tree().quit(1)
		return
	print("PASS: Direct set_component works correctly")
	
	# Test 6: Test empty string as key (valid String key)
	print("Testing empty string as key...")
	var empty_key_config = {
		"": "empty_key_value",
		"normal_key": "normal_value"
	}
	set_component("WorldConfiguration", empty_key_config)
	
	await get_tree().process_frame
	
	result = get_component("WorldConfiguration")
	if not result.has(""):
		print("FAIL: Empty string is a valid String key")
		get_tree().quit(1)
		return
	
	if result[""] != "empty_key_value":
		print("FAIL: Empty string key value incorrect")
		get_tree().quit(1)
		return
	print("PASS: Empty string is accepted as a valid key")
	
	# Test 7: Test null value (valid as Variant)
	print("Testing null value...")
	var null_value_config = {
		"null_value": null,
		"normal_value": 42
	}
	set_component("WorldConfiguration", null_value_config)
	
	await get_tree().process_frame
	
	result = get_component("WorldConfiguration")
	if not result.has("null_value"):
		print("FAIL: null is a valid Variant value")
		get_tree().quit(1)
		return
	
	if result["null_value"] != null:
		print("FAIL: null value not preserved correctly")
		get_tree().quit(1)
		return
	print("PASS: null values are supported")
	
	# Test 8: Test very large dictionary
	print("Testing large dictionary...")
	var large_config = {}
	for i in range(1000):
		large_config["key_" + str(i)] = i * 2
	
	set_component("WorldConfiguration", large_config)
	
	await get_tree().process_frame
	
	result = get_component("WorldConfiguration")
	if result.size() != 1000:
		print("FAIL: Large dictionary size mismatch, expected 1000 got ", result.size())
		get_tree().quit(1)
		return
	
	if result["key_500"] != 1000:
		print("FAIL: Large dictionary value incorrect")
		get_tree().quit(1)
		return
	print("PASS: Large dictionaries handled correctly")
	
	# Test 9: Test StringName keys are converted to String
	print("Testing StringName key conversion...")
	var stringname_key_config = {}
	stringname_key_config[StringName("stringname_key")] = "stringname_value"
	set_component("WorldConfiguration", stringname_key_config)
	
	await get_tree().process_frame
	
	result = get_component("WorldConfiguration")
	# TypedDictionary should accept StringName as it's string-compatible
	if not (result.has("stringname_key") or result.has(StringName("stringname_key"))):
		print("FAIL: StringName key should work as String key")
		get_tree().quit(1)
		return
	print("PASS: StringName keys work correctly")
	
	# Test 10: Test that configuration is truly a singleton (only one instance)
	print("Testing singleton behavior...")
	set_component("WorldConfiguration", {"singleton_test": "value"})
	
	await get_tree().process_frame
	
	# Try to get from the "world singleton" (entity 0) - should work
	var first_access = get_component("WorldConfiguration")
	if not first_access.has("singleton_test"):
		print("FAIL: Singleton access failed")
		get_tree().quit(1)
		return
	
	# Update the configuration
	set_component("WorldConfiguration", {"singleton_test": "updated_value"})
	
	await get_tree().process_frame
	
	# Access again - should see the update
	var second_access = get_component("WorldConfiguration")
	if second_access["singleton_test"] != "updated_value":
		print("FAIL: Singleton not properly updated")
		get_tree().quit(1)
		return
	print("PASS: Singleton behavior verified")
	
	# Test 11: Self-assignment scenario - set the same config multiple times
	print("Testing self-assignment safety...")
	var self_assign_config = {
		"test_key": "test_value",
		"number": 42
	}
	set_component("WorldConfiguration", self_assign_config)
	
	await get_tree().process_frame
	
	# Set the same reference multiple times
	set_component("WorldConfiguration", self_assign_config)
	set_component("WorldConfiguration", self_assign_config)
	
	await get_tree().process_frame
	
	result = get_component("WorldConfiguration")
	if result["test_key"] != "test_value" or result["number"] != 42:
		print("FAIL: Self-assignment corrupted data")
		get_tree().quit(1)
		return
	
	# Read back and write again
	var retrieved = get_component("WorldConfiguration")
	set_component("WorldConfiguration", retrieved)
	
	await get_tree().process_frame
	
	result = get_component("WorldConfiguration")
	if result["test_key"] != "test_value":
		print("FAIL: Read-then-write cycle failed")
		get_tree().quit(1)
		return
	print("PASS: Self-assignment scenarios handled safely")
	
	print("All edge case and warning tests passed!")
	test_passed = true

func _process(delta) -> void:
	if test_passed:
		progress(delta)
		get_tree().quit(0)
