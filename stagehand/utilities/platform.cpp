#include <thread>

#include "stagehand/utilities/platform.h"

unsigned int utilities::Platform::get_thread_count()
{
    return std::min(std::max(std::thread::hardware_concurrency() - 1, 1U), 64U);
}
