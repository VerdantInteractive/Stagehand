class_name Gem extends Node2D

@export var timer: Timer
@export var blink_duration: float = 5.0
@export var blink_interval: float = 0.2

var is_about_to_expire: bool = false
var blink_timer: float = 0.0

@onready var sprite: Sprite2D = $Sprite2D

func _ready() -> void:
	AudioManager.gem_drop.play()

func _process(delta: float) -> void:
	if not is_about_to_expire: return
	# Blink by toggling visibility when close to full expiry
	blink_timer += delta
	if blink_timer > blink_interval:
		sprite.visible = not sprite.visible
		blink_timer = 0.0


func gather_animation(percent: float, start_position: Vector2) -> void:
	var player := get_tree().get_first_node_in_group("Players") as Node2D
	if player == null: return
	global_position = start_position.lerp(player.global_position, percent)


func _on_area_entered(body: Node2D) -> void:
	if not body.is_in_group("Players"): return
	var player_root: Node = body.get_parent()
	if player_root == null: return

	var tween := create_tween()
	tween.set_parallel() # Two tweens in parallel
	tween.tween_method(gather_animation.bind(global_position), 0.0, 1.0, 1.0).set_ease(Tween.EASE_IN_OUT).set_trans(Tween.TRANS_QUINT)
	tween.tween_property(sprite, "scale", Vector2(0.3, 0.3), 0.5).set_delay(0.5) # 0.5 + 0.5 add up to the duration of the other tween
	tween.chain() # But wait for both to complete before calling the callback
	tween.tween_callback(func():
		AudioManager.gem_collect.play()
		player_root.gem_collected.emit(1)
		queue_free()
	)


func _on_expiry() -> void:
	if not is_about_to_expire: # Start blinking
		timer.start(blink_duration)
		is_about_to_expire = true
	else: # Fully expired (ExpirationTimer.wait_time + blink_duration has elapsed)
		queue_free()
