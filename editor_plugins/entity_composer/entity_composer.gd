@tool
extends GraphEdit

# TODO: Custom icon https://docs.godotengine.org/en/stable/classes/class_editorplugin.html#class-editorplugin-private-method-get-plugin-icon

var _node_counter: int = 0

var _copy_buffer: Array = []
var _node_popup: PopupMenu
var _graph_popup: PopupMenu
var _connection_popup: PopupMenu
var _right_click_pos: Vector2
var _undo_redo: EditorUndoRedoManager

const NOMINAL_NODE_SIZE = Vector2(200, 150)
const TITLE_LENGTH_LIMIT = 25

func _ready() -> void:
	_undo_redo = EditorInterface.get_editor_undo_redo()
	
	# Setup GraphEdit
	right_disconnects = true # enables disconnection of existing connections in the GraphEdit by dragging the right end.
	connection_request.connect(_on_connection_request)
	disconnection_request.connect(_on_disconnection_request)
	delete_nodes_request.connect(_on_delete_nodes_request)
	connection_to_empty.connect(_on_connection_to_empty)
	connection_from_empty.connect(_on_connection_from_empty)
	gui_input.connect(_on_gui_input)
	
	var toolbar = get_menu_hbox()
	
	var file_menu = MenuButton.new()
	file_menu.text = "File"
	toolbar.add_child(file_menu)
	
	var edit_menu = MenuButton.new()
	edit_menu.text = "Edit"
	var edit_popup = edit_menu.get_popup()
	edit_popup.add_item("Cut", 0)
	edit_popup.add_item("Copy", 1)
	edit_popup.add_item("Paste", 2)
	edit_popup.add_separator()
	edit_popup.add_item("Duplicate", 3)
	edit_popup.add_item("Delete", 4)
	edit_popup.add_separator()
	edit_popup.add_item("Clear Copy Buffer", 5)
	edit_popup.id_pressed.connect(_on_edit_menu_id_pressed)
	edit_menu.about_to_popup.connect(func():
		var has_selection = not _get_selected_nodes().is_empty()
		var has_buffer = not _copy_buffer.is_empty()
		edit_popup.set_item_disabled(edit_popup.get_item_index(0), not has_selection)
		edit_popup.set_item_disabled(edit_popup.get_item_index(1), not has_selection)
		edit_popup.set_item_disabled(edit_popup.get_item_index(2), not has_buffer)
		edit_popup.set_item_disabled(edit_popup.get_item_index(3), not has_selection)
		edit_popup.set_item_disabled(edit_popup.get_item_index(4), not has_selection)
		edit_popup.set_item_disabled(edit_popup.get_item_index(5), not has_buffer)
	)
	toolbar.add_child(edit_menu)
	
	var add_node_btn = Button.new()
	add_node_btn.text = "Add Node"
	add_node_btn.pressed.connect(_on_add_node_btn_pressed)
	toolbar.add_child(add_node_btn)
	
	
	# Context Menus
	_node_popup = PopupMenu.new()
	_node_popup.name = "NodePopup"
	_node_popup.add_item("Cut", 0)
	_node_popup.add_item("Copy", 1)
	_node_popup.add_item("Paste", 2)
	_node_popup.add_separator()
	_node_popup.add_item("Delete", 3)
	_node_popup.add_item("Duplicate", 4)
	_node_popup.add_separator()
	_node_popup.add_item("Clear Copy Buffer", 5)
	_node_popup.id_pressed.connect(_on_node_popup_id_pressed)
	add_child(_node_popup)

	_graph_popup = PopupMenu.new()
	_graph_popup.name = "GraphPopup"
	_graph_popup.add_item("Add Node", 0)
	_graph_popup.add_item("Paste", 1)
	_graph_popup.add_separator()
	_graph_popup.add_item("Clear Copy Buffer", 2)
	_graph_popup.id_pressed.connect(_on_graph_popup_id_pressed)
	add_child(_graph_popup)

	_connection_popup = PopupMenu.new()
	_connection_popup.name = "ConnectionPopup"
	_connection_popup.add_item("Disconnect", 0)
	_connection_popup.id_pressed.connect(_on_connection_popup_id_pressed)
	add_child(_connection_popup)

