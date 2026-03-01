#include <iostream>
#include <flecs.h>

int main() {
    // Create a Flecs world
    flecs::world world;

    // Create a simple system that prints whenever it runs
    world.system<>()
        .run([](flecs::iter &it) {
            std::cout << "[system] executed" << std::endl;
        });

    // Get pipeline entity and print its initial state
    flecs::entity pipeline = world.get_pipeline();
    std::cout << "pipeline id: " << pipeline.id() << ", valid: " << pipeline.is_valid()
              << ", enabled: " << pipeline.enabled() << std::endl;

    // First progress call: system should run
    std::cout << "--- First progress() ---" << std::endl;
    world.progress();

    // Disable the pipeline entity
    std::cout << "Disabling pipeline (pipeline.disable())" << std::endl;
    pipeline.disable();

    std::cout << "pipeline enabled after disable(): " << pipeline.enabled() << std::endl;

    // Subsequent progress calls: check whether system still runs
    std::cout << "--- Second progress() ---" << std::endl;
    world.progress();

    std::cout << "--- Third progress() ---" << std::endl;
    world.progress();
    
    // Re-enable the pipeline entity so systems can run again
    std::cout << "Re-enabling pipeline (pipeline.enable())" << std::endl;
    pipeline.enable();
    std::cout << "pipeline enabled after enable(): " << pipeline.enabled() << std::endl;

    std::cout << "--- Fourth progress() ---" << std::endl;
    world.progress();

    std::cout << "Done." << std::endl;
    return 0;
}
