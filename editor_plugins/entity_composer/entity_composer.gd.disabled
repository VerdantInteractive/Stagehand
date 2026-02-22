@tool
extends GraphEdit

var _schema: ComponentSchema
var _node_counter: int = 0
var _rename_edit: LineEdit
var _editing_node: GraphNode

var _copy_buffer: Array = []
var _node_popup: PopupMenu
var _graph_popup: PopupMenu
var _connection_popup: PopupMenu
var _right_click_pos: Vector2
var _undo_redo: EditorUndoRedoManager

const NOMINAL_NODE_SIZE = Vector2(200, 150)
const TITLE_LENGTH_LIMIT = 25

func _ready() -> void:
	if ClassDB.class_exists("ComponentSchema"):
		_schema = ComponentSchema.new()
	else:
		printerr("ComponentSchema class not found. Make sure the GDExtension is loaded.")
	_undo_redo = EditorInterface.get_editor_undo_redo()
	
	# Setup GraphEdit
	right_disconnects = true # enables disconnection of existing connections in the GraphEdit by dragging the right end.
	connection_request.connect(_on_connection_request)
	disconnection_request.connect(_on_disconnection_request)
	delete_nodes_request.connect(_on_delete_nodes_request)
	connection_to_empty.connect(_on_connection_to_empty)
	connection_from_empty.connect(_on_connection_from_empty)
	gui_input.connect(_on_gui_input)
	scroll_offset_changed.connect(func(_off): _finish_renaming(_rename_edit.text))
	
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
	
	# Rename Edit
	_rename_edit = LineEdit.new()
	_rename_edit.visible = false
	_rename_edit.max_length = TITLE_LENGTH_LIMIT
	_rename_edit.alignment = HORIZONTAL_ALIGNMENT_CENTER
	_rename_edit.top_level = true
	_rename_edit.focus_exited.connect(_on_rename_focus_exited)
	_rename_edit.text_submitted.connect(_on_rename_text_submitted)
	_rename_edit.gui_input.connect(_on_rename_edit_gui_input)
	add_child(_rename_edit)
	
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
	call_deferred("_start_renaming", node)

func _create_base_node(title_text: String, position: Vector2) -> GraphNode:
	var unique_title = title_text
	while true:
		var collision = false
		for child in get_children():
			if child is GraphNode and child.title == unique_title:
				collision = true
				break
		if not collision:
			break
		unique_title += "_"

	var final_pos = _get_non_overlapping_position(position)
	var node = GraphNode.new()
	node.title = unique_title
	node.name = "PrefabNode_" + str(_node_counter)
	node.position_offset = final_pos
	node.resizable = true
	
	_node_counter += 1
	
	# Row 0: Ports and Labels
	var header_hbox = HBoxContainer.new()
	var label_in = Label.new()
	label_in.text = "Parent"
	var label_out = Label.new()
	label_out.text = "Child"
	label_out.size_flags_horizontal = Control.SIZE_EXPAND | Control.SIZE_SHRINK_END
	label_out.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	
	header_hbox.add_child(label_in)
	header_hbox.add_child(label_out)
	node.add_child(header_hbox)

	# Add a rename and close button to the GraphNode title bar using get_titlebar_hbox()
	var title_hbox: HBoxContainer = node.get_titlebar_hbox()

	var rename_btn = Button.new()
	rename_btn.text = "R"
	rename_btn.flat = true
	rename_btn.pressed.connect(func(): _start_renaming(node))
	title_hbox.add_child(rename_btn)
	# Move the rename button to the left edge of the title bar
	title_hbox.move_child(rename_btn, 0)

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
	var node = _create_base_node(data.get("title", "Pasted Node"), position)
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
	if not _schema:
		printerr("ComponentSchema not initialized.")
		return

	var component_list = _schema.get_registered_components()
	var popup = PopupMenu.new()
	popup.name = "ComponentSelector"
	
	var flat_list = []
	var idx = 0
	
	for ns in component_list:
		if not ns.is_empty():
			popup.add_separator(ns)
		else:
			popup.add_separator("Global")
			
		var comps = component_list[ns]
		if comps is PackedStringArray or comps is Array:
			for comp_name in comps:
				popup.add_item(comp_name, idx)
				flat_list.append(comp_name)
				idx += 1
	
	popup.id_pressed.connect(func(id):
		if id < flat_list.size():
			_add_component_to_node(graph_node, flat_list[id])
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
			call_deferred("_start_renaming", node)
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
				components.append(comp_child.name)
		
		var node_data = {
			"title": node.title,
			"relative_pos": node.position_offset - top_left_pos,
			"components": components,
			"size": node.size,
		}
		_copy_buffer.append(node_data)
	return top_left_pos

func _add_component_to_node(graph_node: GraphNode, component_name: String) -> void:
	var container = graph_node.get_node("Components")
	
	# Check if already exists
	if container.has_node(component_name):
		return
		
	var hbox = HBoxContainer.new()
	hbox.name = component_name
	
	var label = Label.new()
	label.text = component_name
	label.size_flags_horizontal = Control.SIZE_EXPAND
	hbox.add_child(label)
	
	# Get default value
	var default_val = _schema.get_component_default(component_name)
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
	
	call_deferred("_start_renaming", new_node)

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
	
	call_deferred("_start_renaming", new_node)

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


func _start_renaming(node: GraphNode) -> void:
	_editing_node = node
	_rename_edit.text = node.title
	
	_rename_edit.global_position = node.global_position
	_rename_edit.size = Vector2(node.size.x, 32)
	_rename_edit.scale = Vector2(zoom, zoom)
	
	_rename_edit.visible = true
	_rename_edit.grab_focus()
	_rename_edit.select_all()

func _on_rename_text_submitted(new_text: String) -> void:
	_finish_renaming(new_text)

func _on_rename_focus_exited() -> void:
	_finish_renaming(_rename_edit.text)

func _on_rename_edit_gui_input(event: InputEvent) -> void:
	if event is InputEventKey and event.pressed and event.keycode == KEY_ESCAPE:
		_editing_node = null
		_rename_edit.visible = false

func _finish_renaming(text: String) -> void:
	if _editing_node:
		if _editing_node.title != text:
			_undo_redo.create_action("Rename Node", UndoRedo.MERGE_DISABLE, self )
			_undo_redo.add_do_property(_editing_node, "title", text)
			_undo_redo.add_do_property(_editing_node, "size", Vector2.ZERO)
			_undo_redo.add_undo_property(_editing_node, "title", _editing_node.title)
			_undo_redo.add_undo_property(_editing_node, "size", _editing_node.size)
			_undo_redo.commit_action()
		else:
			_editing_node.size = Vector2.ZERO
	_rename_edit.visible = false
	_editing_node = null

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
