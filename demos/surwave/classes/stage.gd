class_name Stage extends Node

signal gem_balance_changed(new_balance: int)
signal player_instantiated(player: Player)

@export_category("Procedural Object Placement")
## The terrain margin in terms of tile count of the Borders tile map layer.
@export var terrain_margin: int = 3
## The number of objects to place. The actual number of total objects placed may be lower due to skip logic
@export var foliage_count: int = 5000

@export_category("Initial Enemy Population Spawning")
## The number of times the spawning logic will run. Each iteration spawns a batch of enemies.
@export var spawn_iterations: int = 50
## The margin from the terrain edge, creating an outer boundary for enemy spawning.
@export var spawn_outer_margin: float = 0.0
## The margin from the center of the map, creating an inner boundary to keep the center clear of initial enemies.
@export var spawn_inner_margin: float = 220.0
## A value greater than 0 biases enemy spawns towards the corners of the spawn area. A value of 0 disables the bias.
@export var spawn_corner_bias: float = 3.0
## Controls the radial distribution of spawns. > 1.0 pushes spawns outward, < 1.0 pulls them inward.
@export var spawn_radial_exponent: float = 1.2

@export_category("Scene Tree")
@export var end_screen_scene: PackedScene

@export_category("Camera zoom out after upgrades")
@export var camera_zoom_out_speed: float = 1.0

const MAXIMUM_GEMS: int = 99
const PAUSE_MENU = preload("uid://bxvjrn4aui4lk")

var altar_nodes: Array[Node]
var portal_nodes: Array[Node]
var half_outer_boundary: float
var landmark_occupied_areas: Array[Rect2]
var spawn_iteration_counter: int = 0
var upgrade_manager: UpgradeManager
var _frame_counter: int = 0
var gem_balance: int = 0
var _is_zooming_out: bool = false
var _camera_zoom_triggered: bool = false
var _timer: Timer

@onready var status_overlay: StatusOverlay = $StatusOverlay
@onready var world: FlecsWorld = $World
@onready var terrain: MeshInstance2D = $Terrain
@onready var terrain_object_multimesh_parents: Array[Node2D] = [$Terrain/Foliage]
@onready var landmarks: Node = $Landmarks

func _ready() -> void:
	add_to_group("stage")
	
	_connect_audio_manager_to_world()
	
	_validate_terrain()
	half_outer_boundary = terrain.mesh.size.x / 2.0

	_mark_landmark_occupied_areas()
	_place_terrain_objects()
	_initialise_altars()
	_initialise_portals()
	_instantiate_player()
	_instantiate_camera()
	_set_camera_limits()
	_apply_difficulty_setting()
	_spawn_initial_enemy_population()
	_set_stage_timer()
	await _play_the_sound_track()


func _validate_terrain() -> void:
	assert(terrain != null, "Node at path $Terrain is missing or not a MeshInstance2D.")
	assert(terrain.mesh != null, "A terrain mesh must be set for the stage.")
	assert(terrain.mesh.size.x == terrain.mesh.size.y, "Terrain mesh must be a square.")

	assert(landmarks != null, "'Landmarks' node is missing or is invalid.")


func _mark_landmark_occupied_areas() -> void:
	if landmarks == null: return

	landmark_occupied_areas.clear()
	for landmark in landmarks.get_children():
		if not landmark is Node2D:
			continue

		var sprite: Sprite2D
		for child in landmark.get_children():
			if child is StaticBody2D:
				for grandchild in child.get_children():
					if grandchild is Sprite2D:
						sprite = grandchild
						break
			if sprite != null:
				break

		if sprite != null:
			var landmark_size: Vector2 = sprite.get_rect().size
			var landmark_position: Vector2 = landmark.position - landmark_size / 2
			landmark_occupied_areas.append(Rect2(landmark_position, landmark_size))


func _place_terrain_objects() -> void:
	for mm_parent in terrain_object_multimesh_parents:
		if mm_parent == null: continue

		var half_terrain_size = terrain.mesh.size / 2.0
		for multimesh_instance in mm_parent.get_children():
			if multimesh_instance.multimesh == null or not multimesh_instance is MultiMeshInstance2D: continue

			multimesh_instance.multimesh.instance_count = foliage_count

			for i in foliage_count:
				var random_position = Vector2(randf_range(-half_terrain_size.x, half_terrain_size.x), randf_range(-half_terrain_size.y, half_terrain_size.y))
				multimesh_instance.multimesh.set_instance_transform_2d(i, Transform2D(randf_range(0, TAU), random_position))


func _initialise_altars() -> void:
	altar_nodes = get_tree().get_nodes_in_group("Altars")
	

func _initialise_portals() -> void:
	portal_nodes = get_tree().get_nodes_in_group("Portals")


