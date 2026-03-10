class_name MinimapManager extends Node

# ENUMS
enum AnimationMode {
	LOOP,
	ONCE
}

const MAX_ENEMY_SAMPLES := 256

var minimap_material: ShaderMaterial
var stage: Stage
var stage_bounds: Rect2
var world: FlecsWorld
var camera_node: Camera2D
var enemy_multimesh: MultiMesh
var minimap_ready: bool = false
var cached_altar_points: PackedVector2Array = PackedVector2Array()
var cached_altar_states: PackedFloat32Array = PackedFloat32Array()

@onready var minimap_rect: ColorRect = $"../../StatusOverlay/%Minimap"


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	minimap_material = minimap_rect.material as ShaderMaterial
	call_deferred("_initialise_minimap")


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta: float) -> void:
	if not minimap_ready:
		_initialise_minimap()
	if minimap_ready:
		_update_minimap()


func _initialise_minimap() -> void:
	if minimap_ready:
		return
	stage = owner
	if stage == null or minimap_material == null:
		return
	stage_bounds = stage.get_stage_bounds()
	world = stage.world
	camera_node = stage.get_node_or_null("Camera")
	var enemies_instance: MultiMeshInstance2D = stage.get_node_or_null("World/Enemies")
	enemy_multimesh = enemies_instance.multimesh if enemies_instance else null
	cached_altar_points = _build_altar_points(stage.get_altar_positions())
	cached_altar_states = _build_altar_states(stage.get_altar_states())
	minimap_material.set_shader_parameter("altar_points", cached_altar_points)
	minimap_material.set_shader_parameter("altar_states", cached_altar_states)
	minimap_material.set_shader_parameter("player_visible", 0.0)
	minimap_material.set_shader_parameter("camera_visible", 0.0)
	minimap_ready = world != null and camera_node != null and stage_bounds.size != Vector2.ZERO


func _update_minimap() -> void:
	if minimap_material == null:
		return
	minimap_material.set_shader_parameter("minimap_size", minimap_rect.size)
	_update_altar_markers()
	_update_player_marker()
	_update_camera_rect()
	_update_enemy_markers()


func _update_player_marker() -> void:
	if world == null:
		minimap_material.set_shader_parameter("player_visible", 0.0)
		return
	if not world.has_component(ECS.components.stagehand_demos.surwave.PlayerPosition):
		minimap_material.set_shader_parameter("player_visible", 0.0)
		return

	var player_variant: Variant = world.get_component(ECS.components.stagehand_demos.surwave.PlayerPosition)
	if player_variant is Vector2 and stage_bounds.size != Vector2.ZERO:
		var player_position: Vector2 = player_variant
		minimap_material.set_shader_parameter("player_point", _normalise_point(player_position))
		minimap_material.set_shader_parameter("player_visible", 1.0)
	else:
		minimap_material.set_shader_parameter("player_visible", 0.0)


func _update_camera_rect() -> void:
	if camera_node == null or stage_bounds.size == Vector2.ZERO:
		minimap_material.set_shader_parameter("camera_visible", 0.0)
		return
	var viewport_size: Vector2 = camera_node.get_viewport().get_visible_rect().size
	var zoom_vector: Vector2 = camera_node.zoom
	var rect_size := Vector2(
		viewport_size.x / max(zoom_vector.x, 0.0001),
		viewport_size.y / max(zoom_vector.y, 0.0001)
	)
	var rect_position: Vector2 = camera_node.global_position - rect_size * 0.5
	var rect_min: Vector2 = rect_position
	var rect_max: Vector2 = rect_position + rect_size
	var stage_min: Vector2 = stage_bounds.position
	var stage_max: Vector2 = stage_bounds.position + stage_bounds.size
	var offset: Vector2 = Vector2.ZERO
	if rect_min.x < stage_min.x:
		offset.x = stage_min.x - rect_min.x
	elif rect_max.x > stage_max.x:
		offset.x = stage_max.x - rect_max.x
	if rect_min.y < stage_min.y:
		offset.y = stage_min.y - rect_min.y
	elif rect_max.y > stage_max.y:
		offset.y = stage_max.y - rect_max.y
	rect_min += offset
	rect_max += offset
	var normalised_position: Vector2 = _normalise_point(rect_min)
	var normalised_size := Vector2(
		rect_size.x / stage_bounds.size.x,
		rect_size.y / stage_bounds.size.y
	).clamp(Vector2.ZERO, Vector2.ONE)
	minimap_material.set_shader_parameter("camera_rect", Vector4(normalised_position.x, normalised_position.y, normalised_size.x, normalised_size.y))
	minimap_material.set_shader_parameter("camera_visible", 1.0)


func _update_enemy_markers() -> void:
	var positions := _sample_enemy_positions()
	minimap_material.set_shader_parameter("enemy_points", positions)
	minimap_material.set_shader_parameter("enemy_count", positions.size())


func _update_altar_markers() -> void:
	if stage == null:
		return
	cached_altar_states = _build_altar_states(stage.get_altar_states())
	minimap_material.set_shader_parameter("altar_states", cached_altar_states)


func _sample_enemy_positions() -> PackedVector2Array:
	var positions := PackedVector2Array()
	if enemy_multimesh == null or stage_bounds.size == Vector2.ZERO:
		return positions
	var visible_count: int = enemy_multimesh.visible_instance_count
	if visible_count <= 0 or visible_count > enemy_multimesh.instance_count:
		visible_count = enemy_multimesh.instance_count
	if visible_count <= 0:
		return positions
	var ratio: float = float(visible_count) / float(MAX_ENEMY_SAMPLES)
	var stride: int = max(int(ceil(ratio)), 1)
	for instance_index in range(0, visible_count, stride):
		var transform: Transform2D = enemy_multimesh.get_instance_transform_2d(instance_index)
		positions.append(_normalise_point(transform.origin))
		if positions.size() >= MAX_ENEMY_SAMPLES:
			break
	return positions


func _build_altar_points(world_positions: Array[Vector2]) -> PackedVector2Array:
	var points := PackedVector2Array()
	if stage_bounds.size == Vector2.ZERO:
		return points
	for position in world_positions:
		points.append(_normalise_point(position))
	if points.size() < 4:
		var rect := stage_bounds
		var fallback := [
			rect.position,
			rect.position + Vector2(rect.size.x, 0.0),
			rect.position + rect.size,
			rect.position + Vector2(0.0, rect.size.y)
		]
		for corner in fallback:
			if points.size() >= 4:
				break
			points.append(_normalise_point(corner))
	elif points.size() > 4:
		var trimmed := PackedVector2Array()
		for i in range(4):
			trimmed.append(points[i])
		points = trimmed
	while points.size() < 4:
		points.append(Vector2.ZERO)
	return points


func _build_altar_states(raw_states: Array[int]) -> PackedFloat32Array:
	var states := PackedFloat32Array()
	for state_value in raw_states:
		states.append(float(clamp(state_value, 0, 1)))
	if states.size() > 4:
		var trimmed := PackedFloat32Array()
		for i in range(4):
			trimmed.append(states[i])
		states = trimmed
	while states.size() < 4:
		states.append(0.0)
	return states


func _normalise_point(world_point: Vector2) -> Vector2:
	if stage_bounds.size == Vector2.ZERO:
		return Vector2.ZERO
	var relative := world_point - stage_bounds.position
	return Vector2(relative.x / stage_bounds.size.x, relative.y / stage_bounds.size.y).clamp(Vector2.ZERO, Vector2.ONE)
