extends FlecsWorld

## WorldConfiguration edge cases and warning behaviors.
## Focuses on invalid inputs, boundary conditions, and warning paths
## that are NOT covered by the core_behavior test.

var test_passed = false

func _ready() -> void:
	print("Test: WorldConfiguration edge cases and warning behaviors")

	# ── Test 1: Non-zero entity_id on get returns empty dict (with warning) ──
	var result = get_component("WorldConfiguration", 123)
	assert_eq(typeof(result), TYPE_DICTIONARY, "Invalid entity_id get → Dictionary")
	assert_true(result.is_empty(), "Invalid entity_id get → empty")

	# ── Test 2: Non-zero entity_id on set is rejected (with warning) ─────────
	set_component("WorldConfiguration", {"should_not_appear": true}, 999)
	await get_tree().process_frame
	var singleton = get_component("WorldConfiguration")
	assert_true(not singleton.has("should_not_appear"), "Invalid entity_id set does nothing")

	# ── Test 3: Non-Dictionary type is rejected (with warning) ───────────────
	set_component("WorldConfiguration", {"baseline": 1})
	await get_tree().process_frame
	set_component("WorldConfiguration", "this_is_a_string")
	await get_tree().process_frame
	result = get_component("WorldConfiguration")
	assert_true(not result.has("this_is_a_string"), "Non-Dictionary type rejected")

	# ── Test 4: Empty string is a valid key ──────────────────────────────────
	set_component("WorldConfiguration", {"": "empty_key_value", "normal": "ok"})
	await get_tree().process_frame
	result = get_component("WorldConfiguration")
	assert_eq(result[""], "empty_key_value", "Empty string key accepted")

	# ── Test 5: Null value is valid ──────────────────────────────────────────
	set_component("WorldConfiguration", {"null_val": null, "normal": 42})
	await get_tree().process_frame
	result = get_component("WorldConfiguration")
	assert_true(result.has("null_val"), "null value key present")
	assert_eq(result["null_val"], null, "null value preserved")

	# ── Test 6: Large dictionary (1000 entries) ──────────────────────────────
	var large = {}
	for i in range(1000):
		large["key_" + str(i)] = i * 2
	set_component("WorldConfiguration", large)
	await get_tree().process_frame
	result = get_component("WorldConfiguration")
	assert_eq(result.size(), 1000, "Large dictionary size")
	assert_eq(result["key_500"], 1000, "Large dictionary spot-check")

	# ── Test 7: StringName keys are accepted (string-compatible) ─────────────
	var sn_config = {}
	sn_config[StringName("sn_key")] = "sn_value"
	set_component("WorldConfiguration", sn_config)
	await get_tree().process_frame
	result = get_component("WorldConfiguration")
	assert_true(result.has("sn_key") or result.has(StringName("sn_key")),
		"StringName key works as String key")

	# ── Test 8: Self-assignment safety ───────────────────────────────────────
	var cfg = {"test_key": "value", "number": 42}
	set_component("WorldConfiguration", cfg)
	await get_tree().process_frame
	set_component("WorldConfiguration", cfg)
	set_component("WorldConfiguration", cfg)
	await get_tree().process_frame
	result = get_component("WorldConfiguration")
	assert_eq(result["test_key"], "value", "Self-assignment data intact")

	# Read-back-then-write cycle
	var retrieved = get_component("WorldConfiguration")
	set_component("WorldConfiguration", retrieved)
	await get_tree().process_frame
	result = get_component("WorldConfiguration")
	assert_eq(result["test_key"], "value", "Read-write cycle intact")

	print("All edge case and warning tests passed!")
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
