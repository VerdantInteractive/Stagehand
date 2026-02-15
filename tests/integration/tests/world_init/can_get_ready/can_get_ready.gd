extends FlecsWorld


func _init() -> void:
	print("Test: FlecsWorld _init() ran")


func _ready() -> void:
	print("Test: FlecsWorld is ready")
	get_tree().quit(0)
