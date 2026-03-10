extends FlecsWorld

func _ready() -> void:
	print("Test: Flecs script module loading (success)")

	var configured_modules = get_modules_to_import()
	assert_true(configured_modules.has("foo::bar"), "modules_to_import contains foo::bar")

	set_progress_tick(PROGRESS_TICK_MANUAL)
	progress(0.016)

	var candidate_names := [
		"foo.bar.ModuleLoadedEntity",
		"foo::bar::ModuleLoadedEntity",
	]
	run_system("stagehand_tests::Lookup Entities", {"names": candidate_names})

	var result = get_component("SceneChildrenResult")
	assert_eq(typeof(result), TYPE_DICTIONARY, "Lookup result is Dictionary")

	var found: Array = result.get("found", [])
	var missing: Array = result.get("missing", [])

	assert_true(found.size() >= 1, "Module-loaded entity should be found when module is configured")
	assert_true(found.has("foo.bar.ModuleLoadedEntity") or found.has("foo::bar::ModuleLoadedEntity"),
		"Found entities include module-qualified ModuleLoadedEntity")
	assert_true(missing.size() <= 1, "At most one naming variant may be absent")

	print("Flecs script module loading (success) passed!")
	get_tree().quit(0)


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
