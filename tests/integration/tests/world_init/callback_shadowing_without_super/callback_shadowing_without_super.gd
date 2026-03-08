extends Node

var forwarded_signals: Array[Dictionary] = []
var exit_tree_signal_received := false


func _ready() -> void:
	print("Test: FlecsWorld lifecycle callback shadowing without super")

	var world = $ShadowedWorld
	world.stagehand_signal_emitted.connect(_on_stagehand_signal_emitted)
	world.script_exit_tree_called.connect(_on_script_exit_tree_called)

	world.set_progress_tick(world.PROGRESS_TICK_MANUAL)

	# Allow deferred post-tree setup to run even though the child script shadows _ready().
	await get_tree().process_frame

	assert_eq(world.get_callback_count("enter_tree"), 1, "GDScript _enter_tree ran once")
	assert_eq(world.get_callback_count("ready"), 1, "GDScript _ready ran once")

	var renderer = world.get_node_or_null("InstancedRenderer3D")
	assert_true(renderer != null, "InstancedRenderer3D child exists")
	assert_true(renderer.get_discovered_instance_uniforms().has("UnitColor"), "Renderer setup survived shadowed _ready()")

	world.run_system("stagehand_tests::Query Instanced Renderers", {})
	world.progress(0.016)
	var render_info = world.get_component("SceneChildrenResult")
	assert_eq(render_info["renderer_count"], 1, "Instanced renderer registered without super._ready()")

	world.emit_event(&"LifecycleShadowedEvent", {"ok": true})
	await get_tree().process_frame
	assert_eq(forwarded_signals.size(), 1, "stagehand_signal_emitted forwarded without super._enter_tree()")
	assert_eq(forwarded_signals[0]["name"], &"LifecycleShadowedEvent", "Forwarded signal name matches")
	assert_true(forwarded_signals[0]["data"].get("ok", false), "Forwarded signal data matches")

	world.reset_frame_callback_counts()
	world.set_component("TickCount", 0)
	world.set_progress_tick(world.PROGRESS_TICK_RENDERING)
	await get_tree().process_frame
	await get_tree().process_frame
	await get_tree().process_frame

	var rendering_ticks = world.get_component("TickCount")
	assert_true(rendering_ticks >= 2, "Rendering auto-progress works without super._process()")
	assert_true(world.get_callback_count("process") >= 2, "GDScript _process still runs without super")

	world.reset_frame_callback_counts()
	world.set_component("TickCount", 0)
	world.set_progress_tick(world.PROGRESS_TICK_PHYSICS)
	await get_tree().process_frame
	await get_tree().process_frame
	await get_tree().process_frame
	await get_tree().process_frame

	var physics_ticks = world.get_component("TickCount")
	assert_true(physics_ticks >= 2, "Physics auto-progress works without super._physics_process()")
	assert_true(world.get_callback_count("physics_process") >= 2, "GDScript _physics_process still runs without super")

	world.queue_free()
	await get_tree().process_frame
	await get_tree().process_frame

	assert_true(exit_tree_signal_received, "GDScript _exit_tree ran without super")

	print("All lifecycle shadowing tests passed!")
	get_tree().quit(0)


func _on_stagehand_signal_emitted(signal_name: StringName, data: Dictionary) -> void:
	forwarded_signals.append({"name": signal_name, "data": data})


func _on_script_exit_tree_called() -> void:
	exit_tree_signal_received = true


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