func _on_gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_RIGHT and event.is_pressed():
		var local_mouse_pos = get_local_mouse_position()
		_right_click_pos = local_mouse_pos
		var screen_pos = Vector2i(get_screen_position() + local_mouse_pos)

		# Check for connection click
		var connection = _get_connection_at_position(local_mouse_pos)
		if not connection.is_empty():
			_connection_popup.set_meta("connection", connection)
			_connection_popup.popup(Rect2i(screen_pos, Vector2i.ZERO))
			return

		# Check for node click
		var clicked_node = _get_node_at_position(local_mouse_pos)
		if clicked_node:
			if not clicked_node.selected:
				_clear_selection()
				clicked_node.selected = true
			
			_node_popup.set_item_disabled(_node_popup.get_item_index(2), _copy_buffer.is_empty())
			_node_popup.popup(Rect2i(screen_pos, Vector2i.ZERO))
		else:
			# Empty area click
			_graph_popup.set_item_disabled(_graph_popup.get_item_index(1), _copy_buffer.is_empty())
			_graph_popup.popup(Rect2i(screen_pos, Vector2i.ZERO))
	
	if event is InputEventKey and event.is_pressed() and event.is_command_or_control_pressed():
		match event.keycode:
			KEY_X:
				var selected = _get_selected_nodes()
				if not selected.is_empty():
					_copy_nodes(selected)
					for node in selected:
						_delete_node(node)
					accept_event()
			KEY_C:
				var selected = _get_selected_nodes()
				if not selected.is_empty():
					_copy_nodes(selected)
					accept_event()
			KEY_V:
				var paste_pos = (scroll_offset + get_local_mouse_position()) / zoom
				_paste_nodes(paste_pos)
				accept_event()
			KEY_D:
				var selected = _get_selected_nodes()
				if not selected.is_empty():
					_duplicate_nodes(selected)
					accept_event()
			KEY_Z:
				if event.shift_pressed:
					_undo_redo.redo_history_action(_undo_redo.get_history_id_for_object(self ))
				else:
					_undo_redo.undo_history_action(_undo_redo.get_history_id_for_object(self ))
				accept_event()
			KEY_Y:
				_undo_redo.redo_history_action(_undo_redo.get_history_id_for_object(self ))
				accept_event()

func _on_add_node_btn_pressed() -> void:
	var spawn_pos = (scroll_offset + (size / 2)) / zoom
	spawn_pos -= NOMINAL_NODE_SIZE / 2
	var node = _add_prefab_node(spawn_pos)
	call_deferred("_start_editing_node_title", node)

