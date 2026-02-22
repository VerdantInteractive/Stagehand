@tool
extends EditorPlugin

const ENTITY_COMPOSER_SCENE = preload("res://addons/stagehand/editor_plugins/entity_composer/entity_composer.tscn")

var _entity_composer_instance

func _enable_plugin() -> void:
	print("EntityComposer EditorPlugin enabled.")

func _disable_plugin() -> void:
	print("EntityComposer EditorPlugin disabled.")

func _enter_tree() -> void:
	_entity_composer_instance = ENTITY_COMPOSER_SCENE.instantiate()
	EditorInterface.get_editor_main_screen().add_child(_entity_composer_instance)
	_make_visible(false) # hide the main panel so it doesn't compete for space when first activating the plugin.

func _exit_tree() -> void:
	if _entity_composer_instance:
		_entity_composer_instance.queue_free()

func _has_main_screen() -> bool:
	return true

func _make_visible(visible: bool) -> void:
	if _entity_composer_instance:
		_entity_composer_instance.visible = visible

func _get_plugin_name() -> String:
	return "Entity Composer"

func _get_plugin_icon() -> Texture2D:
	return EditorInterface.get_base_control().get_theme_icon("GraphEdit", "EditorIcons")
