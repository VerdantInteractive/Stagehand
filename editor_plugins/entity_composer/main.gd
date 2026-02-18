@tool
extends EditorPlugin


func _enable_plugin() -> void:
	# Add autoloads here.
	print("EntityComposer EditorPlugin enabled.")


func _disable_plugin() -> void:
	# Remove autoloads here.
	print("EntityComposer EditorPlugin disabled.")


func _enter_tree() -> void:
	# Initialization of the plugin goes here.
	print("EntityComposer EditorPlugin entered the tree.")


func _exit_tree() -> void:
	# Clean-up of the plugin goes here.
	print("EntityComposer EditorPlugin exited the tree.")
