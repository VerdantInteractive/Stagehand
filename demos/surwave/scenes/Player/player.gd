class_name Player extends AnimatedObject

signal player_took_damage

var max_health: float = 10.0
var health: float
var health_bar_original_modulation: Color
var health_bar_full_colour: Color = Color.LIME_GREEN
var health_bar_empty_colour: Color = Color(0.8392157, 0.101960786, 0.53333336, 1)

var movement_speed: float
var adjusted_movement_speed: float

@export_category("Movement")
@export_range(0.0, 100.0, 0.1) var acceleration_smoothing: float = 20
@export_range(0.0, 1.0, 0.01) var movement_speed_penalty_when_shooting: float = 0.4

@export_category("Combat")
@export_range(0.0, 45.0, 0.5)
var projectile_spread_degrees: float = 7.5

@export var end_screen_scene: PackedScene

var input_movement_vector: Vector2
var is_colliding: bool = false
var position_at_frame_start: Vector2
var movement_in_frame: Vector2
var has_moved_this_frame: bool = false
var aim_direction = Vector2.UP

var shoot_weapon_timer: float
var can_shoot_weapon: bool = false
var just_fired_weapon: bool = false
var is_dead: bool = false

enum PlayerState {
	IDLE,
	TALKING,
	RELOADING,
	RUNNING,
	SHOOTING_UP,
	SHOOTING_DOWN,
	RUNNING_AND_SHOOTING_UP,
	RUNNING_AND_SHOOTING_DOWN,
	SHOOTING_RIGHT,
	RUNNING_AND_SHOOTING_RIGHT,
	DYING
}

const PlayerAnimationFrames: Dictionary[PlayerState, Array] = {
	PlayerState.IDLE: [0, 2, 3, 4, 5],
	PlayerState.TALKING: [9], # unused
	PlayerState.RELOADING: [16, 17, 18, 19, 20], # unused
	PlayerState.RUNNING: [24, 25, 26, 27],
	PlayerState.SHOOTING_RIGHT: [32, 35],
	PlayerState.SHOOTING_UP: [33, 34],
	PlayerState.SHOOTING_DOWN: [37, 38],
	PlayerState.RUNNING_AND_SHOOTING_RIGHT: [48, 51],
	PlayerState.RUNNING_AND_SHOOTING_UP: [49, 50],
	PlayerState.RUNNING_AND_SHOOTING_DOWN: [52, 53],
	PlayerState.DYING: [40, 41, 42, 43, 44, 45, 46]
}
const PlayerAnimationModes: Dictionary[PlayerState, AnimationMode] = {
	PlayerState.IDLE: AnimationMode.LOOP,
	PlayerState.TALKING: AnimationMode.ONCE,
	PlayerState.RELOADING: AnimationMode.ONCE,
	PlayerState.RUNNING: AnimationMode.LOOP,
	PlayerState.SHOOTING_RIGHT: AnimationMode.LOOP,
	PlayerState.SHOOTING_UP: AnimationMode.LOOP,
	PlayerState.SHOOTING_DOWN: AnimationMode.LOOP,
	PlayerState.RUNNING_AND_SHOOTING_RIGHT: AnimationMode.LOOP,
	PlayerState.RUNNING_AND_SHOOTING_UP: AnimationMode.LOOP,
	PlayerState.RUNNING_AND_SHOOTING_DOWN: AnimationMode.LOOP,
	PlayerState.DYING: AnimationMode.ONCE
}
const first_shooting_animation_frames: Array[int] = [
	PlayerAnimationFrames[PlayerState.SHOOTING_RIGHT][0],
	PlayerAnimationFrames[PlayerState.SHOOTING_UP][0],
	PlayerAnimationFrames[PlayerState.SHOOTING_DOWN][0],
	PlayerAnimationFrames[PlayerState.RUNNING_AND_SHOOTING_RIGHT][0],
	PlayerAnimationFrames[PlayerState.RUNNING_AND_SHOOTING_UP][0],
	PlayerAnimationFrames[PlayerState.RUNNING_AND_SHOOTING_DOWN][0]
]

const PROJECTILE = preload("uid://cdgbhqr7en0pv")

func _get_animation_frames(p_state: PlayerState) -> Array:
	return PlayerAnimationFrames[p_state]

func _get_animation_mode(p_state: PlayerState):
	return PlayerAnimationModes[p_state]

signal died
signal gem_collected(value: int)

@onready var world: FlecsWorld = get_node("../World")
@onready var upgrade_manager: UpgradeManager = $UpgradeManager
@onready var character_body: CharacterBody2D = $CharacterBody2D
@onready var _sprite: Sprite2D = $CharacterBody2D/Sprite2D
@onready var health_bar: ProgressBar = $CharacterBody2D/HealthBar
@onready var damage_cooldown_timer: Timer = $DamageCooldownTimer

func _ready() -> void:
	animation_frame_changed.connect(_on_animation_frame_changed)
	set_state(PlayerState.IDLE)
	shoot_weapon_timer = animation_interval
	can_shoot_weapon = true
	
	health = max_health
	health_bar.value = health_bar.max_value
	health_bar_original_modulation = health_bar.modulate
	
	world.stagehand_signal_emitted.connect(_on_flecs_signal)
	
		
func _process(delta: float) -> void:
	just_fired_weapon = false # Reset at the beginning of every frame
	if is_dead:
		super._process(delta)
		return
	_tick_cooldowns(delta)
	_handle_input(delta)
	super._process(delta);


func _on_animation_frame_changed(frame: int) -> void:
	if is_dead:
		return
	if first_shooting_animation_frames.has(frame):
		just_fired_weapon = true
		if can_shoot_weapon:
			_fire_projectile()


