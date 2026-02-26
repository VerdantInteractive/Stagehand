#pragma once

#include <algorithm>

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "stagehand/ecs/components/scene_children.h"
#include "stagehand/ecs/components/world_configuration.h"
#include "stagehand/registry.h"

#include "components.h"
#include "prefabs.h"

using namespace game_of_life;

REGISTER_IN_MODULE(game_of_life::systems, [](flecs::world &world) {
    // This shares a common value between the initialisation and rendering systems and also avoids the overhead of recalculating the size
    // or looking up the configuration every frame in the rendering system, effectively treating it as a runtime constant once initialized.
    const auto grid_population = std::make_shared<godot::Vector2i>();

    world.system<const stagehand::WorldConfiguration>("Grid Initialization").kind(flecs::OnStart).run([grid_population](flecs::iter &it) {
        if (it.next()) {
            auto world_config = it.field<const stagehand::WorldConfiguration>(0)->value;
            const double population_scale = std::clamp(static_cast<double>(world_config.get("population_scale", 1.0)), 0.1, 2.0);
            *grid_population = godot::Vector2(480, 270) * population_scale;
            const double initial_alive_percentage = std::clamp(static_cast<double>(world_config.get("initial_alive_percentage", 0.2)), 0.05, 0.5);

            std::vector<flecs::entity> cells(grid_population->x * grid_population->y);

            // Create cells
            for (int i = 0; i < grid_population->x * grid_population->y; ++i) {
                cells[i] = it.world().entity().is_a(Cell);
            }

            // Populate neighbour references for each cell, using modulo to wrap around the grid (toroidal array).
            for (int y = 0; y < grid_population->y; ++y) {
                for (int x = 0; x < grid_population->x; ++x) {
                    flecs::entity cell_entity = cells[y * grid_population->x + x];

                    static const int offsets[8][2] = {{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};

                    GridNeighbours neighbours;
                    for (int i = 0; i < 8; ++i) {
                        const int neighbour_x = (x + offsets[i][0] + grid_population->x) % grid_population->x;
                        const int neighbour_y = (y + offsets[i][1] + grid_population->y) % grid_population->y;
                        neighbours.value[i] = cells[neighbour_y * grid_population->x + neighbour_x].id();
                    }
                    cell_entity.set<GridNeighbours>(neighbours);
                    cell_entity.set<GridPosition>({x, y});

                    if (godot::UtilityFunctions::randf() < initial_alive_percentage) {
                        cell_entity.add<IsAlive>();
                    }
                }
            }
            godot::UtilityFunctions::print(godot::String("Grid Initialization: Initialized a grid of ") +
                                           godot::String::num_int64(grid_population->x * grid_population->y) + " cells (" +
                                           godot::String::num_int64(grid_population->x) + " x " + godot::String::num_int64(grid_population->y) + ").");
        }
    });

    // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
    world.system<const AliveNeighbourCount>("Birth").with<IsInActiveNeighbourhood>().without<IsAlive>().each(
        [](flecs::entity cell_entity, const AliveNeighbourCount &alive_neighbour_count) {
            if (alive_neighbour_count.value == 3) {
                cell_entity.add<IsAlive>();
            }
        });

    // Any live cell with fewer than two / more than three live neighbours dies, as if by under / overpopulation.
    world.system<const AliveNeighbourCount>("Death").with<IsInActiveNeighbourhood>().with<IsAlive>().each(
        [](flecs::entity cell_entity, const AliveNeighbourCount &alive_neighbour_count) {
            if (alive_neighbour_count.value < 2 || alive_neighbour_count.value > 3) {
                cell_entity.remove<IsAlive>();
            }
        });

    world.system<const GridNeighbours>("Births Post-Processing")
        .kind(flecs::OnValidate)
        .with<IsAlive>()
        .without<WasAlive>()
        .each([](flecs::entity entity, const GridNeighbours &neighbours) {
            entity.add<WasAlive>();
            entity.add<IsInActiveNeighbourhood>();
            flecs::world world = entity.world();
            for (const ecs_entity_t neighbour_id : neighbours.value) {
                flecs::entity neighbour = world.entity(neighbour_id);
                neighbour.get_mut<AliveNeighbourCount>().value++;
                neighbour.add<IsInActiveNeighbourhood>();
            }
        });

    world.system<const GridNeighbours>("Deaths Post-Processing")
        .kind(flecs::OnValidate)
        .without<IsAlive>()
        .with<WasAlive>()
        .each([](flecs::entity entity, const GridNeighbours &neighbours) {
            entity.remove<WasAlive>();
            entity.add<IsInActiveNeighbourhood>();
            flecs::world world = entity.world();
            for (const ecs_entity_t neighbour_id : neighbours.value) {
                flecs::entity neighbour = world.entity(neighbour_id);
                neighbour.get_mut<AliveNeighbourCount>().value--;
                neighbour.add<IsInActiveNeighbourhood>();
            }
        });

    world.system<const AliveNeighbourCount>("Neighbourhood cleanup")
        .kind(flecs::PostUpdate)
        .with<IsInActiveNeighbourhood>()
        .without<IsAlive>()
        .each([](flecs::entity cell_entity, const AliveNeighbourCount &alive_neighbour_count) {
            if (alive_neighbour_count.value == 0) {
                cell_entity.remove<IsInActiveNeighbourhood>();
            }
        });

    // Query for alive cells to avoid iterating dead ones. With caching we avoid recreating it every frame.
    auto alive_cells_query = world.query_builder<const GridPosition>().with<IsAlive>().cached().build();

    world.system<const stagehand::WorldConfiguration, const stagehand::SceneChildren>("Rendering")
        .kind(flecs::OnStore)
        .run([alive_cells_query, grid_population](flecs::iter &it) {
            while (it.next()) {
                flecs::field<const stagehand::SceneChildren> scene_children_field = it.field<const stagehand::SceneChildren>(1);

                // Get the GridView node from the scene children dictionary
                const stagehand::SceneChildren &scene_children = *scene_children_field;
                godot::Variant grid_view_variant = scene_children["GridView"];
                if (grid_view_variant.get_type() == godot::Variant::Type::NIL)
                    continue;

                godot::Node *grid_view_node = godot::Object::cast_to<godot::Node>(grid_view_variant);
                if (!grid_view_node)
                    continue;

                // Cast to TextureRect
                godot::TextureRect *texture_rect = godot::Object::cast_to<godot::TextureRect>(grid_view_node);
                if (!texture_rect)
                    continue;

                // Get the ImageTexture
                godot::Ref<godot::ImageTexture> image_texture = texture_rect->get_texture();
                if (image_texture.is_null()) {
                    godot::Ref<godot::Image> image = godot::Image::create(grid_population->x, grid_population->y, false, godot::Image::FORMAT_R8);
                    image_texture = godot::ImageTexture::create_from_image(image);
                    texture_rect->set_texture(image_texture);
                }

                // Get the Image
                godot::Ref<godot::Image> image = image_texture->get_image();
                if (image.is_null())
                    continue;

                // Optimization: Clear image to background color first (fast), then only draw alive cells.
                image->fill(godot::Color(0.0f, 0.0f, 0.0f, 1.0f));

                const godot::Color alive_color(1.0f, 0.0f, 0.0f, 1.0f);
                alive_cells_query.each([&](const GridPosition &position) { image->set_pixel(position.x, position.y, alive_color); });

                // Update the texture with the modified image
                image_texture->update(image);
            }
        });
});
