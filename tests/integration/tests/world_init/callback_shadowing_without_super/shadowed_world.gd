extends FlecsWorld

signal script_exit_tree_called

var callback_counts := {
	"enter_tree": 0,
	"ready": 0,
	"process": 0,
	"physics_process": 0,
	"exit_tree": 0,
}


func _enter_tree() -> void:
	callback_counts["enter_tree"] += 1


func _ready() -> void:
	callback_counts["ready"] += 1


func _process(_delta: float) -> void:
	callback_counts["process"] += 1


func _physics_process(_delta: float) -> void:
	callback_counts["physics_process"] += 1


func _exit_tree() -> void:
	callback_counts["exit_tree"] += 1
	script_exit_tree_called.emit()


func get_callback_count(callback_name: String) -> int:
	return callback_counts.get(callback_name, 0)


func reset_frame_callback_counts() -> void:
	callback_counts["process"] = 0
	callback_counts["physics_process"] = 0