class_name EndScreen extends CanvasLayer

@onready var panel_container: PanelContainer = %PanelContainer

func _ready() -> void:
	panel_container.pivot_offset = panel_container.size / 2
	var tween := create_tween()
	tween.tween_property(panel_container, "scale", Vector2.ZERO, 0.0) # workaround
	tween.tween_property(panel_container, "scale", Vector2.ONE, 2.7).set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_BACK)
	
	get_tree().paused = true
	$%RestartButton.pressed.connect(on_restart_button_pressed)
	$%QuitButton.pressed.connect(on_quit_button_pressed)


func set_defeat() -> void:
	$%TitleLabel.text = "Dead!"
	$%DescriptionLabel.text = "You didn't last ..."


func on_restart_button_pressed() -> void:
	ScreenTransition.transition()
	await ScreenTransition.transitioned_halfway
	get_tree().paused = false
	get_tree().change_scene_to_file("res://Surwave/scenes/Stage 01/stage_01.tscn")


func on_quit_button_pressed() -> void:
	get_tree().quit()
