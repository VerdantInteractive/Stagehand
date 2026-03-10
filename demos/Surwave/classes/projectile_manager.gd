class_name ProjectileManager extends Node

@export var projectile_global_node_group_name: String = "Projectiles"
@export var singleton_component_name: String = ECS.components.stagehand_demos.surwave.ProjectileData

var projectile_positions: Array[Vector2] = []

@onready var world: FlecsWorld = get_node("../../World")
@onready var upgrade_manager: UpgradeManager = $"../UpgradeManager"

func _ready() -> void:
	if world == null:
		push_warning("ProjectileManager: World node not found.")
		return


func _process(delta: float) -> void:
	projectile_positions.clear()

	for projectile_node in get_tree().get_nodes_in_group(projectile_global_node_group_name):
		if not projectile_node is Node2D: continue

		projectile_positions.append(projectile_node.global_position)

		projectile_node.age += delta

		if (projectile_node.spawn_position - projectile_node.global_position).length_squared() > projectile_node.range_squared:
			projectile_node.queue_free()

	world.set_component(singleton_component_name, {
		"projectile_positions": projectile_positions
	})
