#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/components/physics.h"
#include "stagehand/ecs/components/transform.h"
#include "stagehand/ecs/pipeline_phases.h"
#include "stagehand/registry.h"

namespace stagehand::physics {

    struct XPBDSpatialHash3D {
        float spacing = 0.01f;
        int32_t table_size = 1;
        int32_t max_num_objects = 0;
        int32_t query_size = 0;

        std::vector<int32_t> cell_start;
        std::vector<int32_t> cell_entries;
        std::vector<int32_t> query_ids;
        std::vector<int32_t> first_adj_id;
        std::vector<int32_t> adj_ids;

        void initialize(float in_spacing, int32_t in_max_num_objects) {
            spacing = std::max(in_spacing, 1e-5f);
            max_num_objects = std::max(in_max_num_objects, 0);
            table_size = std::max(5 * std::max(max_num_objects, 1), 1);

            cell_start.assign(static_cast<size_t>(table_size) + 1U, 0);
            cell_entries.assign(static_cast<size_t>(max_num_objects), 0);
            query_ids.assign(static_cast<size_t>(std::max(max_num_objects, 1)), 0);
            first_adj_id.assign(static_cast<size_t>(max_num_objects) + 1U, 0);
            adj_ids.assign(static_cast<size_t>(std::max(10 * max_num_objects, 1)), 0);
            query_size = 0;
        }

        [[nodiscard]] int32_t hash_coords(int32_t xi, int32_t yi, int32_t zi) const {
            const int64_t h = (static_cast<int64_t>(xi) * 92837111LL) ^ (static_cast<int64_t>(yi) * 689287499LL) ^
                              (static_cast<int64_t>(zi) * 283923481LL);
            return static_cast<int32_t>(std::llabs(h) % static_cast<int64_t>(table_size));
        }

        [[nodiscard]] int32_t int_coord(float coord) const { return static_cast<int32_t>(std::floor(coord / spacing)); }

        [[nodiscard]] int32_t hash_pos(const std::vector<godot::Vector3> &pos, int32_t id) const {
            const godot::Vector3 &p = pos[static_cast<size_t>(id)];
            return hash_coords(int_coord(p.x), int_coord(p.y), int_coord(p.z));
        }

        void create(const std::vector<godot::Vector3> &pos, int32_t num_objects) {
            num_objects = std::min(num_objects, max_num_objects);
            if (num_objects <= 0) {
                return;
            }

            std::fill(cell_start.begin(), cell_start.end(), 0);
            std::fill(cell_entries.begin(), cell_entries.end(), 0);

            for (int32_t i = 0; i < num_objects; ++i) {
                const int32_t h = hash_pos(pos, i);
                cell_start[static_cast<size_t>(h)] += 1;
            }

            int32_t start = 0;
            for (int32_t i = 0; i < table_size; ++i) {
                start += cell_start[static_cast<size_t>(i)];
                cell_start[static_cast<size_t>(i)] = start;
            }
            cell_start[static_cast<size_t>(table_size)] = start;

            for (int32_t i = 0; i < num_objects; ++i) {
                const int32_t h = hash_pos(pos, i);
                int32_t &cell_begin = cell_start[static_cast<size_t>(h)];
                cell_begin -= 1;
                cell_entries[static_cast<size_t>(cell_begin)] = i;
            }
        }

        void query(const std::vector<godot::Vector3> &pos, int32_t id, float max_dist) {
            const godot::Vector3 &p = pos[static_cast<size_t>(id)];

            const int32_t x0 = int_coord(p.x - max_dist);
            const int32_t y0 = int_coord(p.y - max_dist);
            const int32_t z0 = int_coord(p.z - max_dist);

            const int32_t x1 = int_coord(p.x + max_dist);
            const int32_t y1 = int_coord(p.y + max_dist);
            const int32_t z1 = int_coord(p.z + max_dist);

            query_size = 0;

            for (int32_t xi = x0; xi <= x1; ++xi) {
                for (int32_t yi = y0; yi <= y1; ++yi) {
                    for (int32_t zi = z0; zi <= z1; ++zi) {
                        const int32_t h = hash_coords(xi, yi, zi);
                        const int32_t start = cell_start[static_cast<size_t>(h)];
                        const int32_t end = cell_start[static_cast<size_t>(h + 1)];

                        for (int32_t i = start; i < end; ++i) {
                            if (query_size >= static_cast<int32_t>(query_ids.size())) {
                                query_ids.resize(std::max(1, query_size * 2));
                            }
                            query_ids[static_cast<size_t>(query_size)] = cell_entries[static_cast<size_t>(i)];
                            query_size += 1;
                        }
                    }
                }
            }
        }