func _create_base_node(title_text: String, position: Vector2) -> GraphNode:
	var unique_title = title_text
	while true:
		var collision = false
		for child in get_children():
			if child is GraphNode and child.get_meta("title", child.title) == unique_title:
				collision = true
				break
		if not collision:
			break
		unique_title += "_"

	var final_pos = _get_non_overlapping_position(position)
	var node = GraphNode.new()
	node.title = ""
	node.set_meta("title", unique_title)
	node.name = "PrefabNode_" + str(_node_counter)
	node.position_offset = final_pos
	node.resizable = true
	
	_node_counter += 1
	
	# Row 0: Ports and Labels
	var header_hbox = HBoxContainer.new()
	header_hbox.add_theme_constant_override("separation", 0)
	var label_in = Label.new()
	label_in.text = "Parent"
	label_in.add_theme_font_size_override("font_size", 16)
	
	var spacer = Control.new()
	spacer.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	spacer.mouse_filter = Control.MOUSE_FILTER_IGNORE
	
	var label_out = Label.new()
	label_out.text = "Child"
	label_out.add_theme_font_size_override("font_size", 16)
	
	header_hbox.add_child(label_in)
	header_hbox.add_child(spacer)
	header_hbox.add_child(label_out)
	node.add_child(header_hbox)

	# Add a rename and close button to the GraphNode title bar using get_titlebar_hbox()
	var title_hbox: HBoxContainer = node.get_titlebar_hbox()

	var title_label = _create_editable_label(unique_title, func(new_text):
		if node.get_meta("title") != new_text:
			_undo_redo.create_action("Rename Node", UndoRedo.MERGE_DISABLE, self )
			_undo_redo.add_do_method(self , "_set_node_title", node, new_text)
			_undo_redo.add_undo_method(self , "_set_node_title", node, node.get_meta("title"))
			_undo_redo.commit_action()
	)
	title_label.set_meta("is_title", true)
	title_hbox.add_child(title_label)
	title_hbox.move_child(title_label, 0)

	var _spacer = Control.new()
	_spacer.size_flags_horizontal = Control.SIZE_EXPAND
	title_hbox.add_child(_spacer)

	var close_btn = Button.new()
	close_btn.text = "X"
	close_btn.flat = true
	close_btn.pressed.connect(func(): _show_delete_confirmation(node, close_btn))
	title_hbox.add_child(close_btn)
	
	# Enable slots for Row 0
	# Left: Input (Parent), Right: Output (Child)
	node.set_slot(0, true, 0, Color.CORNFLOWER_BLUE, true, 0, Color.CORNFLOWER_BLUE)
	
	# Row 1: Component Container
	var components_vbox = VBoxContainer.new()
	components_vbox.name = "Components"
	node.add_child(components_vbox)
	# Disable slots for Row 1
	node.set_slot(1, false, 0, Color.WHITE, false, 0, Color.WHITE)
	
	# Row 2: Add Component Button
	var add_comp_btn = Button.new()
	add_comp_btn.text = "Add Component"
	add_comp_btn.pressed.connect(func(): _show_component_selector(node, add_comp_btn))
	node.add_child(add_comp_btn)
	# Disable slots for Row 2
	node.set_slot(2, false, 0, Color.WHITE, false, 0, Color.WHITE)
	
	return node

func _add_prefab_node(position: Vector2) -> GraphNode:
	var node = _create_base_node("Prefab" + str(_node_counter), position)
	
	_undo_redo.create_action("Add Node", UndoRedo.MERGE_DISABLE, self )
	_undo_redo.add_do_method(self , "add_child", node)
	_undo_redo.add_do_method(self , "set_selected", node)
	_undo_redo.add_undo_method(self , "remove_child", node)
	_undo_redo.add_do_reference(node)
	_undo_redo.commit_action()
	
	return node

func _create_node_from_data(data: Dictionary, position: Vector2) -> GraphNode:
	var node = _create_base_node(data.get("title", data.get("name", "Pasted Node")), position)
	node.size = data.get("size", Vector2.ZERO)
	
	# Add components from data
	var components = data.get("components", [])
	for comp_name in components:
		_add_component_to_node(node, comp_name)
	
	return node

func _get_non_overlapping_position(target_pos: Vector2) -> Vector2:
	var current_pos = target_pos
	var offset = Vector2(30, 30)
	var attempts = 0
	var overlap = true
	
	while overlap and attempts < 100:
		overlap = false
		var target_rect = Rect2(current_pos, NOMINAL_NODE_SIZE)
		for child in get_children():
			if child is GraphNode:
				var child_size = child.size
				if child_size == Vector2.ZERO: child_size = NOMINAL_NODE_SIZE
				if target_rect.intersects(Rect2(child.position_offset, child_size)):
					overlap = true
					current_pos += offset
					break
		attempts += 1
	return current_pos

func _show_delete_confirmation(node: GraphNode, button: Button) -> void:
	var popup = PopupMenu.new()
	popup.add_item("delete ?")
	popup.id_pressed.connect(func(_id): _delete_node(node))
	popup.popup_hide.connect(func(): popup.queue_free())
	
	add_child(popup)
	popup.position = Vector2i(button.get_screen_position()) + Vector2i(0, button.size.y)
	popup.popup()

