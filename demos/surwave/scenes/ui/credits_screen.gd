extends CanvasLayer

signal back_button_pressed

@onready var back_button: AudibleButton = %BackButton


func _ready() -> void:
	back_button.pressed.connect(_on_back_button_pressed)


func _on_back_button_pressed():
	ScreenTransition.transition()
	await ScreenTransition.transitioned_halfway
	back_button_pressed.emit()
