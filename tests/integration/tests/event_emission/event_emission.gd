extends FlecsWorld

## Comprehensive integration tests for the emit_event() method.
## Tests event emission with/without data, with/without source entities,
## error handling, and Flecs observer integration.

func _ready() -> void:
	print("Test: Event Emission – emit_event() method with Flecs observers")
	
	set_progress_tick(PROGRESS_TICK_MANUAL)
	
	# ── Test 1: Basic event emission (no source entity) ──────────────────────
	print("\n=== Test 1: Basic event emission (no source entity) ===")
	
	print("Test: Can we call emit_event with just a name?")
	emit_event(&"SimplePing")
	print("Simple event emitted successfully")
	
	print("Now setting components...")
	set_component("EventReceivedCount", 0)
	set_component("TestEventACount", 0)
	progress(0.016)
	
	print("About to emit event with data...")
	emit_event(&"TestEventA", {"message": "hello", "value": 42})
	print("Event emitted, calling progress...")
	progress(0.016)
	
	print("Getting components...")
	var count: int = get_component("EventReceivedCount")
	print("EventReceivedCount: ", count)
	assert_eq(count, 1, "EventReceivedCount should be 1")
	
	var event_a_count: int = get_component("TestEventACount")
	assert_eq(event_a_count, 1, "TestEventACount should be 1")
	
	var last_event: Dictionary = get_component("LastEventData")
	assert_eq(last_event.get("name"), &"TestEventA", "Event name should be TestEventA")
	
	var data: Dictionary = last_event.get("data", {})
	assert_eq(data.get("message"), "hello", "Message should be 'hello'")
	assert_eq(data.get("value"), 42, "Value should be 42")
	assert_eq(last_event.get("source_entity_id", 0), 0, "Source entity ID should be 0")
	
	var event_a_data: Dictionary = get_component("TestEventAData")
	assert_eq(event_a_data.get("count"), 1, "TestEventA observer count should be 1")
	
	# ── Test 2: Event emission with source entity ────────────────────────────
	print("\n=== Test 2: Event emission with source entity ===")
	var entity_id = create_entity("EventSource")
	
	set_component("EventReceivedCount", 0)
	set_component("TestEventBCount", 0)
	progress(0.016)
	
	emit_event(&"TestEventB", {"type": "with_source", "count": 100}, entity_id)
	progress(0.016)
	
	count = get_component("EventReceivedCount")
	assert_eq(count, 1, "EventReceivedCount should be 1")
	
	var event_b_count: int = get_component("TestEventBCount")
	assert_eq(event_b_count, 1, "TestEventBCount should be 1")
	
	last_event = get_component("LastEventData")
	assert_eq(last_event.get("name"), &"TestEventB", "Event name should be TestEventB")
	
	data = last_event.get("data", {})
	assert_eq(data.get("type"), "with_source", "Type should be 'with_source'")
	assert_eq(data.get("count"), 100, "Count should be 100")
	
	var source_id = last_event.get("source_entity_id", 0)
	assert_true(source_id != 0, "Source entity ID should be non-zero")
	assert_eq(get_entity_name(source_id), "EventSource", "Source entity name should be 'EventSource'")
	
	var event_b_data: Dictionary = get_component("TestEventBData")
	assert_eq(event_b_data.get("count"), 1, "TestEventB observer count should be 1")
	assert_true(event_b_data.get("source_entity_id", 0) != 0, "TestEventB observer should have source_entity_id")
	
	# ── Test 3: Event emission without data ──────────────────────────────────
	print("\n=== Test 3: Event emission without data ===")
	set_component("EventReceivedCount", 0)
	progress(0.016)
	
	emit_event(&"EmptyEvent")
	progress(0.016)
	
	count = get_component("EventReceivedCount")
	assert_eq(count, 1, "EventReceivedCount should be 1")
	
	last_event = get_component("LastEventData")
	assert_eq(last_event.get("name"), &"EmptyEvent", "Event name should be EmptyEvent")
	
	data = last_event.get("data", {})
	assert_true(data.is_empty(), "Data should be empty")
	
	# ── Test 4: Multiple events in sequence ──────────────────────────────────
	print("\n=== Test 4: Multiple events in sequence ===")
	set_component("EventReceivedCount", 0)
	set_component("TestEventACount", 0)
	set_component("TestEventBCount", 0)
	progress(0.016)
	
	emit_event(&"TestEventA", {"sequence": 1})
	progress(0.016)
	emit_event(&"TestEventB", {"sequence": 2})
	progress(0.016)
	emit_event(&"TestEventA", {"sequence": 3})
	progress(0.016)
	
	count = get_component("EventReceivedCount")
	assert_eq(count, 3, "EventReceivedCount should be 3")
	
	event_a_count = get_component("TestEventACount")
	assert_eq(event_a_count, 2, "TestEventACount should be 2")
	
	event_b_count = get_component("TestEventBCount")
	assert_eq(event_b_count, 1, "TestEventBCount should be 1")
	
	# ── Test 5: Event with complex data structures ───────────────────────────
	print("\n=== Test 5: Event with complex data structures ===")
	set_component("EventReceivedCount", 0)
	progress(0.016)
	
	var complex_data = {
		"nested": {
			"array": [1, 2, 3],
			"dict": {"a": "alpha", "b": "beta"}
		},
		"vector": Vector2(10.5, 20.5),
		"color": Color(1.0, 0.5, 0.0, 1.0)
	}
	
	emit_event(&"ComplexEvent", complex_data)
	progress(0.016)
	
	count = get_component("EventReceivedCount")
	assert_eq(count, 1, "EventReceivedCount should be 1")
	
	last_event = get_component("LastEventData")
	assert_eq(last_event.get("name"), &"ComplexEvent", "Event name should be ComplexEvent")
	
	data = last_event.get("data", {})
	assert_true(data.has("nested"), "Data should have 'nested' key")
	
	var nested: Dictionary = data.get("nested", {})
	assert_true(nested.has("array"), "Nested should have 'array' key")
	
	var array: Array = nested.get("array", [])
	assert_eq(array.size(), 3, "Array size should be 3")
	assert_eq(array[0], 1, "Array[0] should be 1")
	assert_eq(array[1], 2, "Array[1] should be 2")
	assert_eq(array[2], 3, "Array[2] should be 3")
	
	assert_true(data.has("vector"), "Data should have 'vector' key")
	var vec: Vector2 = data.get("vector", Vector2())
	assert_true(vec.is_equal_approx(Vector2(10.5, 20.5)), "Vector should be (10.5, 20.5)")
	
	# ── Test 6: Event with invalid source entity (should warn) ───────────────
	print("\n=== Test 6: Event emission with invalid source entity ===")
	set_component("EventReceivedCount", 0)
	progress(0.016)
	
	# This should push a warning but not crash
	emit_event(&"InvalidSourceEvent", {"test": "data"}, 999999999)
	progress(0.016)
	
	count = get_component("EventReceivedCount")
	assert_eq(count, 0, "EventReceivedCount should be 0 (event not emitted)")
	
	print("\nAll event emission tests passed!")
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