func _add_delete_node_to_action(node: GraphNode) -> void:
	var connections = []
	for conn in get_connection_list():
		if conn.from_node == node.name or conn.to_node == node.name:
			connections.append(conn)
	
	_undo_redo.add_do_method(self , "_do_delete_node", node, connections)
	_undo_redo.add_undo_method(self , "_undo_delete_node", node, connections)
	_undo_redo.add_do_reference(node)

func _do_delete_node(node: GraphNode, connections: Array) -> void:
	for conn in connections:
		disconnect_node(conn.from_node, conn.from_port, conn.to_node, conn.to_port)
	remove_child(node)

func _undo_delete_node(node: GraphNode, connections: Array) -> void:
	add_child(node)
	for conn in connections:
		connect_node(conn.from_node, conn.from_port, conn.to_node, conn.to_port)

func _delete_node(node: GraphNode) -> void:
	_undo_redo.create_action("Delete Node", UndoRedo.MERGE_DISABLE, self )
	_add_delete_node_to_action(node)
	_undo_redo.commit_action()

func _show_component_selector(graph_node: GraphNode, button: Button = null) -> void:
	var component_list = {}
	for path in ECS.SCHEMA.components:
		var info = ECS.SCHEMA.components[path]
		if info.get("is_change_detection_tag", false):
			continue
		var ns = info.get("namespace", "") # 'namespace' is a reserved keywork in GDScript, so we use 'ns'
		if not component_list.has(ns):
			component_list[ns] = []
		component_list[ns].append(path)
	
	var popup = PopupMenu.new()
	popup.name = "ComponentSelector"
	
	var flat_list = []
	var idx = 0
	
	var namespaces = component_list.keys()
	namespaces.sort()
	
	for ns in namespaces:
		if not ns.is_empty():
			popup.add_separator(ns)
		else:
			popup.add_separator("Global")
			
		var comps = component_list[ns]
		comps.sort()
		if comps is PackedStringArray or comps is Array:
			for comp_name in comps:
				var display_name = comp_name
				if not ns.is_empty() and display_name.begins_with(ns):
					display_name = display_name.substr(ns.length() + 2)
				popup.add_item(display_name, idx)
				flat_list.append(comp_name)
				idx += 1
	
	popup.id_pressed.connect(func(id):
		if id < flat_list.size():
			_add_component_to_node(graph_node, flat_list[id], true)
		popup.queue_free()
	)
	
	# Cleanup popup if closed without selection
	popup.popup_hide.connect(func(): popup.queue_free())
	
	add_child(popup)
	if button:
		popup.position = Vector2i(button.get_screen_position()) + Vector2i(0, button.size.y)
	else:
		popup.position = Vector2(get_window().position) + get_screen_position() + get_local_mouse_position()
	popup.popup()

func _on_edit_menu_id_pressed(id: int) -> void:
	var selected_nodes = _get_selected_nodes()
	match id:
		0: # Cut
			if not selected_nodes.is_empty():
				_copy_nodes(selected_nodes)
				for node in selected_nodes:
					_delete_node(node)
		1: # Copy
			if not selected_nodes.is_empty():
				_copy_nodes(selected_nodes)
		2: # Paste
			var paste_pos = (scroll_offset + (size / 2)) / zoom
			_paste_nodes(paste_pos)
		3: # Duplicate
			if not selected_nodes.is_empty():
				_duplicate_nodes(selected_nodes)
		4: # Delete
			for node in selected_nodes:
				_delete_node(node)
		5: # Clear Copy Buffer
			_clear_copy_buffer()

