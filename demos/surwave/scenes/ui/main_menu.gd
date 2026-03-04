extends CanvasLayer

const OPTIONS_MENU = preload("uid://crlhaxk0hrl4f")
const CREDITS_SCREEN = preload("uid://dq8qlbgcgavbj")

func _ready() -> void:
	$%PlayButton.pressed.connect(_on_play_pressed)
	$%OptionsButton.pressed.connect(_on_options_pressed)
	$%CreditsButton.pressed.connect(_on_credits_pressed)
	$%QuitButton.pressed.connect(_on_quit_pressed)
	
	AudioManager.intro.play()


func _on_play_pressed() -> void:
	ScreenTransition.transition()
	await ScreenTransition.transitioned_halfway
	get_tree().change_scene_to_file("res://game.tscn")


func _on_options_pressed() -> void:
	ScreenTransition.transition()
	await ScreenTransition.transitioned_halfway
	var options_menu_instance = OPTIONS_MENU.instantiate()
	add_child(options_menu_instance)
	options_menu_instance.back_button_pressed.connect(_on_options_menu_closed.bind(options_menu_instance))


func _on_credits_pressed() -> void:
	ScreenTransition.transition()
	await ScreenTransition.transitioned_halfway
	var credits_menu_instance = CREDITS_SCREEN.instantiate()
	add_child(credits_menu_instance)
	credits_menu_instance.back_button_pressed.connect(_on_options_menu_closed.bind(credits_menu_instance))


func _on_quit_pressed() -> void:
	get_tree().quit()


func _on_options_menu_closed(options_menu_instance: Node) -> void:
	options_menu_instance.queue_free()
