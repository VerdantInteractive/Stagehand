extends Node

func _ready():
	print("Test: Editor FlecsWorld availability and ComponentSchema")

	# Ensure FlecsWorld class exists
	assert(ClassDB.class_exists("FlecsWorld"), "FlecsWorld class must be registered")

	# Instantiate a FlecsWorld; if running in the editor, verify manual progress
	var world = FlecsWorld.new()
	if Engine.is_editor_hint():
		assert(world.get_progress_tick() == FlecsWorld.PROGRESS_TICK_MANUAL, "Editor FlecsWorld should use manual progress tick")

	# ComponentSchema should be available and return defaults for a known component
	assert(ClassDB.class_exists("ComponentSchema"), "ComponentSchema class must be registered")
	var schema = ComponentSchema.new()
	var default_wc = schema.get_component_default("WorldConfiguration")
	print("WorldConfiguration default:", default_wc)

	# Free created objects to avoid ObjectDB leaks in headless/editor tests
	if schema:
		schema.free()
	if world:
		world.free()

	print("All editor FlecsWorld tests passed!")
	get_tree().quit(0)