func _on_node_popup_id_pressed(id: int) -> void:
	var selected_nodes = _get_selected_nodes()
	if selected_nodes.is_empty():
		return

	match id:
		0: # Cut
			_copy_nodes(selected_nodes)
			for node in selected_nodes:
				_delete_node(node)
		1: # Copy
			_copy_nodes(selected_nodes)
		2: # Paste
			_paste_nodes()
		3: # Delete
			for node in selected_nodes:
				_delete_node(node)
		4: # Duplicate
			_duplicate_nodes(selected_nodes)
		5: # Clear Copy Buffer
			_clear_copy_buffer()

func _on_graph_popup_id_pressed(id: int) -> void:
	match id:
		0: # Add Node
			var spawn_pos = (scroll_offset + _right_click_pos) / zoom
			var node = _add_prefab_node(spawn_pos)
			call_deferred("_start_editing_node_title", node)
		1: # Paste
			_paste_nodes()
		2: # Clear Copy Buffer
			_clear_copy_buffer()

func _on_connection_popup_id_pressed(id: int) -> void:
	match id:
		0: # Disconnect
			var conn = _connection_popup.get_meta("connection")
			if not conn.is_empty():
				_undo_redo.create_action("Disconnect", UndoRedo.MERGE_DISABLE, self )
				_undo_redo.add_do_method(self , "disconnect_node", conn.from_node, conn.from_port, conn.to_node, conn.to_port)
				_undo_redo.add_undo_method(self , "connect_node", conn.from_node, conn.from_port, conn.to_node, conn.to_port)
				_undo_redo.commit_action()

func _clear_copy_buffer() -> void:
	_copy_buffer.clear()

func _copy_nodes(nodes: Array[GraphNode]) -> Vector2:
	_clear_copy_buffer()
	
	var top_left_pos = Vector2(INF, INF)
	for node in nodes:
		if node.position_offset.x < top_left_pos.x: top_left_pos.x = node.position_offset.x
		if node.position_offset.y < top_left_pos.y: top_left_pos.y = node.position_offset.y

	for node in nodes:
		var components_container = node.get_node_or_null("Components")
		var components = []
		if components_container:
			for comp_child in components_container.get_children():
				if comp_child.has_meta("component_name"):
					components.append(comp_child.get_meta("component_name"))
				else:
					components.append(comp_child.name)
		
		var node_data = {
			"title": node.get_meta("title", node.title),
			"relative_pos": node.position_offset - top_left_pos,
			"components": components,
			"size": node.size,
		}
		_copy_buffer.append(node_data)
	return top_left_pos

func _add_component_to_node(graph_node: GraphNode, component_name: String, focus_edit: bool = false) -> void:
	var container = graph_node.get_node("Components")
	
	var node_name = component_name.replace(":", "_").replace("/", "_")
	
	# Check if already exists
	if container.has_node(node_name):
		return
		
	var hbox = HBoxContainer.new()
	hbox.name = node_name
	hbox.set_meta("component_name", component_name)
	hbox.tooltip_text = component_name
	
	var display_name = component_name
	if ECS.SCHEMA.components.has(component_name):
		var info = ECS.SCHEMA.components[component_name]
		hbox.tooltip_text = info.get("data_type", component_name)
		var ns = info.get("namespace", "")
		if not ns.is_empty():
			display_name = display_name.substr(ns.length() + 2)

		var type_name = info.get("type", "")
		if type_name.is_empty():
			type_name = display_name

		if type_name and EditorInterface.get_editor_theme().has_icon(type_name, "EditorIcons"):
			var icon = EditorInterface.get_editor_theme().get_icon(type_name, "EditorIcons")
			var icon_rect = TextureRect.new()
			icon_rect.texture = icon
			icon_rect.stretch_mode = TextureRect.STRETCH_KEEP_CENTERED
			hbox.add_child(icon_rect)

	var name_control: Control
	var on_name_change = func(new_text):
		if hbox.get_meta("component_name") != new_text:
			_undo_redo.create_action("Rename Component", UndoRedo.MERGE_DISABLE, self )
			_undo_redo.add_do_method(self , "_set_component_name", hbox, new_text)
			_undo_redo.add_undo_method(self , "_set_component_name", hbox, hbox.get_meta("component_name"))
			_undo_redo.commit_action()

	if focus_edit:
		name_control = _create_editable_edit(display_name, display_name, on_name_change, "main", true)
	else:
		name_control = _create_editable_label(display_name, on_name_change, "main")
	name_control.set_meta("is_component_name", true)
	hbox.add_child(name_control)
	
	# Get default value
	var default_val = "-" # Default value not available in ECS registry
	var val_label = Label.new()
	val_label.text = str(default_val)
	val_label.modulate = Color(0.7, 0.7, 0.7)
	val_label.clip_text = true
	hbox.add_child(val_label)
	
	var del_btn = Button.new()
	del_btn.text = "X"
	del_btn.flat = true
	del_btn.pressed.connect(func(): hbox.queue_free())
	hbox.add_child(del_btn)
	
	container.add_child(hbox)

	if focus_edit and name_control is LineEdit:
		name_control.grab_focus()
		name_control.select_all()

