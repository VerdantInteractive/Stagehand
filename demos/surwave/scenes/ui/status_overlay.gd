class_name StatusOverlay extends CanvasLayer

@onready var animation_player: AnimationPlayer = $AnimationPlayer
@onready var gem_count_label: Label = $OverlayMargin/GemCount/VBoxContainer/GemCount
@onready var time_remaining_label: Label = %TimeRemaining

func _ready() -> void:
	await get_tree().process_frame
	var stage = get_tree().get_first_node_in_group("stage")
	if stage:
		stage.gem_balance_changed.connect(update_gem_count)
		update_gem_count(stage.gem_balance)

func update_gem_count(new_balance: int) -> void:
	animation_player.play("gem_count_grow")
	gem_count_label.text = str(new_balance)


func set_time_remaining(seconds: float) -> void:
	if time_remaining_label:
		var minutes: int = int(seconds) / 60
		var remaining_seconds: int = int(seconds) % 60
		time_remaining_label.text = "%d:%02d" % [minutes, remaining_seconds]