        void query_all(const std::vector<godot::Vector3> &pos, float max_dist, int32_t num_objects) {
            num_objects = std::min(num_objects, max_num_objects);
            if (num_objects <= 0) {
                return;
            }

            if (first_adj_id.size() < static_cast<size_t>(num_objects) + 1U) {
                first_adj_id.resize(static_cast<size_t>(num_objects) + 1U, 0);
            }

            int32_t num = 0;
            const float max_dist2 = max_dist * max_dist;

            for (int32_t i = 0; i < num_objects; ++i) {
                first_adj_id[static_cast<size_t>(i)] = num;
                query(pos, i, max_dist);

                for (int32_t q = 0; q < query_size; ++q) {
                    const int32_t id1 = query_ids[static_cast<size_t>(q)];
                    if (id1 >= i) {
                        continue;
                    }

                    const float dist2 = pos[static_cast<size_t>(i)].distance_squared_to(pos[static_cast<size_t>(id1)]);
                    if (dist2 > max_dist2) {
                        continue;
                    }

                    if (num >= static_cast<int32_t>(adj_ids.size())) {
                        adj_ids.resize(std::max(1, num * 2));
                    }
                    adj_ids[static_cast<size_t>(num)] = id1;
                    num += 1;
                }
            }

            first_adj_id[static_cast<size_t>(num_objects)] = num;
        }
    };

    struct XPBDCloth3DState {
        bool initialized = false;
        int32_t num_x = 0;
        int32_t num_y = 0;
        int32_t num_particles = 0;
        float thickness = 0.01f;

        int32_t grab_id = -1;
        float grab_inv_mass = 0.0f;

        XPBDCloth3DConfig baked_config;

        std::vector<godot::Vector3> pos;
        std::vector<godot::Vector3> prev_pos;
        std::vector<godot::Vector3> rest_pos;
        std::vector<godot::Vector3> vel;
        std::vector<float> inv_mass;

        std::vector<int32_t> ids;
        std::vector<float> compliances;
        std::vector<float> rest_lens;
        std::vector<uint8_t> constraint_types;

        XPBDSpatialHash3D hash;
    };

    REGISTER([](flecs::world &world) { world.component<XPBDCloth3DState>(); });

    namespace internal {
        inline constexpr float XPBD_MIN_DT = 1e-6f;

        inline bool is_xpbd3d_body(PhysicsBodyType body_type) { return body_type == PhysicsBodyType::XPBD3D; }

        inline XPBDCloth3DConfig sanitize_config(XPBDCloth3DConfig config) {
            config.num_x = std::max(config.num_x, 2);
            config.num_y = std::max(config.num_y, 2);
            config.num_substeps = std::max(config.num_substeps, 1);
            config.spacing = std::max(config.spacing, 1e-5f);
            config.thickness = std::max(config.thickness, 1e-5f);
            return config;
        }

        inline bool requires_rebuild(const XPBDCloth3DState &state, const XPBDCloth3DConfig &config) {
            if (!state.initialized) {
                return true;
            }

            return state.baked_config.num_x != config.num_x || state.baked_config.num_y != config.num_y ||
                   state.baked_config.attach_corners != config.attach_corners || std::abs(state.baked_config.spacing - config.spacing) > 1e-6f ||
                   std::abs(state.baked_config.thickness - config.thickness) > 1e-6f;
        }

        inline void build_visual_indices(int32_t num_x, int32_t num_y, XPBDCloth3DTriangleIndices &triangles, XPBDCloth3DEdgeIndices &edges) {
            triangles.clear();
            edges.clear();

            for (int32_t i = 0; i < num_x; ++i) {
                for (int32_t j = 0; j < num_y; ++j) {
                    const int32_t id = i * num_y + j;

                    if (i < num_x - 1 && j < num_y - 1) {
                        triangles.push_back(id + 1);
                        triangles.push_back(id);
                        triangles.push_back(id + 1 + num_y);

                        triangles.push_back(id + 1 + num_y);
                        triangles.push_back(id);
                        triangles.push_back(id + num_y);
                    }

                    if (i < num_x - 1) {
                        edges.push_back(id);
                        edges.push_back(id + num_y);
                    }

                    if (j < num_y - 1) {
                        edges.push_back(id);
                        edges.push_back(id + 1);
                    }
                }
            }
        }

