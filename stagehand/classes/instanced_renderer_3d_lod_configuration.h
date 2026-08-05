#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/resource.hpp>

/// Resource representing a single LOD level on an InstancedRenderer3D.
class InstancedRenderer3DLODConfiguration : public godot::Resource {
    GDCLASS(InstancedRenderer3DLODConfiguration, godot::Resource)

  public:
    void set_mesh(const godot::Ref<godot::Mesh> &p_mesh);
    [[nodiscard]] godot::Ref<godot::Mesh> get_mesh() const { return mesh; }

    /// Starting distance from which the GeometryInstance3D will be visible, taking visibility_range_begin_margin into account as well. The default value of 0
    /// is used to disable the range check.
    void set_visibility_range_begin(float p_value);
    [[nodiscard]] float get_visibility_range_begin() const { return visibility_range_begin; }

    /// Distance from which the GeometryInstance3D will be hidden, taking visibility_range_end_margin into account as well. The default value of 0 is used to
    /// disable the range check.
    void set_visibility_range_end(float p_value);
    [[nodiscard]] float get_visibility_range_end() const { return visibility_range_end; }

    /// Margin for the visibility_range_begin threshold. The GeometryInstance3D will only change its visibility state when it goes over or under the
    /// visibility_range_begin threshold by this amount. If visibility_range_fade_mode is VISIBILITY_RANGE_FADE_DISABLED, this acts as a hysteresis distance. If
    /// visibility_range_fade_mode is VISIBILITY_RANGE_FADE_SELF or VISIBILITY_RANGE_FADE_DEPENDENCIES, this acts as a fade transition distance and must be set
    /// to a value greater than 0.0 for the effect to be noticeable.
    void set_visibility_range_begin_margin(float p_value);
    [[nodiscard]] float get_visibility_range_begin_margin() const { return visibility_range_begin_margin; }

    /// Margin for the visibility_range_end threshold. The GeometryInstance3D will only change its visibility state when it goes over or under the
    /// visibility_range_end threshold by this amount. If visibility_range_fade_mode is VISIBILITY_RANGE_FADE_DISABLED, this acts as a hysteresis distance. If
    /// visibility_range_fade_mode is VISIBILITY_RANGE_FADE_SELF or VISIBILITY_RANGE_FADE_DEPENDENCIES, this acts as a fade transition distance and must be set
    /// to a value greater than 0.0 for the effect to be noticeable.
    void set_visibility_range_end_margin(float p_value);
    [[nodiscard]] float get_visibility_range_end_margin() const { return visibility_range_end_margin; }

    /// Controls which instances will be faded when approaching the limits of the visibility range.
    void set_visibility_range_fade_mode(godot::RenderingServer::VisibilityRangeFadeMode p_value);
    [[nodiscard]] godot::RenderingServer::VisibilityRangeFadeMode get_visibility_range_fade_mode() const { return visibility_range_fade_mode; }

  protected:
    static void _bind_methods();

  private:
    godot::Ref<godot::Mesh> mesh;
    float visibility_range_begin = 0.0f;
    float visibility_range_end = 0.0f;
    float visibility_range_begin_margin = 0.0f;
    float visibility_range_end_margin = 0.0f;
    godot::RenderingServer::VisibilityRangeFadeMode visibility_range_fade_mode = godot::RenderingServer::VISIBILITY_RANGE_FADE_SELF;
};
