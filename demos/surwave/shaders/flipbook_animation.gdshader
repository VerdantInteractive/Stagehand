shader_type canvas_item;

// Color Masking constants
const int BLEND_MODE_NONE = 0;
const int BLEND_MODE_ADD = 1;
const int BLEND_MODE_SUBTRACT = 2;
const int BLEND_MODE_MULTIPLY = 3;

// --- Spritesheet layout ---
uniform int columns = 1;
uniform int rows = 1;

varying vec4 anim_and_flip_data; // .r: animated_frame_index, .g: flip_horizontal, .b: flip_vertical, .a: alpha
varying vec4 vertex_to_fragment_data; // .xy: actual_frame_size_uv, .zw: frame_image_origin_uv
varying vec3 color_mask;
varying float blend_mode;
varying float has_color_mask;

void vertex() {
    // ─────────────────────────────────────────
    // 1. Retrieve and process per-instance data
    // ─────────────────────────────────────────
    // The hardcoded values are for temporary testing purposes
    float base_frame_and_transparency = INSTANCE_CUSTOM.r; // Abs(Whole part) holds base frame index. When negative, the fractional part determines transparency as a percentage.
    float anim_range_and_time_offset  = INSTANCE_CUSTOM.g; // Abs(Whole part) Animation range in frames, fractional part: Animation time offset as a fraction of the cycle duration (sign respected).
    float anim_speed_and_direction    = INSTANCE_CUSTOM.b; // Animation speed in frames per second; sign controls direction. Set to zero to show the base frame without animation.    
    float flags                       = INSTANCE_CUSTOM.a; // Controls flip and blend modes, and a colour mask. See the detailed explanation at the bottom.

    float column_count = max(float(columns), 1.0);
    float row_count = max(float(rows), 1.0);

    vec4 local_anim_and_flag_data;

    // If base_frame_and_transparency is negative, its fractional part determines transparency.
    float is_transparent = step(base_frame_and_transparency, -1e-5); // 1.0 if base_frame_and_transparency is negative, 0.0 otherwise
    float transparency = fract(abs(base_frame_and_transparency));
    local_anim_and_flag_data.a = mix(1.0, 1.0 - transparency, is_transparent);

    // Color masking is active only if the blend mode is something other than BLEND_MODE_NONE (0).
    // This correctly handles cases where only flipping is desired (flags = 1, 2, or 3).
    has_color_mask = step(0.5, float((int(flags) >> 2) & 7));

    int flags_int = int(flags);

    // Decode flip, blend mode, and color from the integer part of 'flags'
    int flip_mode = flags_int & 3; // Bits 0-1
    blend_mode = float((flags_int >> 2) & 7); // Bits 2-4

    float r = float((flags_int >> 5) & 63) / 63.0;
    float g = float((flags_int >> 11) & 63) / 63.0;
    float b = float((flags_int >> 17) & 63) / 63.0;
    color_mask = vec3(r, g, b);

    local_anim_and_flag_data.y = float(flip_mode & 1); // flip_h
    local_anim_and_flag_data.z = float((flip_mode >> 1) & 1); // flip_v

    float base_frame_index = floor(abs(base_frame_and_transparency));

    // ─────────────────────────────────────────
    // 2. Animation timing and current frame
    // ─────────────────────────────────────────
    // Extract animation range and direction
    float anim_window_length = floor(abs(anim_range_and_time_offset)) + 1.0;
    float anim_direction     = sign(anim_speed_and_direction); // +1 → right, -1 → left

    // Extract animation time offset
    float anim_offset_fraction = anim_range_and_time_offset - trunc(anim_range_and_time_offset);

    // Calculate cycle duration and animation phase
    float anim_speed = abs(anim_speed_and_direction);
    float cycle_duration = anim_window_length / max(anim_speed, 1e-5); // Add epsilon to prevent division by zero
    float time_offset = anim_offset_fraction * cycle_duration;
    
    // Compute time within the current animation cycle, accounting for direction
    float time_in_cycle = mod((TIME * anim_direction) + time_offset, cycle_duration);

    // Current frame offset within the animation window
    float raw_anim_frame_offset = floor(time_in_cycle * anim_speed);
    float anim_frame_offset = min(raw_anim_frame_offset, anim_window_length - 1.0);

    float total_frames_in_sheet = column_count * row_count;
    float animated_frame_index = mod(base_frame_index + anim_frame_offset, total_frames_in_sheet);
    local_anim_and_flag_data.x = animated_frame_index;

    // Calculate and store data for the fragment shader
    // UV size of a single grid cell (including padding and margin distribution)
    vec2 actual_frame_size_uv = vec2(1.0 / column_count, 1.0 / row_count);

    // Calculate the UV offset for the current frame's top-left corner
    float column_index = mod(animated_frame_index, column_count);
    float row_index    = floor(animated_frame_index / column_count);
    
    // Origin of the current frame's *image* on the spritesheet
    vec2 frame_image_origin_uv = vec2(column_index, row_index) * actual_frame_size_uv;

    vertex_to_fragment_data = vec4(actual_frame_size_uv, frame_image_origin_uv);
    anim_and_flip_data = local_anim_and_flag_data;
}

