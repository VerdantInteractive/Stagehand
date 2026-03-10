class_name UpgradeManager extends Node

signal upgrade_purchased(cost: int)

@onready var world: FlecsWorld = get_node("../../World")

enum Upgradeable {
	PROJECTILE_DAMAGE,
	PROJECTILE_COUNT,
	SHOCKWAVE,
	SPEED
}

const UPGRADE_INFO: Dictionary[Upgradeable, Array] = {
	Upgradeable.PROJECTILE_DAMAGE: [
		"Increase the damage of your gun",
		[
			["Base", 1.0],
			["Damage I", 2.0],
			["Damage II", 3.0],
		]
	],
	Upgradeable.PROJECTILE_COUNT: [
		"Fire multiple projectiles each time you fire your gun",
		[
			["Base", 1],
			["Multi-shot I", 2],
			["Multi-shot II", 3],
		]
	],
	Upgradeable.SHOCKWAVE: [
		"Increase the area of effect of your shockwave",
		[
			["Base", 0.40],
			["Shockwave I", 0.70],
			["Shockwave II", 1.0]
		]
	],
	Upgradeable.SPEED: [
		"Increase your movement speed",
		[
			["Base", 100],
			["Speed I", 200],
			["Speed II", 300],
		]
	]
}

const TIER_COSTS = [0, 20, 40]

# Holds the current upgrade tier that the player has for each upgradeable type
var upgrade_tiers: Dictionary[Upgradeable, int] = {
	Upgradeable.PROJECTILE_DAMAGE: 0,
	Upgradeable.PROJECTILE_COUNT: 0,
	Upgradeable.SHOCKWAVE: 0,
	Upgradeable.SPEED: 0
}


func get_upgrade_value(upgradeable: Upgradeable) -> Variant:
	return UPGRADE_INFO[upgradeable][1][upgrade_tiers[upgradeable]][1]


func get_available_upgrades() -> Dictionary[Upgradeable, Dictionary]:
	var available_upgrades: Dictionary[Upgradeable, Dictionary] = {}
	var stage = get_tree().get_first_node_in_group("stage")
	var current_gems = stage.gem_balance

	for upgradeable in upgrade_tiers.keys():
		var tiers: Array = UPGRADE_INFO[upgradeable][1]
		var next_tier_index: int = upgrade_tiers[upgradeable] + 1
		if next_tier_index >= tiers.size(): continue

		var tier_data: Array = tiers[next_tier_index]
		var cost: int = TIER_COSTS[next_tier_index]
		available_upgrades[upgradeable] = {
			"name": tier_data[0],
			"description": UPGRADE_INFO[upgradeable][0],
			"cost": cost,
			"can_afford": current_gems >= cost
		}

	return available_upgrades


func upgrade(upgradeable: Upgradeable) -> void:
	if not UPGRADE_INFO.has(upgradeable):
		return

	var tiers: Array = UPGRADE_INFO[upgradeable][1]
	var current_tier: int = upgrade_tiers[upgradeable]
	if current_tier >= tiers.size() - 1: return

	var next_tier_index: int = current_tier + 1
	var cost: int = TIER_COSTS[next_tier_index]

	upgrade_tiers[upgradeable] = next_tier_index
	upgrade_purchased.emit(cost)
	AudioManager.upgrade_acquired.play()

	match upgradeable:
		Upgradeable.PROJECTILE_DAMAGE:
			_update_enemy_take_damage_settings(float(tiers[next_tier_index][1]))
			AudioManager.projectile.volume_db += 2.0
			AudioManager.projectile.pitch_scale /= 1.2

		Upgradeable.SHOCKWAVE:
			AudioManager.shockwave.volume_db += 3.0
			AudioManager.shockwave.pitch_scale /= 1.2

		_:
			pass


func _update_enemy_take_damage_settings(new_damage: float) -> void:
	if world == null:
		push_warning("UpgradeManager: Cannot set EnemyTakeDamageSettings without a FlecsWorld reference.")
		return

	world.set_component(ECS.components.stagehand_demos.surwave.EnemyTakeDamageSettings, {
		"projectile_damage": new_damage
	})
