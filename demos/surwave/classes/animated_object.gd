@abstract class_name AnimatedObject extends Node2D

enum AnimationMode {
	LOOP,
	ONCE
}

var state: int = -1
var animation_interval: float = 0.25
var _animation_frames: Array
var _animation_mode
var _animation_timer: float = 0.0
var _animation_current_frame: int = 0

@export var sprite: Sprite2D

signal animation_frame_changed(frame: int)

func _process(delta: float) -> void:
	_animate(delta)


@abstract func _get_animation_frames(p_state) -> Array
@abstract func _get_animation_mode(p_state) -> Array

func set_state(new_state) -> void:
	if state == new_state: return
	state = new_state
	_animation_frames = _get_animation_frames(new_state)
	_animation_mode = _get_animation_mode(new_state)
	_animation_current_frame = 0
	_animation_timer = 0.0
	if not _animation_frames.is_empty():
		sprite.frame = _animation_frames[0]
		animation_frame_changed.emit(sprite.frame)


func _animate(delta: float) -> void:
	_animation_timer += delta
	if ((_animation_mode == AnimationMode.ONCE and _animation_current_frame >= _animation_frames.size() - 1)
		or _animation_timer < animation_interval):
			return
	
	if _animation_timer >= animation_interval:
		_animation_timer = 0.0
		_animation_current_frame = (_animation_current_frame + 1) % _animation_frames.size()
		sprite.frame = _animation_frames[_animation_current_frame]
		animation_frame_changed.emit(sprite.frame)