void fragment() {
    // ─────────────────────────────────────────
    // 3. Unpack vertex shader results
    // ─────────────────────────────────────────
    float animated_frame_index = anim_and_flip_data.r;
    float flip_horizontal = anim_and_flip_data.g;
    float flip_vertical = anim_and_flip_data.b;
    float alpha = anim_and_flip_data.a;
    vec2 actual_frame_size_uv = vertex_to_fragment_data.xy;
    vec2 frame_image_origin_uv = vertex_to_fragment_data.zw;

    // ─────────────────────────────────────────
    // 4. Local UV transform (flip)
    // ─────────────────────────────────────────
    vec2 uv_flipped = UV;

    // Apply flips
    uv_flipped.x = mix(uv_flipped.x, 1.0 - uv_flipped.x, flip_horizontal);
    uv_flipped.y = mix(uv_flipped.y, 1.0 - uv_flipped.y, flip_vertical);

    // ─────────────────────────────────────────
    // 5. Sample the texture
    // ─────────────────────────────────────────
    vec4 tex_color = texture(TEXTURE, frame_image_origin_uv + uv_flipped * actual_frame_size_uv);

    // Branchless color masking
    // Determine which blend mode is active.
    float is_add = 1.0 - step(0.1, abs(blend_mode - float(BLEND_MODE_ADD)));
    float is_subtract = 1.0 - step(0.1, abs(blend_mode - float(BLEND_MODE_SUBTRACT)));
    float is_multiply = 1.0 - step(0.1, abs(blend_mode - float(BLEND_MODE_MULTIPLY)));

    // Calculate the result for each blend mode.
    vec3 add_result = tex_color.rgb + color_mask;
    vec3 subtract_result = tex_color.rgb - color_mask;
    vec3 multiply_result = tex_color.rgb * color_mask;

    // Combine the results based on the active blend mode and whether masking is enabled.
    vec3 masked_color = is_add * add_result + is_subtract * subtract_result + is_multiply * multiply_result;
    tex_color.rgb = mix(tex_color.rgb, masked_color, has_color_mask);
    
    // When blend_mode is 0 (BLEND_MODE_NONE), none of the 'is_*' flags will be active, resulting in masked_color being vec3(0.0).
    // The mix() function then correctly defaults to tex_color.rgb, effectively applying no blending.

    COLOR = tex_color;
    COLOR.a *= alpha;
}

// The 'flags' variable is a bit-packed float used to control flipping and colour masking effects.
// A value of 0.0 indicates that no effects should be applied.
//
// The integer part of the float is used for bitwise operations. Here is the layout:
//
// Bits    | Purpose        | Range   | Description
// --------|----------------|---------|-------------------------------------------------
// 0-1     | Flip Mode      | 0-3     | 0: None, 1: H-Flip, 2: V-Flip, 3: H & V-Flip
// 2-4     | Blend Mode     | 0-3     | 0: None, 1: Add, 2: Subtract, 3: Multiply
// 5-10    | Red Channel    | 0-63    | 6 bits for the Red colour channel (0-63).
// 11-16   | Green Channel  | 0-63    | 6 bits for the Green colour channel (0-63).
// 17-22   | Blue Channel   | 0-63    | 6 bits for the Blue colour channel (0-63).
//
// --- How to Calculate the 'flags' Value ---
// You can construct the 'flags' value by summing the integer values for each component.
//
// Value = (Flip) + (Blend << 2) + (R << 5) + (G << 11) + (B << 17)
//
// Where:
// - R, G, B are 0-63. To convert from 0-255, use: floor(value * 63.0 / 255.0).
//
// The largest valid integer value for flags is 8,388,607.0. Invalid (incl. negative) values will result in non-transparent pixels coloured in black.
//
// --- Examples ---
//
// 1. Horizontal Flip only:
//    flags = 1.0
//
// 2. Additive blend with pure Red (R:63, G:0, B:0):
//    - Flip: 0
//    - Blend: 1 (Add) -> 1 << 2 = 4
//    - Red: 63 -> 63 << 5 = 2016
//    flags = 0 + 4 + 2016 = 2020.0
//
// 3. Multiply blend with Cyan (R:0, G:63, B:63) and H+V flip:
//    - Flip: 3
//    - Blend: 3 (Multiply) -> 3 << 2 = 12
//    - Green: 63 -> 63 << 11 = 129024
//    - Blue: 63 -> 63 << 17 = 8257536
//    flags = 3 + 12 + 129024 + 8257536 = 8386575.0
//
