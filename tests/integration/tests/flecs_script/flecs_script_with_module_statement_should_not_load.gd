extends FlecsWorld

func _ready() -> void:
    print("Test: Flecs script module loading (failure)")

    # Do NOT configure the foo::bar module; the script should be skipped.

    set_progress_tick(PROGRESS_TICK_MANUAL)
    progress(0.016)

    # The script should NOT create ModuleLoadedEntity when module not configured
    run_system("stagehand_tests::Lookup Entities", {"names": ["ModuleLoadedEntity"]})

    var result = get_component("SceneChildrenResult")
    assert_eq(typeof(result), TYPE_DICTIONARY, "Lookup result is Dictionary")

    var found: Array = result.get("found", [])
    var missing: Array = result.get("missing", [])

    assert_eq(found.size(), 0, "ModuleLoadedEntity should NOT be found when module NOT configured")
    assert_eq(missing.size(), 1, "Exactly 1 missing entity expected")

    print("Flecs script module loading (failure) passed!")
    get_tree().quit(0)


func assert_eq(actual, expected, label: String) -> void:
    if actual != expected:
        _fail("%s: expected %s, got %s" % [label, str(expected), str(actual)])
    else:
        print("  PASS: %s" % label)


func _fail(msg: String) -> void:
    print("FAIL: %s" % msg)
    get_tree().quit(1)