func _fire_projectile() -> void:
	var projectile_count: int = _get_projectile_count()
	var spread_radians: float = deg_to_rad(projectile_spread_degrees)
	var middle_index: float = (projectile_count - 1) * 0.5

	for projectile_index in range(projectile_count):
		var projectile: Node2D = PROJECTILE.instantiate()
		var rotation_offset: float = spread_radians * (projectile_index - middle_index)
		projectile.global_position = character_body.global_position
		projectile.direction = aim_direction.rotated(rotation_offset)
		add_child(projectile)

	can_shoot_weapon = false
	shoot_weapon_timer = 0.0
	AudioManager.projectile.play()


func _tick_cooldowns(delta: float) -> void:
	shoot_weapon_timer += delta
	if not can_shoot_weapon and shoot_weapon_timer >= animation_interval:
		can_shoot_weapon = true


func _poll_aim_direction() -> void:
	var controller_aim_direction: Vector2 = Input.get_vector("aim_west", "aim_east", "aim_north", "aim_south")
	if controller_aim_direction.length() > 0.125: # Aiming with the controller
		aim_direction = controller_aim_direction.normalized()
	else:
		aim_direction = (get_global_mouse_position() - character_body.global_position).normalized() # Aiming with the mouse


func _handle_input(delta: float) -> void:
	position_at_frame_start = character_body.global_position

	var is_shooting_input := Input.is_action_pressed("shoot_weapon")

	var is_shooting := is_shooting_input or shoot_weapon_timer < animation_interval

	var base_movement_speed: float = _get_base_movement_speed()
	adjusted_movement_speed = base_movement_speed * (1 - movement_speed_penalty_when_shooting) if is_shooting else base_movement_speed

	
	_poll_aim_direction()
	var is_aiming_up: bool = abs(aim_direction.y) > abs(aim_direction.x) and aim_direction.y < 0
	var is_aiming_down: bool = abs(aim_direction.y) > abs(aim_direction.x) and aim_direction.y > 0

	input_movement_vector = Input.get_vector("move_left", "move_right", "move_up", "move_down")
	var target_velocity := input_movement_vector * adjusted_movement_speed
	character_body.velocity = character_body.velocity.lerp(target_velocity, 1 - exp(-delta * acceleration_smoothing))

	is_colliding = character_body.move_and_slide()
	world.set_component(ECS.components.stagehand_demos.surwave.PlayerPosition, character_body.global_position)

	movement_in_frame = character_body.global_position - position_at_frame_start
	has_moved_this_frame = movement_in_frame.length() > 0.01

	var new_state = state

	# Sprite Animations
	if is_shooting:
		if aim_direction.x > 0:
			_sprite.flip_h = false
		elif aim_direction.x < 0:
			_sprite.flip_h = true
	elif has_moved_this_frame:
		if Input.is_action_pressed("move_right") and movement_in_frame.x > 0:
			_sprite.flip_h = false
		elif Input.is_action_pressed("move_left") and movement_in_frame.x < 0:
			_sprite.flip_h = true

	if has_moved_this_frame:
		if is_shooting:
			if is_aiming_up:
				new_state = PlayerState.RUNNING_AND_SHOOTING_UP
			elif is_aiming_down:
				new_state = PlayerState.RUNNING_AND_SHOOTING_DOWN
			else:
				new_state = PlayerState.RUNNING_AND_SHOOTING_RIGHT
		else:
			if not is_colliding:
				new_state = PlayerState.RUNNING
	else:
		if is_shooting:
			if is_aiming_up:
				new_state = PlayerState.SHOOTING_UP
			elif is_aiming_down:
				new_state = PlayerState.SHOOTING_DOWN
			else:
				new_state = PlayerState.SHOOTING_RIGHT
		else:
			new_state = PlayerState.IDLE
	
	set_state(new_state)


func _get_base_movement_speed() -> float:
	if upgrade_manager != null:
		return float(upgrade_manager.get_upgrade_value(UpgradeManager.Upgradeable.SPEED))
	return movement_speed


func _get_projectile_count() -> int:
	if upgrade_manager != null:
		return max(1, int(upgrade_manager.get_upgrade_value(UpgradeManager.Upgradeable.PROJECTILE_COUNT)))
	return 1


func _on_damage_cooldown_timeout() -> void:
	health_bar.modulate = health_bar_original_modulation


func _on_health_bar_value_changed(value: float) -> void:
	var fill_style: StyleBoxFlat = health_bar.get_theme_stylebox("fill")
	fill_style.bg_color = health_bar_empty_colour.lerp(health_bar_full_colour, health_bar.value / health_bar.max_value)


func _on_flecs_signal(signal_name: StringName, data: Dictionary) -> void:
	if is_dead: return
	if signal_name == "enemy_hit_player":
		if (damage_cooldown_timer.time_left > 0): return
		
		player_took_damage.emit()
		AudioManager.player_hurt.play()
		
		health -= data.damage_amount
		health_bar.value = max(health / max_health, 0)
		health_bar.modulate = Color.WHITE
		
		if health > 0:
			damage_cooldown_timer.start()
		else:
			await _handle_death()
			health_bar.visible = false


func _handle_death() -> void:
	if is_dead: return
	
	is_dead = true
	character_body.velocity = Vector2.ZERO
	input_movement_vector = Vector2.ZERO
	$ShockwaveManager/Timer.stop()
	set_state(PlayerState.DYING)
	AudioManager.player_death.play()
	var dying_animation_frames: int = PlayerAnimationFrames[PlayerState.DYING].size()
	var dying_duration: float = float(dying_animation_frames) * animation_interval
	await get_tree().create_timer(dying_duration).timeout
	emit_signal("died")
