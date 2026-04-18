extends Node3D

const PREFAB_XPBD_CLOTH_3D := "stagehand::XPBDCloth3D"
const FIXED_DT := 1.0 / 60.0
const PICK_DISTANCE_THRESHOLD_SQ := 0.0016

@onready var world: FlecsWorld = $World
@onready var world_environment: WorldEnvironment = $WorldEnvironment
@onready var sun_light: DirectionalLight3D = $DirectionalLight3D
@onready var camera: Camera3D = $Camera3D

var cloth_entity_id: int = 0
var running := true
var show_edges := false

var cloth_mesh_instance: MeshInstance3D
var edge_mesh_instance: MeshInstance3D

var run_button: Button
var show_edges_checkbox: CheckBox
var collision_checkbox: CheckBox
var bending_slider: HSlider
var tris_label: Label
var verts_label: Label
var ms_label: Label

var timing_sum_ms := 0.0
var timing_frames := 0

var orbit_target := Vector3(0.0, 0.1, 0.0)
var orbit_yaw := 0.0
var orbit_pitch := 0.35
var orbit_distance := 0.55
var orbit_dragging := false

var drag_particle_id := -1
var drag_distance := 0.0
var previous_drag_position := Vector3.ZERO
var drag_velocity := Vector3.ZERO


func _ready() -> void:
	_setup_world()
	_setup_environment()
	_setup_render_nodes()
	_setup_ui()
	_spawn_cloth()
	_update_camera_transform()


func _setup_world() -> void:
	world.set_progress_tick(FlecsWorld.PROGRESS_TICK_MANUAL)


func _setup_environment() -> void:
	if world_environment.environment == null:
		push_warning("WorldEnvironment is missing an Environment resource.")

	sun_light.light_energy = 1.5
	camera.current = true
	camera.near = 0.01
	camera.far = 100.0


func _setup_render_nodes() -> void:
	cloth_mesh_instance = MeshInstance3D.new()
	var cloth_material := StandardMaterial3D.new()
	cloth_material.albedo_color = Color(1.0, 0.25, 0.2)
	cloth_material.roughness = 0.75
	cloth_material.cull_mode = BaseMaterial3D.CULL_DISABLED
	cloth_mesh_instance.material_override = cloth_material
	add_child(cloth_mesh_instance)

	edge_mesh_instance = MeshInstance3D.new()
	edge_mesh_instance.visible = false
	var edge_material := StandardMaterial3D.new()
	edge_material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	edge_material.albedo_color = Color(1.0, 0.1, 0.1)
	edge_mesh_instance.material_override = edge_material
	add_child(edge_mesh_instance)


func _setup_ui() -> void:
	var canvas := CanvasLayer.new()
	add_child(canvas)

	var panel := PanelContainer.new()
	panel.position = Vector2(16.0, 16.0)
	panel.size = Vector2(360.0, 230.0)
	canvas.add_child(panel)

	var vbox := VBoxContainer.new()
	panel.add_child(vbox)

	var title := Label.new()
	title.text = "Cloth Self Collision (XPBD3D)"
	vbox.add_child(title)

	var button_row := HBoxContainer.new()
	vbox.add_child(button_row)

	run_button = Button.new()
	run_button.text = "Stop"
	run_button.pressed.connect(_on_run_pressed)
	button_row.add_child(run_button)

	var restart_button := Button.new()
	restart_button.text = "Restart"
	restart_button.pressed.connect(_on_restart_pressed)
	button_row.add_child(restart_button)

	show_edges_checkbox = CheckBox.new()
	show_edges_checkbox.text = "Show edges"
	show_edges_checkbox.toggled.connect(_on_show_edges_toggled)
	vbox.add_child(show_edges_checkbox)

	collision_checkbox = CheckBox.new()
	collision_checkbox.text = "Handle collisions"
	collision_checkbox.button_pressed = true
	collision_checkbox.toggled.connect(_on_collision_toggled)
	vbox.add_child(collision_checkbox)

	var slider_label := Label.new()
	slider_label.text = "Bending compliance"
	vbox.add_child(slider_label)

	bending_slider = HSlider.new()
	bending_slider.min_value = 0.0
	bending_slider.max_value = 10.0
	bending_slider.step = 0.05
	bending_slider.value = 1.0
	bending_slider.value_changed.connect(_on_bending_changed)
	vbox.add_child(bending_slider)

	var stats_row := HBoxContainer.new()
	vbox.add_child(stats_row)

	tris_label = Label.new()
	tris_label.text = "0 tris"
	stats_row.add_child(tris_label)

	verts_label = Label.new()
	verts_label.text = "0 verts"
	stats_row.add_child(verts_label)

	ms_label = Label.new()
	ms_label.text = "0.000 ms"
	stats_row.add_child(ms_label)

	var controls_label := Label.new()
	controls_label.text = "LMB drag cloth, RMB orbit, wheel zoom"
	vbox.add_child(controls_label)


