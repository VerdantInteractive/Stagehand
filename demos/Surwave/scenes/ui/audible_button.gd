class_name AudibleButton extends Button


func _ready() -> void:
	pressed.connect(_on_pressed)


func _on_pressed() -> void:
	AudioManager.click.play()
