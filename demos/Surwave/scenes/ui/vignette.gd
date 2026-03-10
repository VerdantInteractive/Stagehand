extends CanvasLayer

var player: Node

func _ready() -> void:
	call_deferred("connect_player_damage_signal")

func connect_player_damage_signal() -> void:
	var players = get_tree().get_nodes_in_group("Players")
	if players.is_empty():
		push_warning("vignette.gd: No player found in 'Players' group")
		return
	player = players[0]
	player.owner.player_took_damage.connect(_on_player_took_damage)

func _on_player_took_damage() -> void:
	$AnimationPlayer.play("hit")
