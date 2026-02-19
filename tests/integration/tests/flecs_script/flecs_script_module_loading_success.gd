extends FlecsWorld

func _ready() -> void:
    print("Test: Flecs script module loading (success)")

    # Configure world to load the foo::bar module
    set_modules_to_import(["foo::bar"])

    set_progress_tick(PROGRESS_TICK_MANUAL)
    progress(0.016)

    # The script should create an entity named ModuleLoadedEntity when loaded
    run_system("stagehand_tests::Lookup Entities", {"names": ["ModuleLoadedEntity"]})

    var result = get_component("SceneChildrenResult")
    assert_eq(typeof(result), TYPE_DICTIONARY, "Lookup result is Dictionary")

    var found: Array = result.get("found", [])
    var missing: Array = result.get("missing", [])

    assert_eq(missing.size(), 0, "ModuleLoadedEntity should be found when module configured (missing: %s)" % str(missing))
    assert_eq(found.size(), 1, "Exactly 1 module entity found")

    print("Flecs script module loading (success) passed!")
    get_tree().quit(0)


func assert_eq(actual, expected, label: String) -> void:
    if actual != expected:
        _fail("%s: expected %s, got %s" % [label, str(expected), str(actual)])
    else:
        print("  PASS: %s" % label)


func _fail(msg: String) -> void:
    print("FAIL: %s" % msg)
    get_tree().quit(1)
