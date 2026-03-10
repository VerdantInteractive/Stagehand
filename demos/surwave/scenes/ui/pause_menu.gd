extends CanvasLayer

@onready var panel_container: PanelContainer = $MarginContainer/PanelContainer

const OPTIONS_MENU_SCENE = preload("uid://crlhaxk0hrl4f")

var is_closing: bool = false

func _ready() -> void:
	get_tree().paused = true
	panel_container.pivot_offset = panel_container.size / 2

	$%ResumeButton.pressed.connect(_on_resume_button_pressed)
	$%OptionsButton.pressed.connect(_on_options_button_pressed)
	$%QuitToMenuButton.pressed.connect(_on_quit_to_menu_button_pressed)
	
	$AnimationPlayer.play("in")
	
	var tween = create_tween()
	tween.tween_property(panel_container, "scale", Vector2.ZERO, 0.0)
	tween.tween_property(panel_container, "scale", Vector2.ONE, 0.3).set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_BACK)


func _close() -> void:
	if is_closing: return
	is_closing = true
	
	$AnimationPlayer.play("out")
	
	var tween = create_tween()
	tween.tween_property(panel_container, "scale", Vector2.ONE, 0.0)
	tween.tween_property(panel_container, "scale", Vector2.ZERO, 0.3).set_ease(Tween.EASE_IN).set_trans(Tween.TRANS_BACK)
	await tween.finished
	
	get_tree().paused = false
	queue_free()


func _unhandled_input(event: InputEvent) -> void:
	if not event.is_action_pressed("ui_cancel"):
		return
	_close()
	get_tree().root.set_input_as_handled()


func _on_resume_button_pressed() -> void:
	_close()


func _on_options_button_pressed() -> void:
	ScreenTransition.transition()
	await ScreenTransition.transitioned_halfway
	var options_menu_instance = OPTIONS_MENU_SCENE.instantiate()
	add_child(options_menu_instance)
	options_menu_instance.back_button_pressed.connect(_on_options_back_button_pressed.bind(options_menu_instance))


func _on_quit_to_menu_button_pressed() -> void:
	AudioManager.sound_track.stop()
	ScreenTransition.transition()
	await ScreenTransition.transitioned_halfway
	get_tree().paused = false
	AudioManager.intro.volume_db = -10.0
	AudioManager.intro.play()
	get_tree().change_scene_to_file("res://surwave/scenes/ui/main_menu.tscn")


func _on_options_back_button_pressed(options_menu_instance: Node) -> void:
	options_menu_instance.queue_free()