        inline void update_output_vertices(const std::vector<godot::Vector3> &positions, XPBDCloth3DVertices &vertices) {
            vertices.resize(static_cast<int32_t>(positions.size()));
            for (int32_t i = 0; i < static_cast<int32_t>(positions.size()); ++i) {
                vertices.set(i, positions[static_cast<size_t>(i)]);
            }
        }

        inline void initialize_state(XPBDCloth3DState &state, const XPBDCloth3DConfig &config, const godot::Vector3 &origin) {
            state.baked_config = config;
            state.num_x = config.num_x;
            state.num_y = config.num_y;
            state.num_particles = state.num_x * state.num_y;
            state.thickness = config.thickness;
            state.grab_id = -1;
            state.grab_inv_mass = 0.0f;

            state.pos.assign(static_cast<size_t>(state.num_particles), godot::Vector3());
            state.prev_pos.assign(static_cast<size_t>(state.num_particles), godot::Vector3());
            state.rest_pos.assign(static_cast<size_t>(state.num_particles), godot::Vector3());
            state.vel.assign(static_cast<size_t>(state.num_particles), godot::Vector3());
            state.inv_mass.assign(static_cast<size_t>(state.num_particles), 1.0f);

            std::mt19937 rng(static_cast<uint32_t>(state.num_particles * 977U + state.num_x * 131U + state.num_y * 167U));
            std::uniform_real_distribution<float> jitter_dist(-1.0f, 1.0f);
            const float jitter = 0.001f * config.spacing;

            for (int32_t i = 0; i < state.num_x; ++i) {
                for (int32_t j = 0; j < state.num_y; ++j) {
                    const int32_t id = i * state.num_y + j;
                    godot::Vector3 p;
                    p.x = origin.x - static_cast<float>(state.num_x) * config.spacing * 0.5f + static_cast<float>(i) * config.spacing;
                    p.y = origin.y + 0.2f + static_cast<float>(j) * config.spacing;
                    p.z = origin.z;

                    p.x += jitter * jitter_dist(rng);
                    p.y += jitter * jitter_dist(rng);
                    p.z += jitter * jitter_dist(rng);

                    state.pos[static_cast<size_t>(id)] = p;

                    if (config.attach_corners && j == state.num_y - 1 && (i == 0 || i == state.num_x - 1)) {
                        state.inv_mass[static_cast<size_t>(id)] = 0.0f;
                    }
                }
            }

            state.prev_pos = state.pos;
            state.rest_pos = state.pos;

            constexpr int32_t num_constraint_types = 6;
            constexpr std::array<int32_t, 24> offsets = {
                0, 0, 0, 1,
                0, 0, 1, 0,
                0, 0, 1, 1,
                0, 1, 1, 0,
                0, 0, 0, 2,
                0, 0, 2, 0,
            };

            state.ids.clear();
            state.compliances.clear();
            state.rest_lens.clear();
            state.constraint_types.clear();

            const float stretch_compliance = 0.0f;
            const float shear_compliance = 0.0001f;
            const std::array<float, num_constraint_types> compliance_by_type = {
                stretch_compliance,
                stretch_compliance,
                shear_compliance,
                shear_compliance,
                config.bending_compliance,
                config.bending_compliance,
            };

            state.ids.reserve(static_cast<size_t>(state.num_particles) * num_constraint_types * 2U);
            state.compliances.reserve(static_cast<size_t>(state.num_particles) * num_constraint_types);
            state.constraint_types.reserve(static_cast<size_t>(state.num_particles) * num_constraint_types);

            for (int32_t constraint_type = 0; constraint_type < num_constraint_types; ++constraint_type) {
                for (int32_t i = 0; i < state.num_x; ++i) {
                    for (int32_t j = 0; j < state.num_y; ++j) {
                        const int32_t p = 4 * constraint_type;

                        const int32_t i0 = i + offsets[static_cast<size_t>(p)];
                        const int32_t j0 = j + offsets[static_cast<size_t>(p + 1)];
                        const int32_t i1 = i + offsets[static_cast<size_t>(p + 2)];
                        const int32_t j1 = j + offsets[static_cast<size_t>(p + 3)];

                        if (i0 < state.num_x && j0 < state.num_y && i1 < state.num_x && j1 < state.num_y) {
                            const int32_t id0 = i0 * state.num_y + j0;
                            const int32_t id1 = i1 * state.num_y + j1;

                            state.ids.push_back(id0);
                            state.ids.push_back(id1);
                            state.compliances.push_back(compliance_by_type[static_cast<size_t>(constraint_type)]);
                            state.constraint_types.push_back(static_cast<uint8_t>(constraint_type));
                        }
                    }
                }
            }

            const int32_t num_constraints = static_cast<int32_t>(state.compliances.size());
            state.rest_lens.resize(static_cast<size_t>(num_constraints), 0.0f);
            for (int32_t i = 0; i < num_constraints; ++i) {
                const int32_t id0 = state.ids[static_cast<size_t>(2 * i)];
                const int32_t id1 = state.ids[static_cast<size_t>(2 * i + 1)];
                state.rest_lens[static_cast<size_t>(i)] = state.pos[static_cast<size_t>(id0)].distance_to(state.pos[static_cast<size_t>(id1)]);
            }

            state.hash.initialize(config.spacing, state.num_particles);
            state.initialized = true;
        }

