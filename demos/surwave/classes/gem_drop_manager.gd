class_name GemDropManager extends Node

@export var gem_scene: PackedScene
## How much chance (from 0.0 to 1.0) does each prefab have to drop a gem
@export var drop_probabilities: Dictionary[String, float]

@onready var world: FlecsWorld = $".."

func _ready() -> void:
	world.stagehand_signal_emitted.connect(_on_flecs_signal)


func _on_flecs_signal(signal_name: StringName, data: Dictionary) -> void:
	if signal_name == "enemy_died":
		var enemy_type := String(data.get("enemy_type", ""))
		var drop_chance := float(drop_probabilities.get(enemy_type, 0.0))
		if randf() < drop_chance:
			if gem_scene == null:
				return
			var enemy_position := data.get("enemy_position", null)
			if not enemy_position is Vector2:
				return
			var gem = gem_scene.instantiate() as Node2D
			gem.global_position = enemy_position
			add_child(gem)
