extends GraphEdit


func _ready() -> void:
	var schema = FlecsSchema.new()
	var components_in_schema = schema.get_registered_components()
	print("Components in schema:", components_in_schema)