        inline void update_constraint_compliances(XPBDCloth3DState &state, const XPBDCloth3DConfig &config) {
            const float stretch_compliance = 0.0f;
            const float shear_compliance = 0.0001f;
            const float bending_compliance = config.bending_compliance;

            for (size_t i = 0; i < state.compliances.size(); ++i) {
                const uint8_t type = state.constraint_types[i];
                if (type <= 1U) {
                    state.compliances[i] = stretch_compliance;
                } else if (type <= 3U) {
                    state.compliances[i] = shear_compliance;
                } else {
                    state.compliances[i] = bending_compliance;
                }
            }
        }

        inline void solve_ground_collisions(XPBDCloth3DState &state, const XPBDCloth3DConfig &config) {
            const float min_y = config.ground_height + 0.5f * state.thickness;

            for (int32_t i = 0; i < state.num_particles; ++i) {
                if (state.inv_mass[static_cast<size_t>(i)] <= 0.0f) {
                    continue;
                }

                godot::Vector3 &p = state.pos[static_cast<size_t>(i)];
                if (p.y < min_y) {
                    const godot::Vector3 delta = p - state.prev_pos[static_cast<size_t>(i)];
                    p -= delta;
                    p.y = min_y;
                }
            }
        }

        inline void solve_constraints(XPBDCloth3DState &state, float dt) {
            const int32_t num_constraints = static_cast<int32_t>(state.compliances.size());

            for (int32_t i = 0; i < num_constraints; ++i) {
                const int32_t id0 = state.ids[static_cast<size_t>(2 * i)];
                const int32_t id1 = state.ids[static_cast<size_t>(2 * i + 1)];

                const float w0 = state.inv_mass[static_cast<size_t>(id0)];
                const float w1 = state.inv_mass[static_cast<size_t>(id1)];
                const float w = w0 + w1;
                if (w <= 0.0f) {
                    continue;
                }

                godot::Vector3 dir = state.pos[static_cast<size_t>(id0)] - state.pos[static_cast<size_t>(id1)];
                const float len = dir.length();
                if (len <= 0.0f) {
                    continue;
                }

                dir /= len;
                const float c = len - state.rest_lens[static_cast<size_t>(i)];
                const float alpha = state.compliances[static_cast<size_t>(i)] / (dt * dt);
                const float s = -c / (w + alpha);

                state.pos[static_cast<size_t>(id0)] += dir * (s * w0);
                state.pos[static_cast<size_t>(id1)] -= dir * (s * w1);
            }
        }