func _spawn_cloth() -> void:
	if cloth_entity_id != 0 and world.is_alive(cloth_entity_id):
		world.destroy_entity(cloth_entity_id)

	cloth_entity_id = world.instantiate_prefab(PREFAB_XPBD_CLOTH_3D, {
		"Position3D": Vector3.ZERO,
		"XPBDCloth3DConfig": {
			"num_x": 30,
			"num_y": 200,
			"num_substeps": 10,
			"spacing": 0.01,
			"thickness": 0.01,
			"bending_compliance": 1.0,
			"gravity_y": - 10.0,
			"ground_height": 0.0,
			"friction": 0.0,
			"handle_collisions": true,
			"attach_corners": false,
		},
		"XPBDCloth3DGrab": {
			"particle_id": - 1,
			"position": Vector3.ZERO,
			"velocity": Vector3.ZERO,
		},
	})

	if cloth_entity_id == 0:
		push_error("Failed to instantiate prefab '%s'" % PREFAB_XPBD_CLOTH_3D)
		return

	drag_particle_id = -1
	timing_sum_ms = 0.0
	timing_frames = 0
	ms_label.text = "0.000 ms"

	world.progress(FIXED_DT)
	_update_meshes()
	_update_stats_labels()


func _process(_delta: float) -> void:
	if cloth_entity_id == 0 or not world.is_alive(cloth_entity_id):
		return

	if running:
		var frame_start_usec := Time.get_ticks_usec()
		world.progress(FIXED_DT)
		var elapsed_ms := float(Time.get_ticks_usec() - frame_start_usec) / 1000.0
		timing_sum_ms += elapsed_ms
		timing_frames += 1
		if timing_frames >= 10:
			var average_ms := timing_sum_ms / float(timing_frames)
			ms_label.text = "%.3f ms" % average_ms
			timing_sum_ms = 0.0
			timing_frames = 0

	_update_meshes()


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_RIGHT:
			orbit_dragging = event.pressed
		elif event.button_index == MOUSE_BUTTON_WHEEL_UP and event.pressed:
			orbit_distance = max(0.2, orbit_distance * 0.92)
			_update_camera_transform()
		elif event.button_index == MOUSE_BUTTON_WHEEL_DOWN and event.pressed:
			orbit_distance = min(4.0, orbit_distance * 1.08)
			_update_camera_transform()
		elif event.button_index == MOUSE_BUTTON_LEFT:
			if event.pressed:
				_start_grab(event.position)
			else:
				_end_grab()
	elif event is InputEventMouseMotion:
		if orbit_dragging:
			orbit_yaw -= event.relative.x * 0.01
			orbit_pitch = clamp(orbit_pitch + event.relative.y * 0.01, -1.4, 1.4)
			_update_camera_transform()
		elif drag_particle_id >= 0:
			_update_grab(event.position)


func _start_grab(screen_position: Vector2) -> void:
	var vertices_variant: Variant = world.get_component("XPBDCloth3DVertices", cloth_entity_id)
	if not (vertices_variant is PackedVector3Array):
		return

	var vertices: PackedVector3Array = vertices_variant
	if vertices.is_empty():
		return

	var ray_origin := camera.project_ray_origin(screen_position)
	var ray_direction := camera.project_ray_normal(screen_position)

	var best_id := -1
	var best_distance_sq := INF
	var best_t := 0.0

	for i in range(vertices.size()):
		var point := vertices[i]
		var to_point := point - ray_origin
		var t := to_point.dot(ray_direction)
		if t < 0.0:
			continue
		var closest := ray_origin + ray_direction * t
		var distance_sq := point.distance_squared_to(closest)
		if distance_sq < best_distance_sq:
			best_distance_sq = distance_sq
			best_id = i
			best_t = t

	if best_id < 0 or best_distance_sq > PICK_DISTANCE_THRESHOLD_SQ:
		return

	drag_particle_id = best_id
	drag_distance = best_t
	previous_drag_position = ray_origin + ray_direction * drag_distance
	drag_velocity = Vector3.ZERO
	_push_grab_component(previous_drag_position, drag_velocity)


func _update_grab(screen_position: Vector2) -> void:
	if drag_particle_id < 0:
		return

	var ray_origin := camera.project_ray_origin(screen_position)
	var ray_direction := camera.project_ray_normal(screen_position)
	var target_position := ray_origin + ray_direction * drag_distance

	var dt: float = max(get_process_delta_time(), 0.0001)
	drag_velocity = (target_position - previous_drag_position) / dt
	previous_drag_position = target_position

	_push_grab_component(target_position, drag_velocity)


func _end_grab() -> void:
	if drag_particle_id < 0:
		return

	world.set_component("XPBDCloth3DGrab", {
		"particle_id": - 1,
		"position": previous_drag_position,
		"velocity": drag_velocity,
	}, cloth_entity_id)

	drag_particle_id = -1


func _push_grab_component(position: Vector3, velocity: Vector3) -> void:
	world.set_component("XPBDCloth3DGrab", {
		"particle_id": drag_particle_id,
		"position": position,
		"velocity": velocity,
	}, cloth_entity_id)


