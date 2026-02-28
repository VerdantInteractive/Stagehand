extends InstancedRenderer3D

@onready var world: FlecsWorld = $".."

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	world.instantiate_prefab("instanced_renderer::TestInstance", {"Position3D": Vector3(-2, 0, 0)})
	world.instantiate_prefab("instanced_renderer::TestInstance", {"Position3D": Vector3(0, 0, 0)})
	world.instantiate_prefab("instanced_renderer::TestInstance", {"Position3D": Vector3(2, 0, 0)})


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