        inline void solve_self_collisions(XPBDCloth3DState &state, const XPBDCloth3DConfig &config) {
            const float thickness2 = state.thickness * state.thickness;

            for (int32_t i = 0; i < state.num_particles; ++i) {
                if (state.inv_mass[static_cast<size_t>(i)] <= 0.0f) {
                    continue;
                }

                const int32_t first = state.hash.first_adj_id[static_cast<size_t>(i)];
                const int32_t last = state.hash.first_adj_id[static_cast<size_t>(i + 1)];

                for (int32_t j = first; j < last; ++j) {
                    const int32_t id1 = state.hash.adj_ids[static_cast<size_t>(j)];
                    if (state.inv_mass[static_cast<size_t>(id1)] <= 0.0f) {
                        continue;
                    }

                    godot::Vector3 delta = state.pos[static_cast<size_t>(id1)] - state.pos[static_cast<size_t>(i)];
                    const float dist2 = delta.length_squared();
                    if (dist2 > thickness2 || dist2 <= 0.0f) {
                        continue;
                    }

                    const float rest_dist2 = state.rest_pos[static_cast<size_t>(i)].distance_squared_to(state.rest_pos[static_cast<size_t>(id1)]);
                    if (dist2 > rest_dist2) {
                        continue;
                    }

                    float min_dist = state.thickness;
                    if (rest_dist2 < thickness2) {
                        min_dist = std::sqrt(rest_dist2);
                    }

                    const float dist = std::sqrt(dist2);
                    if (dist <= 0.0f) {
                        continue;
                    }

                    const godot::Vector3 correction = delta * ((min_dist - dist) / dist);
                    state.pos[static_cast<size_t>(i)] -= correction * 0.5f;
                    state.pos[static_cast<size_t>(id1)] += correction * 0.5f;

                    const godot::Vector3 v0 = state.pos[static_cast<size_t>(i)] - state.prev_pos[static_cast<size_t>(i)];
                    const godot::Vector3 v1 = state.pos[static_cast<size_t>(id1)] - state.prev_pos[static_cast<size_t>(id1)];
                    const godot::Vector3 v_avg = (v0 + v1) * 0.5f;

                    const godot::Vector3 corr0 = v_avg - v0;
                    const godot::Vector3 corr1 = v_avg - v1;

                    state.pos[static_cast<size_t>(i)] += corr0 * config.friction;
                    state.pos[static_cast<size_t>(id1)] += corr1 * config.friction;
                }
            }
        }

        inline void apply_grab(XPBDCloth3DState &state, const XPBDCloth3DGrab &grab) {
            const int32_t requested_id = grab.particle_id;

            if (requested_id >= 0 && requested_id < state.num_particles) {
                if (state.grab_id != requested_id) {
                    if (state.grab_id >= 0 && state.grab_id < state.num_particles) {
                        state.inv_mass[static_cast<size_t>(state.grab_id)] = state.grab_inv_mass;
                    }

                    state.grab_id = requested_id;
                    state.grab_inv_mass = state.inv_mass[static_cast<size_t>(requested_id)];
                    state.inv_mass[static_cast<size_t>(requested_id)] = 0.0f;
                }

                state.pos[static_cast<size_t>(requested_id)] = grab.position;
                return;
            }

            if (state.grab_id >= 0 && state.grab_id < state.num_particles) {
                state.inv_mass[static_cast<size_t>(state.grab_id)] = state.grab_inv_mass;
                state.vel[static_cast<size_t>(state.grab_id)] = grab.velocity;
            }
            state.grab_id = -1;
        }

        inline void simulate(XPBDCloth3DState &state, const XPBDCloth3DConfig &config, const XPBDCloth3DGrab &grab, float frame_dt) {
            const float safe_frame_dt = std::max(frame_dt, XPBD_MIN_DT);
            const float dt = safe_frame_dt / static_cast<float>(config.num_substeps);
            const float max_velocity = 0.2f * state.thickness / dt;
            const godot::Vector3 gravity(0.0f, config.gravity_y, 0.0f);

            update_constraint_compliances(state, config);
            apply_grab(state, grab);

            if (config.handle_collisions) {
                state.hash.create(state.pos, state.num_particles);
                const float max_travel_dist = max_velocity * safe_frame_dt;
                state.hash.query_all(state.pos, max_travel_dist, state.num_particles);
            }

            for (int32_t step = 0; step < config.num_substeps; ++step) {
                apply_grab(state, grab);

                for (int32_t i = 0; i < state.num_particles; ++i) {
                    if (state.inv_mass[static_cast<size_t>(i)] <= 0.0f) {
                        continue;
                    }

                    godot::Vector3 &v = state.vel[static_cast<size_t>(i)];
                    v += gravity * dt;

                    const float speed = v.length();
                    if (speed > max_velocity && speed > 0.0f) {
                        v *= max_velocity / speed;
                    }

                    state.prev_pos[static_cast<size_t>(i)] = state.pos[static_cast<size_t>(i)];
                    state.pos[static_cast<size_t>(i)] += v * dt;
                }

                solve_ground_collisions(state, config);
                solve_constraints(state, dt);
                if (config.handle_collisions) {
                    solve_self_collisions(state, config);
                }

                for (int32_t i = 0; i < state.num_particles; ++i) {
                    if (state.inv_mass[static_cast<size_t>(i)] <= 0.0f) {
                        continue;
                    }
                    state.vel[static_cast<size_t>(i)] = (state.pos[static_cast<size_t>(i)] - state.prev_pos[static_cast<size_t>(i)]) / dt;
                }
            }
        }
    } // namespace internal

