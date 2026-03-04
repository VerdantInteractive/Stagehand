class_name UpgradeScreen extends CanvasLayer

signal upgrade_finalized(requester: Node, upgradeable: UpgradeManager.Upgradeable)

@export var upgrade_card_scene: PackedScene
@export var resume_delay_seconds: float = .5

var upgrade_manager: UpgradeManager
var was_game_paused: bool = false
var purchase_in_progress: bool = false
var requesting_node: Node

@onready var card_container: HBoxContainer = %CardContainer

func _ready() -> void:
	visible = false


func show_with_available_upgrades(requester: Node = null, player_body: Node = null) -> void:
	if not _ensure_upgrade_manager(player_body):
		push_warning("UpgradeScreen: No UpgradeManager found on any Player.")
		return
	
	AudioManager.upgrade_screen_loop.play()
	
	requesting_node = requester
	purchase_in_progress = false
	var upgrades := upgrade_manager.get_available_upgrades()
	set_ability_upgrades(upgrades)
	was_game_paused = get_tree().paused
	_set_game_paused(true)
	visible = true
	$AnimationPlayer.play("in")
	await $AnimationPlayer.animation_finished


func hide_screen() -> void:
	$AnimationPlayer.play("out")
	await $AnimationPlayer.animation_finished
	AudioManager.upgrade_screen_loop.stop()
	_set_game_paused(was_game_paused)
	requesting_node = null
	purchase_in_progress = false


func set_ability_upgrades(upgrades: Dictionary[UpgradeManager.Upgradeable, Dictionary]):
	for child in card_container.get_children(): child.queue_free()
	var animate_in_delay: float = 0
	for upgradeable in UpgradeManager.Upgradeable.values():
		if not upgrades.has(upgradeable): continue
		
		var card_instance := upgrade_card_scene.instantiate() as UpgradeCard
		card_container.add_child(card_instance)
		card_instance.upgrade_selected.connect(_on_upgrade_selected)
		card_instance.set_upgrade_info(upgradeable, upgrades[upgradeable])
		card_instance.animate_in(animate_in_delay)
		animate_in_delay += 0.2


func _on_upgrade_selected(upgradeable: UpgradeManager.Upgradeable) -> void:
	if purchase_in_progress: return
	if upgrade_manager == null: return
	purchase_in_progress = true
	upgrade_manager.upgrade(upgradeable)
	_disable_all_cards()
	_finalize_upgrade(upgradeable)


func _wait_resume_delay() -> void:
	var delay: float = max(resume_delay_seconds, 0.0)
	if delay <= 0.0: return
	
	var timer := Timer.new()
	timer.wait_time = delay
	timer.one_shot = true
	timer.process_mode = Node.PROCESS_MODE_ALWAYS
	add_child(timer)
	timer.start()
	await timer.timeout
	timer.queue_free()


func _finalize_upgrade(upgradeable: UpgradeManager.Upgradeable) -> void:
	await _wait_resume_delay()
	var requester := requesting_node
	await hide_screen()
	emit_signal("upgrade_finalized", requester, upgradeable)
	purchase_in_progress = false


func _disable_all_cards() -> void:
	for child in card_container.get_children():
		if child is UpgradeCard:
			(child as UpgradeCard).set_selection_enabled(false)


func _unhandled_input(event: InputEvent) -> void:
	if not visible: return
	if purchase_in_progress: return
	if event.is_action_pressed("menu"):
		_cancel_selection()
		get_viewport().set_input_as_handled()

	if event.is_action_pressed("directional_controller_input"):
		if get_viewport().gui_get_focus_owner() == null:
			if card_container.get_child_count() > 0:
				(card_container.get_child(0) as UpgradeCard).select_button.grab_focus()
			get_viewport().set_input_as_handled()


func _cancel_selection() -> void:
	hide_screen()


func _ensure_upgrade_manager(player_body: Node = null) -> bool:
	if upgrade_manager != null and is_instance_valid(upgrade_manager): return true

	upgrade_manager = _get_manager_from_body(player_body)
	if upgrade_manager != null: return true

	upgrade_manager = _find_manager_in_players_group()
	return upgrade_manager != null


func _get_manager_from_body(player_body: Node) -> UpgradeManager:
	if player_body == null: return null

	var player_root := player_body.owner
	if player_root == null: player_root = player_body.get_parent()
	if player_root == null: return null

	var manager_node := player_root.get_node_or_null("UpgradeManager")
	return manager_node as UpgradeManager


func _find_manager_in_players_group() -> UpgradeManager:
	var player := get_tree().get_first_node_in_group("Players")
	if player == null: return null
	var manager_node := player.get_node_or_null("UpgradeManager")
	return manager_node as UpgradeManager


func _set_game_paused(paused: bool) -> void:
	get_tree().paused = paused
