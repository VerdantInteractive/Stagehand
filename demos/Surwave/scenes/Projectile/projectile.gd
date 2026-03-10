class_name Projectile extends Node2D

@export_category("General")
@export var damage: float = 1.0;
@export var range_squared: float = 100_000;
@export var random_direction_offset: float = 0.0; # In degrees. A random angle between -this and +this will be added to the initial direction.
@export var spawn_position_offset: Vector2 = Vector2(8.2, 7.4) # To ensure the projectile is spawned closer to the weapon barrel

@export_category("Movement")
@export var speed: float = 4.0;
@export var oscillation_frequency: float = 44.0; # how fast the projectile oscillates
@export var oscillation_amplitude_increment: float = 5.2; # rate at which the oscillation widens over time

var direction: Vector2
var age: float = 0.0
var forward_offset: Vector2
var right: Vector2
var spawn_position: Vector2


func _ready() -> void:
	forward_offset = direction * speed;
	right = Vector2(-direction.y, direction.x)

	if random_direction_offset > 0.0:
		var half_range: float = random_direction_offset * 0.5;
		var random_degrees: float = randf_range(-half_range, half_range);
		var random_radians: float = deg_to_rad(random_degrees);
		direction = direction.rotated(random_radians);

	global_position += direction * spawn_position_offset
	spawn_position = global_position;

func _process(_delta: float) -> void:
	var amplitude: float = oscillation_amplitude_increment * age
	var sideways_offset: Vector2 = right * sin(age * oscillation_frequency) * amplitude
	global_position += forward_offset + sideways_offset
