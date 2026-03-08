extends FlecsWorld

## Engine-runtime integration checks only.
## Most error-path/state-path behavior is now covered in unit tests.
## This suite focuses on class-based Variant values (Object refs),
## which require a running engine instance.

func _ready() -> void:
	print("Test: FlecsWorld integration-only variant behavior")

	set_progress_tick(PROGRESS_TICK_MANUAL)

	# ── Test 1: class-based Variant values survive dictionary component round-trip ──
	var payload := {
		"self": self ,
		"root": get_tree().root,
	}
	set_component("TestDictionary", payload)
	var round_trip = get_component("TestDictionary")
	assert_true(round_trip is Dictionary, "TestDictionary returns a Dictionary")
	assert_true(round_trip.has("self"), "Round-tripped dictionary has 'self' key")
	assert_true(round_trip.has("root"), "Round-tripped dictionary has 'root' key")
	assert_true(round_trip["self"] == self , "Object reference for self is preserved")
	assert_true(round_trip["root"] == get_tree().root, "Object reference for root is preserved")

	# ── Test 2: singleton path still works in-engine for tag operations ───────────
	assert_true(not has_component("TestTag"), "Singleton TestTag initially absent")
	add_component("TestTag")
	assert_true(has_component("TestTag"), "add_component() adds singleton tag")
	remove_component("TestTag")
	assert_true(not has_component("TestTag"), "remove_component() removes singleton tag")

	print("All FlecsWorld integration-only tests passed!")
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