func _create_editable_edit(text: String, revert_text: String, on_change: Callable = Callable(), font_style: String = "bold", restrict_alphanumeric: bool = false) -> LineEdit:
	var name_edit = LineEdit.new()
	name_edit.text = text
	name_edit.max_length = TITLE_LENGTH_LIMIT
	name_edit.size_flags_horizontal = Control.SIZE_EXPAND
	name_edit.add_theme_font_override("font", EditorInterface.get_editor_theme().get_font(font_style, "EditorFonts"))
	
	if restrict_alphanumeric:
		var regex = RegEx.new()
		regex.compile("[^a-zA-Z0-9_]")
		name_edit.text_changed.connect(func(new_text):
			var filtered_text = regex.sub(new_text, "", true)
			if new_text != filtered_text:
				var caret = name_edit.caret_column
				name_edit.text = filtered_text
				name_edit.caret_column = min(caret, filtered_text.length())
		)

	var commit = func(new_text: String):
		_replace_edit_with_label.call_deferred(name_edit, new_text, on_change)
		if on_change.is_valid():
			on_change.call(new_text)

	name_edit.focus_exited.connect(func(): commit.call(name_edit.text))
	name_edit.text_submitted.connect(commit)
	name_edit.gui_input.connect(func(event):
		if event is InputEventKey and event.pressed and event.keycode == KEY_ESCAPE:
			_replace_edit_with_label.call_deferred(name_edit, revert_text, on_change)
	)
	return name_edit

func _create_editable_label(text: String, on_change: Callable = Callable(), font_style: String = "bold") -> Label:
	var label = Label.new()
	label.text = text
	label.size_flags_horizontal = Control.SIZE_EXPAND
	label.add_theme_font_override("font", EditorInterface.get_editor_theme().get_font(font_style, "EditorFonts"))
	label.mouse_filter = Control.MOUSE_FILTER_STOP
	label.gui_input.connect(func(event):
		if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
			_replace_label_with_edit.call_deferred(label, on_change)
	)
	return label

func _replace_label_with_edit(label: Label, on_change: Callable) -> void:
	if not is_instance_valid(label): return
	var parent = label.get_parent()
	if not parent: return
	
	var text = label.text
	var idx = label.get_index()
	
	var font_style = "bold"
	if label.has_meta("is_component_name"):
		font_style = "main"
	
	var name_edit = _create_editable_edit(text, text, on_change, font_style, label.has_meta("is_component_name") or label.has_meta("is_title"))
	# Copy metadata if any (like is_title or is_component_name)
	if label.has_meta("is_title"): name_edit.set_meta("is_title", true)
	if label.has_meta("is_component_name"): name_edit.set_meta("is_component_name", true)
	
	parent.remove_child(label)
	label.queue_free()
	
	parent.add_child(name_edit)
	parent.move_child(name_edit, idx)
	
	name_edit.grab_focus()
	name_edit.select_all()