func _instantiate_player() -> void:
	var player_scene: PackedScene = preload("res://surwave/scenes/Player/player.tscn")
	var player_instance: Player = player_scene.instantiate() as Player
	player_instance.position = Vector2(0, 0)
	if DifficultySetting.value == 0: player_instance.max_health *= 2.5
	elif DifficultySetting.value == 2: player_instance.max_health = 0.1
	player_instance.connect("died", _on_player_died)
	player_instance.connect("gem_collected", _on_gem_collected)
	add_child(player_instance)

	upgrade_manager = player_instance.get_node_or_null("UpgradeManager")
	if upgrade_manager:
		upgrade_manager.upgrade_purchased.connect(_on_upgrade_purchased)
	else:
		push_warning("Stage: Could not find UpgradeManager node in player scene.")
	
	player_instantiated.emit(player_instance)


func _instantiate_camera() -> void:
	var camera_scene: PackedScene = preload("res://surwave/scenes/Camera/camera.tscn")
	var camera_instance: Camera2D = camera_scene.instantiate() as Camera2D
	add_child(camera_instance)
	camera_instance.position = Vector2(0, 0)


func _set_camera_limits() -> void:
	var camera = get_node("Camera") as Camera2D
	if camera:
		camera.set_limits(terrain.mesh.size)
	else:
		push_warning("Stage: Camera node not found, couldn't set limits.")


func _apply_difficulty_setting() -> void:
	match DifficultySetting.value:
		0:
			gem_balance += 10
			# health doubling applied in _instantiate_player()
		1:
			pass
			# no adjustments
		2:
			pass
			# health reduction applied in _instantiate_player()


func _connect_audio_manager_to_world() -> void:
	AudioManager.world = world
	AudioManager.connect_to_flecs_signal()


func _spawn_initial_enemy_population() -> void:
	while spawn_iteration_counter < spawn_iterations:
		var enemy_types_to_spawn: Dictionary = {
			"BugSmall": 17,
			"BugHumanoid": 2,
			"BugLarge": 1,
		}
		
		for enemy_type in enemy_types_to_spawn:
			var count: int = enemy_types_to_spawn[enemy_type]
			var prefab_path: String = _enemy_prefab_path_for_type(enemy_type)
			if prefab_path.is_empty():
				continue
			var transforms: Array = []
			
			for i in range(count):
				var transform: Transform2D = _get_random_spawn_transform()
				transforms.append(transform)

			for transform in transforms:
				world.instantiate_prefab(prefab_path, {
					ECS.components.stagehand.transform.Transform2D_: transform,
					ECS.components.stagehand.transform.Position2D: transform.origin,
				})
		spawn_iteration_counter += 1


func _enemy_prefab_path_for_type(enemy_type: String) -> String:
	match enemy_type:
		"BugSmall":
			return "stagehand_demos::surwave::BugSmall"
		"BugHumanoid":
			return "stagehand_demos::surwave::BugHumanoid"
		"BugLarge":
			return "stagehand_demos::surwave::BugLarge"
		_:
			return ""


func _get_random_spawn_transform() -> Transform2D:
	var spawn_pos: Vector2
	var valid_pos: bool = false
	
	var spawn_area_max: Vector2 = terrain.mesh.size / 2.0 - Vector2(spawn_outer_margin, spawn_outer_margin)
	var bias_strength: float = spawn_corner_bias if spawn_corner_bias > 0.0 else 0.0
	var max_weight: float = 1.0 + bias_strength
	# Choose angles uniformly so directions are equally likely, compute angle-specific max radius inside the rectangular spawn area,
	# then sample radius using an inverse CDF so radial density increases outward while remaining normalized per-angle.
	while not valid_pos:
		var angle: float = randf_range(0.0, TAU) # Sample angle uniformly to ensure equal directional counts

		if bias_strength > 0.0: # Apply corner bias
			var weight: float = 1.0 + bias_strength * ((1.0 - cos(4.0 * angle)) * 0.5)
			if randf() > (weight / max_weight):
				continue

		var cos_angle: float = cos(angle)
		var sin_angle: float = sin(angle)

		# Compute the angle-specific maximum radius constrained by the spawn rectangle
		var max_radius: float
		if abs(cos_angle) > 0.0001 and abs(sin_angle) > 0.0001:
			max_radius = min(spawn_area_max.x / abs(cos_angle), spawn_area_max.y / abs(sin_angle))
		else:
			max_radius = spawn_area_max.x if abs(cos_angle) > abs(sin_angle) else spawn_area_max.y

		# Sample a radius normalized on [spawn_inner_margin, max_radius]
		var radius: float = _sample_radius_for_angle(max_radius, spawn_inner_margin, spawn_radial_exponent)
		var x: float = cos_angle * radius
		var y: float = sin_angle * radius

		spawn_pos = Vector2(x, y)
		valid_pos = true
		for area in landmark_occupied_areas:
			if area.has_point(spawn_pos):
				valid_pos = false
				break
	
	return Transform2D(0.0, spawn_pos)