    REGISTER([](flecs::world &world) {
        world.system<const PhysicsBodyType>("stagehand::physics::XPBD3D Bootstrap")
            .kind(stagehand::OnEarlyUpdate)
            .template without<XPBDCloth3DState>()
            .each([](flecs::entity entity, const PhysicsBodyType &body_type) {
                if (!internal::is_xpbd3d_body(body_type) || entity.has(flecs::Prefab)) {
                    return;
                }
                if (!entity.has<XPBDCloth3DConfig>()) {
                    entity.set<XPBDCloth3DConfig>(XPBDCloth3DConfig());
                }
                if (!entity.has<XPBDCloth3DGrab>()) {
                    entity.set<XPBDCloth3DGrab>(XPBDCloth3DGrab());
                }
                entity.set<XPBDCloth3DState>(XPBDCloth3DState());

                if (godot::gdextension_interface::library != nullptr) {
                    if (!entity.has<XPBDCloth3DVertices>()) {
                        entity.set<XPBDCloth3DVertices>(XPBDCloth3DVertices());
                    }
                    if (!entity.has<XPBDCloth3DTriangleIndices>()) {
                        entity.set<XPBDCloth3DTriangleIndices>(XPBDCloth3DTriangleIndices());
                    }
                    if (!entity.has<XPBDCloth3DEdgeIndices>()) {
                        entity.set<XPBDCloth3DEdgeIndices>(XPBDCloth3DEdgeIndices());
                    }
                }
            });

        // clang-format off
        world.system<
                XPBDCloth3DState,
                XPBDCloth3DVertices,
                XPBDCloth3DTriangleIndices,
                XPBDCloth3DEdgeIndices,
                const XPBDCloth3DConfig,
                const XPBDCloth3DGrab,
                const PhysicsBodyType,
                const transform::Position3D
            >("stagehand::physics::XPBD3D Simulation")
            .kind(flecs::OnUpdate)
            .run([](flecs::iter &it) {
                // clang-format on
                const float frame_dt = std::max(static_cast<float>(it.delta_time()), 1.0f / 60.0f);

                while (it.next()) {
                    auto state_field = it.field<XPBDCloth3DState>(0);
                    auto vertices_field = it.field<XPBDCloth3DVertices>(1);
                    auto triangles_field = it.field<XPBDCloth3DTriangleIndices>(2);
                    auto edges_field = it.field<XPBDCloth3DEdgeIndices>(3);
                    auto config_field = it.field<const XPBDCloth3DConfig>(4);
                    auto grab_field = it.field<const XPBDCloth3DGrab>(5);
                    auto body_type_field = it.field<const PhysicsBodyType>(6);
                    auto position_field = it.field<const transform::Position3D>(7);

                    for (auto i : it) {
                        if (!internal::is_xpbd3d_body(body_type_field[i])) {
                            continue;
                        }

                        flecs::entity entity = it.entity(i);
                        if (entity.has(flecs::Prefab)) {
                            continue;
                        }

                        XPBDCloth3DState &state = state_field[i];
                        XPBDCloth3DVertices &vertices = vertices_field[i];
                        XPBDCloth3DTriangleIndices &triangles = triangles_field[i];
                        XPBDCloth3DEdgeIndices &edges = edges_field[i];
                        const XPBDCloth3DConfig config = internal::sanitize_config(config_field[i]);
                        const XPBDCloth3DGrab &grab = grab_field[i];

                        if (internal::requires_rebuild(state, config)) {
                            internal::initialize_state(state, config, position_field[i]);
                            internal::build_visual_indices(config.num_x, config.num_y, triangles, edges);
                            internal::update_output_vertices(state.pos, vertices);
                        }

                        internal::simulate(state, config, grab, frame_dt);
                        internal::update_output_vertices(state.pos, vertices);
                    }
                }
            });
    });

} // namespace stagehand::physics
