class_name UpgradeCard extends PanelContainer

signal upgrade_selected(upgradeable: UpgradeManager.Upgradeable)

@onready var name_label: Label = %NameLabel
@onready var description_label: Label = %DescriptionLabel
@onready var cost_label: Label = %CostLabel
@onready var select_button: Button = %SelectButton

var upgradeable_type: UpgradeManager.Upgradeable = UpgradeManager.Upgradeable.PROJECTILE_DAMAGE
var can_afford_upgrade: bool = false
const AFFORDABLE_COLOR: Color = Color(1, 1, 1, 1)
const UNAFFORDABLE_COLOR: Color = Color(0.5, 0.5, 0.5, 1)


func _ready() -> void:
	select_button.pressed.connect(_on_select_button_pressed)
	mouse_entered.connect(_on_mouse_entered)

	# To have the hover animation play also when focus enters, e.g. when using a controller
	select_button.focus_entered.connect(_on_mouse_entered)


func animate_in(delay: float = 0.0) -> void:
	modulate = Color.TRANSPARENT
	await get_tree().create_timer(delay).timeout
	$AnimationPlayer.play("in")


func animate_discarded(delay: float = 0.0) -> void:
	$AnimationPlayer.play("discarded")


func set_upgrade_info(upgradeable: UpgradeManager.Upgradeable, upgrade_data: Dictionary):
	upgradeable_type = upgradeable
	var tier_name: String = String(upgrade_data.get("name", "Unknown Upgrade"))
	var description: String = String(upgrade_data.get("description", ""))
	var cost: int = int(upgrade_data.get("cost", 0))
	can_afford_upgrade = bool(upgrade_data.get("can_afford", false))
	name_label.text = tier_name
	description_label.text = description
	cost_label.text = str(cost)
	cost_label.modulate = Color.WHITE if can_afford_upgrade else Color.DARK_GRAY
	select_button.disabled = not can_afford_upgrade
	modulate = AFFORDABLE_COLOR if can_afford_upgrade else UNAFFORDABLE_COLOR


func set_selection_enabled(enabled: bool) -> void:
	select_button.disabled = not enabled


func _on_select_button_pressed() -> void:
	if not can_afford_upgrade: return
	
	$AnimationPlayer.play("selected")
	
	for other_card in get_tree().get_nodes_in_group("UpgradeCards"):
		if other_card == self: continue
		other_card.animate_discarded()
	
	await $AnimationPlayer.animation_finished
	emit_signal("upgrade_selected", upgradeable_type)

func _on_mouse_entered() -> void:
	$HoverAnimationPlayer.play("hover")
