extends CanvasLayer

signal transitioned_halfway

var dont_emit_transitioned_halfway_signal = false

func transition() -> void:
	$AnimationPlayer.play("transition")
	await transitioned_halfway
	$AnimationPlayer.play_backwards("transition")


func _emit_transitioned_halfway() -> void:
	if dont_emit_transitioned_halfway_signal:
		dont_emit_transitioned_halfway_signal = true
		return
	transitioned_halfway.emit()