func _radial_cdf_unnormalized(radius: float, inner_margin: float, exponent: float) -> float:
	# Returns an unnormalized primitive for the target radial density. The density model is proportional to (r - in_margin)^p in area terms,
	# and converting to polar coordinates yields a primitive that can be evaluated and inverted numerically.
	var radius_minus_margin: float = radius - inner_margin
	if radius_minus_margin <= 0.0:
		return 0.0
	return (pow(radius_minus_margin, exponent + 2) / (exponent + 2)) + inner_margin * (pow(radius_minus_margin, exponent + 1) / (exponent + 1))


func _sample_radius_for_angle(max_radius: float, inner_margin: float, exponent: float) -> float:
	# Samples a radius in [in_margin, max_radius] according to the radial distribution controlled by exponent p.
	# Uses numeric inversion (binary search) of the unnormalized CDF for robustness.
	if max_radius <= inner_margin:
		return inner_margin

	var cdf_max: float = _radial_cdf_unnormalized(max_radius, inner_margin, exponent)
	if cdf_max <= 0.0:
		# degenerate: fall back to uniform between inner and max
		return randf_range(inner_margin, max_radius)

	var uniform_sample: float = randf() * cdf_max
	var lower_bound: float = inner_margin
	var higher_bound: float = max_radius
	for i in range(28):
		var mid: float = (lower_bound + higher_bound) * 0.5
		if _radial_cdf_unnormalized(mid, inner_margin, exponent) < uniform_sample:
			lower_bound = mid
		else:
			higher_bound = mid
	return (lower_bound + higher_bound) * 0.5


func get_stage_bounds() -> Rect2:
	if terrain == null or terrain.mesh == null:
		return Rect2()
	var size: Vector2 = terrain.mesh.size
	return Rect2(-size / 2.0, size)


func get_altar_positions() -> Array[Vector2]:
	var positions: Array[Vector2] = []
	for altar in altar_nodes:
		if altar is Node2D:
			positions.append((altar as Node2D).global_position)
	return positions


func get_altar_states() -> Array[int]:
	var states: Array[int] = []
	for altar in altar_nodes:
		if altar is Altar:
			states.append(int((altar as Altar).state))
		else:
			states.append(0)
	return states


func _set_stage_timer(wait_time: float = 360.0) -> void:
	_timer = Timer.new()
	_timer.wait_time = wait_time
	_timer.autostart = true
	_timer.one_shot = true
	_timer.timeout.connect(_on_timer_timeout)
	add_child(_timer)


func _play_the_sound_track(delay: float = 5.0) -> void:
	var tween = create_tween()
	tween.tween_property(AudioManager.intro, "volume_db", -80, delay).set_ease(Tween.EASE_OUT)
	await tween.finished
	AudioManager.intro.stop()
	AudioManager.sound_track.play()


func _process(_delta: float) -> void:
	_frame_counter += 1
	if _frame_counter % 5 == 0:
		status_overlay.set_time_remaining(_timer.time_left)
	
	if not _camera_zoom_triggered and _timer.time_left < _timer.wait_time / 2.0:
		_is_zooming_out = true
		_camera_zoom_triggered = true

	if _is_zooming_out:
		var camera = get_node("Camera") as Camera2D
		if camera:
			camera.zoom = camera.zoom.lerp(Vector2.ONE, 1.0 - exp(-camera_zoom_out_speed * _delta))
			if camera.zoom.is_equal_approx(Vector2.ONE):
				camera.zoom = Vector2.ONE
				_is_zooming_out = false


func _on_gem_collected(value: int) -> void:
	gem_balance = min(gem_balance + value, MAXIMUM_GEMS)
	gem_balance_changed.emit(gem_balance)


func _on_upgrade_purchased(cost: int) -> void:
	gem_balance -= cost
	gem_balance_changed.emit(gem_balance)


func _on_timer_timeout() -> void:
	var end_screen_instance = end_screen_scene.instantiate()
	add_child(end_screen_instance)
	AudioManager.victory.play()


func _on_player_died() -> void:
	var end_screen_instance = end_screen_scene.instantiate()
	add_child(end_screen_instance)
	end_screen_instance.set_defeat()
	AudioManager.sound_track.stop()
	await get_tree().create_timer(1.5).timeout
	AudioManager.defeat.play()


func _unhandled_input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_cancel"):
		add_child(PAUSE_MENU.instantiate())
	get_tree().root.set_input_as_handled()