func _replace_edit_with_label(edit: LineEdit, text: String, on_change: Callable) -> void:
	if not is_instance_valid(edit): return
	var parent = edit.get_parent()
	if not parent: return
	
	var font_style = "bold"
	if edit.has_meta("is_component_name"):
		font_style = "main"
	
	var label = _create_editable_label(text, on_change, font_style)
	# Copy metadata
	if edit.has_meta("is_title"): label.set_meta("is_title", true)
	if edit.has_meta("is_component_name"): label.set_meta("is_component_name", true)
	
	var idx = edit.get_index()
	parent.remove_child(edit)
	edit.queue_free()
	
	parent.add_child(label)
	parent.move_child(label, idx)

func _paste_nodes(position: Vector2 = Vector2(INF, INF)) -> void:
	if _copy_buffer.is_empty():
		return

	var paste_pos: Vector2
	if position.x != INF:
		paste_pos = position
	else:
		paste_pos = (scroll_offset + _right_click_pos) / zoom
	
	_undo_redo.create_action("Paste Nodes", UndoRedo.MERGE_DISABLE, self )
	
	var new_nodes = []
	for node_data in _copy_buffer:
		var new_node = _create_node_from_data(node_data, paste_pos + node_data.relative_pos)
		_undo_redo.add_do_method(self , "add_child", new_node)
		_undo_redo.add_do_reference(new_node)
		_undo_redo.add_undo_method(self , "remove_child", new_node)
		new_nodes.append(new_node)
	
	_undo_redo.add_do_method(self , "_select_nodes", new_nodes)
	_undo_redo.commit_action()

func _select_nodes(nodes: Array) -> void:
	_clear_selection()
	for node in nodes:
		node.selected = true
	grab_focus()

func _duplicate_nodes(nodes: Array[GraphNode]) -> void:
	var top_left = _copy_nodes(nodes)
	_paste_nodes(top_left + Vector2(20, 20))

func _on_connection_request(from_node: StringName, from_port: int, to_node: StringName, to_port: int) -> void:
	if from_node == to_node:
		return
	
	_undo_redo.create_action("Connect", UndoRedo.MERGE_DISABLE, self )
	_undo_redo.add_do_method(self , "connect_node", from_node, from_port, to_node, to_port)
	_undo_redo.add_undo_method(self , "disconnect_node", from_node, from_port, to_node, to_port)
	_undo_redo.commit_action()

func _on_connection_to_empty(from_node: StringName, from_port: int, release_position: Vector2) -> void:
	var spawn_pos = (release_position + scroll_offset) / zoom
	spawn_pos -= NOMINAL_NODE_SIZE / 2
	var new_node = _create_base_node("Prefab" + str(_node_counter), spawn_pos)
	
	_undo_redo.create_action("Add Node and Connect", UndoRedo.MERGE_DISABLE, self )
	_undo_redo.add_do_method(self , "add_child", new_node)
	_undo_redo.add_do_reference(new_node)
	_undo_redo.add_undo_method(self , "remove_child", new_node)
	_undo_redo.add_do_method(self , "connect_node", from_node, from_port, new_node.name, 0)
	_undo_redo.add_undo_method(self , "disconnect_node", from_node, from_port, new_node.name, 0)
	_undo_redo.commit_action()
	
	call_deferred("_start_editing_node_title", new_node)

func _on_connection_from_empty(to_node: StringName, to_port: int, release_position: Vector2) -> void:
	var spawn_pos = (release_position + scroll_offset) / zoom
	spawn_pos -= NOMINAL_NODE_SIZE / 2
	var new_node = _create_base_node("Prefab" + str(_node_counter), spawn_pos)
	
	_undo_redo.create_action("Add Node and Connect", UndoRedo.MERGE_DISABLE, self )
	_undo_redo.add_do_method(self , "add_child", new_node)
	_undo_redo.add_do_reference(new_node)
	_undo_redo.add_undo_method(self , "remove_child", new_node)
	_undo_redo.add_do_method(self , "connect_node", new_node.name, 0, to_node, to_port)
	_undo_redo.add_undo_method(self , "disconnect_node", new_node.name, 0, to_node, to_port)
	_undo_redo.commit_action()
	
	call_deferred("_start_editing_node_title", new_node)

