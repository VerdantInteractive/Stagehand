#pragma once

namespace utilities
{
    /// Platform-level utility functions.
    class Platform
    {
    public:
        /// Returns the number of threads to use for the ECS, based on hardware concurrency.
        static unsigned int get_thread_count();
    };
}
