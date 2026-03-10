class_name ShockwaveManager extends Node

@export var singleton_component_name: String = ECS.components.stagehand_demos.surwave.ShockwaveData
@export var radiation_speed: float = 1.0
@export var inner_ring_range: Vector2 = Vector2(0.5, -0.2)
@export var outer_ring_range: Vector2 = Vector2(0.05, 0.4)
@export var hit_radius_adjustment: float = 0.75

var is_firing: bool = false
var has_played_sound: bool = false
var max_radius: float = 0.5
var active_radius: float = 0.0
var mesh_size: float = 0.0

@onready var world: FlecsWorld = get_node("../../World")
@onready var player: Player = $".."
@onready var timer: Timer = $Timer
@onready var vfx: MeshInstance2D = %ShockwaveVFX
@onready var shader_material: ShaderMaterial = vfx.material as ShaderMaterial
@onready var quad_mesh: QuadMesh = vfx.mesh as QuadMesh
@onready var upgrade_manager: UpgradeManager = $"../UpgradeManager"


func _ready() -> void:
	if world == null:
		push_warning("ShockwaveManager: World node not found.")
		set_process(false)
		return

	if shader_material == null:
		push_warning("ShockwaveManager: ShockwaveVFX requires a ShaderMaterial.")
		set_process(false)
		return

	if quad_mesh == null:
		push_warning("ShockwaveManager: ShockwaveVFX mesh must be a QuadMesh.")
		set_process(false)
		return

	if not is_equal_approx(quad_mesh.size.x, quad_mesh.size.y):
		push_warning("ShockwaveManager: ShockwaveVFX mesh must be square.")
		set_process(false)
		return

	mesh_size = quad_mesh.size.x * 0.5

func _process(delta: float) -> void:
	if not is_firing:
		return

	active_radius += radiation_speed * delta
	var clamped_radius: float = clampf(active_radius, 0.0, max_radius)
	var progress: float = clamped_radius / max_radius # 0.0 to 1.0
	var upgrade_radius_multiplier: float = upgrade_manager.get_upgrade_value(UpgradeManager.Upgradeable.SHOCKWAVE)
	var effective_progress: float = clampf(progress * upgrade_radius_multiplier, 0.0, 1.0)
	var effective_radius: float = effective_progress * mesh_size
	var shader_radius: float = effective_progress * max_radius

	if not vfx.visible:
		vfx.visible = true

	shader_material.set_shader_parameter("radius", shader_radius)
	shader_material.set_shader_parameter("inner_ring", lerpf(inner_ring_range.x, inner_ring_range.y * upgrade_radius_multiplier, progress))
	shader_material.set_shader_parameter("outer_ring", lerpf(outer_ring_range.x, outer_ring_range.y * upgrade_radius_multiplier, progress))

	world.set_component(singleton_component_name, {
		"radius": effective_radius * hit_radius_adjustment
	})

	if active_radius >= max_radius:
		_reset_shockwave()


func _fire() -> void:
	is_firing = true
	active_radius = 0.0
	shader_material.set_shader_parameter("radius", 0.0)
	shader_material.set_shader_parameter("inner_ring", inner_ring_range.x)
	shader_material.set_shader_parameter("outer_ring", outer_ring_range.x)
	
	if not has_played_sound: AudioManager.shockwave.play()
	has_played_sound = true


func _reset_shockwave() -> void:
	active_radius = 0.0
	is_firing = false
	vfx.visible = false
	world.set_component(singleton_component_name, {
		"radius": 0.0
	})
	has_played_sound = false