func _on_disconnection_request(from_node: StringName, from_port: int, to_node: StringName, to_port: int) -> void:
	_undo_redo.create_action("Disconnect", UndoRedo.MERGE_DISABLE, self )
	_undo_redo.add_do_method(self , "disconnect_node", from_node, from_port, to_node, to_port)
	_undo_redo.add_undo_method(self , "connect_node", from_node, from_port, to_node, to_port)
	_undo_redo.commit_action()

func _on_delete_nodes_request(nodes: Array[StringName]) -> void:
	_undo_redo.create_action("Delete Nodes", UndoRedo.MERGE_DISABLE, self )
	for node_name in nodes:
		var node = get_node_or_null(str(node_name))
		if node:
			_add_delete_node_to_action(node)
	_undo_redo.commit_action()

func _start_editing_node_title(node: GraphNode) -> void:
	var hbox = node.get_titlebar_hbox()
	for child in hbox.get_children():
		if child is Label and child.has_meta("is_title"):
			# Simulate click to start editing
			_replace_label_with_edit(child, func(new_text):
				if node.get_meta("title") != new_text:
					_undo_redo.create_action("Rename Node", UndoRedo.MERGE_DISABLE, self )
					_undo_redo.add_do_method(self , "_set_node_title", node, new_text)
					_undo_redo.add_undo_method(self , "_set_node_title", node, node.get_meta("title"))
					_undo_redo.commit_action()
			)
			break

func _set_node_title(node: GraphNode, new_title: String) -> void:
	node.set_meta("title", new_title)
	var hbox = node.get_titlebar_hbox()
	for child in hbox.get_children():
		if (child is Label or child is LineEdit) and child.has_meta("is_title"):
			child.text = new_title
			break

func _set_component_name(hbox: HBoxContainer, new_name: String) -> void:
	hbox.set_meta("component_name", new_name)
	hbox.tooltip_text = new_name
	if ECS.SCHEMA.components.has(new_name):
		hbox.tooltip_text = ECS.SCHEMA.components[new_name].get("data_type", new_name)
	for child in hbox.get_children():
		if (child is Label or child is LineEdit) and child.has_meta("is_component_name"):
			child.text = new_name
			break

func _get_selected_nodes() -> Array[GraphNode]:
	var selected: Array[GraphNode] = []
	for child in get_children():
		if child is GraphNode and child.selected:
			selected.append(child)
	return selected

func _clear_selection() -> void:
	for child in get_children():
		if child is GraphNode:
			child.selected = false

func _get_node_at_position(pos: Vector2) -> GraphNode:
	var children = get_children()
	# Iterate in reverse to find the top-most node
	for i in range(children.size() - 1, -1, -1):
		var child = children[i]
		if child is GraphNode:
			if child.get_rect().has_point(pos):
				return child
	return null

func _get_connection_at_position(pos: Vector2) -> Dictionary:
	var connection_list = get_connection_list()
	for conn in connection_list:
		var from_node = get_node(str(conn.from_node))
		var to_node = get_node(str(conn.to_node))
		if not from_node or not to_node:
			continue
		
		var start = from_node.position + from_node.get_output_port_position(conn.from_port)
		var end = to_node.position + to_node.get_input_port_position(conn.to_port)
		
		# Approximate bezier curve hit test
		var dist = start.distance_to(end)
		var cp_offset = dist * 0.5
		var cp1 = start + Vector2(cp_offset, 0)
		var cp2 = end - Vector2(cp_offset, 0)
		
		var samples = 20
		for i in range(samples + 1):
			var t = i / float(samples)
			var p = start.bezier_interpolate(cp1, cp2, end, t)
			if p.distance_to(pos) < 10.0: # Hit threshold
				return conn
	return {}