func _update_meshes() -> void:
	var vertices_variant: Variant = world.get_component("XPBDCloth3DVertices", cloth_entity_id)
	var tri_indices_variant: Variant = world.get_component("XPBDCloth3DTriangleIndices", cloth_entity_id)
	var edge_indices_variant: Variant = world.get_component("XPBDCloth3DEdgeIndices", cloth_entity_id)

	if not (vertices_variant is PackedVector3Array):
		return
	if not (tri_indices_variant is PackedInt32Array):
		return
	if not (edge_indices_variant is PackedInt32Array):
		return

	var vertices: PackedVector3Array = vertices_variant
	var tri_indices: PackedInt32Array = tri_indices_variant
	var edge_indices: PackedInt32Array = edge_indices_variant

	if vertices.is_empty() or tri_indices.is_empty():
		return

	var tri_arrays := []
	tri_arrays.resize(Mesh.ARRAY_MAX)
	tri_arrays[Mesh.ARRAY_VERTEX] = vertices
	tri_arrays[Mesh.ARRAY_INDEX] = tri_indices
	tri_arrays[Mesh.ARRAY_NORMAL] = _build_vertex_normals(vertices, tri_indices)

	var cloth_mesh := ArrayMesh.new()
	cloth_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, tri_arrays)
	cloth_mesh_instance.mesh = cloth_mesh

	if show_edges:
		var edge_arrays := []
		edge_arrays.resize(Mesh.ARRAY_MAX)
		edge_arrays[Mesh.ARRAY_VERTEX] = vertices
		edge_arrays[Mesh.ARRAY_INDEX] = edge_indices
		var edge_mesh := ArrayMesh.new()
		edge_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_LINES, edge_arrays)
		edge_mesh_instance.mesh = edge_mesh


func _build_vertex_normals(vertices: PackedVector3Array, tri_indices: PackedInt32Array) -> PackedVector3Array:
	var normals := PackedVector3Array()
	normals.resize(vertices.size())

	for i in range(normals.size()):
		normals[i] = Vector3.ZERO

	for i in range(0, tri_indices.size(), 3):
		var i0: int = tri_indices[i]
		var i1: int = tri_indices[i + 1]
		var i2: int = tri_indices[i + 2]

		if i0 < 0 or i1 < 0 or i2 < 0:
			continue
		if i0 >= vertices.size() or i1 >= vertices.size() or i2 >= vertices.size():
			continue

		var p0: Vector3 = vertices[i0]
		var p1: Vector3 = vertices[i1]
		var p2: Vector3 = vertices[i2]
		var face_normal := (p1 - p0).cross(p2 - p0)
		if face_normal.length_squared() <= 1e-12:
			continue

		normals[i0] += face_normal
		normals[i1] += face_normal
		normals[i2] += face_normal

	for i in range(normals.size()):
		var n: Vector3 = normals[i]
		if n.length_squared() <= 1e-12:
			normals[i] = Vector3.UP
		else:
			normals[i] = n.normalized()

	return normals


func _update_stats_labels() -> void:
	var vertices_variant: Variant = world.get_component("XPBDCloth3DVertices", cloth_entity_id)
	var tri_indices_variant: Variant = world.get_component("XPBDCloth3DTriangleIndices", cloth_entity_id)

	if vertices_variant is PackedVector3Array:
		var vertices: PackedVector3Array = vertices_variant
		verts_label.text = "%d verts" % vertices.size()

	if tri_indices_variant is PackedInt32Array:
		var triangles: PackedInt32Array = tri_indices_variant
		tris_label.text = "%d tris" % int(triangles.size() / 3)


func _update_camera_transform() -> void:
	var cos_pitch := cos(orbit_pitch)
	var offset := Vector3(
		orbit_distance * sin(orbit_yaw) * cos_pitch,
		orbit_distance * sin(orbit_pitch),
		orbit_distance * cos(orbit_yaw) * cos_pitch
	)
	camera.global_position = orbit_target + offset
	camera.look_at(orbit_target)


func _on_run_pressed() -> void:
	running = not running
	run_button.text = "Stop" if running else "Run"


func _on_restart_pressed() -> void:
	_spawn_cloth()


func _on_show_edges_toggled(enabled: bool) -> void:
	show_edges = enabled
	cloth_mesh_instance.visible = not enabled
	edge_mesh_instance.visible = enabled


func _on_collision_toggled(enabled: bool) -> void:
	if cloth_entity_id == 0:
		return
	var config_variant: Variant = world.get_component("XPBDCloth3DConfig", cloth_entity_id)
	if not (config_variant is Dictionary):
		return
	var config: Dictionary = config_variant
	config["handle_collisions"] = enabled
	world.set_component("XPBDCloth3DConfig", config, cloth_entity_id)


func _on_bending_changed(value: float) -> void:
	if cloth_entity_id == 0:
		return
	var config_variant: Variant = world.get_component("XPBDCloth3DConfig", cloth_entity_id)
	if not (config_variant is Dictionary):
		return
	var config: Dictionary = config_variant
	config["bending_compliance"] = value
	world.set_component("XPBDCloth3DConfig", config, cloth_entity_id)
