extends FlecsWorld

## Consolidated WorldConfiguration core behavior test.
## Covers: empty state → set → variant types → update → clear → re-set,
## nested dictionaries, persistence across frames, and explicit entity_id 0.

var test_passed = false

func _ready() -> void:
	print("Test: WorldConfiguration core behavior")

	# ── Test 1: Initial state is empty ───────────────────────────────────────
	var initial = get_component("WorldConfiguration")
	assert_eq(typeof(initial), TYPE_DICTIONARY, "Initial WorldConfiguration is Dictionary")
	assert_true(initial.is_empty(), "Initial WorldConfiguration is empty")

	# ── Test 2: Set with various Variant types ───────────────────────────────
	var config = {
		"int_value": 123,
		"float_value": 3.14,
		"string_value": "test",
		"vector_value": Vector3(1, 2, 3),
		"bool_value": false,
		"color_value": Color(1.0, 0.5, 0.0),
		"array_value": [1, 2, 3],
		"dict_value": {"nested": "data"}
	}
	set_component("WorldConfiguration", config)
	await get_tree().process_frame

	var registry = get_component("WorldConfiguration")
	assert_eq(typeof(registry), TYPE_DICTIONARY, "Registry is Dictionary after set")
	assert_eq(registry["int_value"], 123, "Int value preserved")
	assert_eq(registry["string_value"], "test", "String value preserved")
	assert_eq(registry["vector_value"], Vector3(1, 2, 3), "Vector3 value preserved")
	assert_eq(registry["bool_value"], false, "Bool value preserved")
	assert_eq(registry["color_value"], Color(1.0, 0.5, 0.0), "Color value preserved")
	assert_eq(registry["array_value"], [1, 2, 3], "Array value preserved")
	assert_eq(registry["dict_value"], {"nested": "data"}, "Dict value preserved")

	# ── Test 3: Update existing config ───────────────────────────────────────
	config["int_value"] = 999
	config["new_key"] = "added"
	set_component("WorldConfiguration", config)
	await get_tree().process_frame

	registry = get_component("WorldConfiguration")
	assert_eq(registry["int_value"], 999, "Updated int value")
	assert_eq(registry["new_key"], "added", "New key added alongside existing")
	assert_eq(registry["string_value"], "test", "Existing key preserved after update")

	# ── Test 4: Nested dictionaries preserve non-String key types ────────────
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
	var nested = registry["nested"]
	assert_eq(typeof(nested), TYPE_DICTIONARY, "Nested value is Dictionary")
	assert_true(nested.has(1), "Nested integer key present")
	assert_true(nested.has(StringName("named")), "Nested StringName key present")
	assert_true(nested.has(Vector2i(3, 4)), "Nested Vector2i key present")

	# ── Test 5: Clear via empty dictionary ───────────────────────────────────
	set_component("WorldConfiguration", {})
	await get_tree().process_frame

	registry = get_component("WorldConfiguration")
	assert_true(registry.is_empty(), "Registry empty after clear")

	# ── Test 6: Re-set after clearing ────────────────────────────────────────
	set_component("WorldConfiguration", {"restored": 42})
	await get_tree().process_frame

	registry = get_component("WorldConfiguration")
	assert_eq(registry["restored"], 42, "Value restored after clear")

	# ── Test 7: Explicit entity_id 0 works the same as default ───────────────
	set_component("WorldConfiguration", {"explicit_zero": true}, 0)
	await get_tree().process_frame

	registry = get_component("WorldConfiguration", 0)
	assert_eq(registry["explicit_zero"], true, "Explicit entity_id 0 round-trip")

	# ── Test 8: Persistence across multiple frames ───────────────────────────
	set_component("WorldConfiguration", {"persist": "value"})
	await get_tree().process_frame
	await get_tree().process_frame
	await get_tree().process_frame

	registry = get_component("WorldConfiguration")
	assert_eq(registry["persist"], "value", "Value persists across frames")

	# ── Test 9: Multiple rapid updates ───────────────────────────────────────
	for i in range(5):
		set_component("WorldConfiguration", {"counter": i})
		await get_tree().process_frame
		registry = get_component("WorldConfiguration")
		assert_eq(registry["counter"], i, "Rapid update #%d" % i)

	print("All WorldConfiguration core behavior tests passed!")
	test_passed = true

func _process(delta) -> void:
	if test_passed:
		progress(delta)
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
