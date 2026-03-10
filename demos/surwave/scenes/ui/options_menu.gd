extends CanvasLayer

signal back_button_pressed

@onready var difficulty_slider: HSlider = %DifficultySlider
@onready var difficulty_name: Label = %DifficultyName
@onready var difficulty_description: Label = %DifficultyDescription
@onready var sfx_slider: HSlider = %SFXSlider
@onready var music_slider: HSlider = %MusicSlider
@onready var window_mode_button: AudibleButton = %WindowModeButton
@onready var back_button: AudibleButton = %BackButton


func _ready() -> void:
	difficulty_slider.value_changed.connect(_on_difficulty_slider_changed)
	sfx_slider.value_changed.connect(_on_audio_slider_changed.bind("SFX"))
	music_slider.value_changed.connect(_on_audio_slider_changed.bind("Music"))
	window_mode_button.pressed.connect(_on_window_mode_button_pressed)
	back_button.pressed.connect(_on_back_button_pressed)
	_update_options()


func _update_options() -> void:
	difficulty_slider.value = DifficultySetting.value
	sfx_slider.value = _get_bus_volume_percent("SFX")
	music_slider.value = _get_bus_volume_percent("Music")
	window_mode_button.text = "Windowed"
	if DisplayServer.window_get_mode() == DisplayServer.WINDOW_MODE_FULLSCREEN:
		window_mode_button.text = "Fullscreen"


func _get_bus_volume_percent(bus_name: String) -> float:
	var bus_index = AudioServer.get_bus_index(bus_name)
	var bus_volume_db = AudioServer.get_bus_volume_db(bus_index)
	return db_to_linear(bus_volume_db)


func _set_bus_volume_percent(bus_name: String, percent: float) -> void:
	var bus_index = AudioServer.get_bus_index(bus_name)
	var bus_volume_db = linear_to_db(percent)
	AudioServer.set_bus_volume_db(bus_index, bus_volume_db)


func _on_window_mode_button_pressed() -> void:
	var mode = DisplayServer.window_get_mode()
	if mode != DisplayServer.WINDOW_MODE_FULLSCREEN:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
	else:
		DisplayServer.window_set_flag(DisplayServer.WINDOW_FLAG_BORDERLESS, false)
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)
	
	_update_options()


func _on_difficulty_slider_changed(value: float) -> void:
	var value_int = int(value)
	DifficultySetting.value = value_int
	match value_int:
		0:
			difficulty_name.text = "A walk in the park"
			difficulty_description.text = "You start with 10 gems and can take more hits"
		1:
			difficulty_name.text = "A fair challenge"
			difficulty_description.text = "You will survive with a bit of agility and planning"
		2:
			difficulty_name.text = "Hardcore"
			difficulty_description.text = "Get hit once and you're dead. Not for the faint hearted"


func _on_audio_slider_changed(value: float, bus_name: String) -> void:
	_set_bus_volume_percent(bus_name, value)
	if bus_name == "SFX": AudioManager.click.play()


func _on_back_button_pressed():
	ScreenTransition.transition()
	await ScreenTransition.transitioned_halfway
	back_button_pressed.emit()
