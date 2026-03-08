class_name Altar extends AnimatedObject

@export var upgrade_screen_scene: PackedScene

var upgrade_screen

enum AltarState {
	AVAILABLE, # 0
	SPENT # 1
}

const AltarAnimationFrames: Dictionary[AltarState, Array] = {
	AltarState.AVAILABLE: [0, 1, 2, 3, 4, 5],
	AltarState.SPENT: [9, 8, 7, 6]
}
const AltarAnimationModes: Dictionary[AltarState, AnimationMode] = {
	AltarState.AVAILABLE: AnimationMode.LOOP,
	AltarState.SPENT: AnimationMode.ONCE
}


func _get_animation_frames(p_state: AltarState) -> Array:
	return AltarAnimationFrames[p_state]


func _get_animation_mode(p_state: AltarState):
	return AltarAnimationModes[p_state]


func _ready() -> void:
	set_state(AltarState.AVAILABLE)


func _on_area_2d_body_entered(body: Node2D) -> void:
	if not body.is_in_group("Players"): return
	if state != AltarState.AVAILABLE: return

	var screen = _ensure_upgrade_screen()
	if screen == null: return
	screen.show_with_available_upgrades(self, body)


func _ensure_upgrade_screen():
	if is_instance_valid(upgrade_screen): return upgrade_screen
	if upgrade_screen_scene == null:
		push_warning("Altar: upgrade_screen_scene is not assigned.")
		return null
	upgrade_screen = upgrade_screen_scene.instantiate()
	if upgrade_screen == null: return null

	get_tree().root.add_child(upgrade_screen)
	if not upgrade_screen.upgrade_finalized.is_connected(_on_upgrade_screen_finalized):
		upgrade_screen.upgrade_finalized.connect(_on_upgrade_screen_finalized)
	return upgrade_screen


func _on_upgrade_screen_finalized(requester: Node, _upgradeable: UpgradeManager.Upgradeable) -> void:
	if requester != self: return
	set_state(AltarState.SPENT)
	$AnimationPlayer.play("go_off")
