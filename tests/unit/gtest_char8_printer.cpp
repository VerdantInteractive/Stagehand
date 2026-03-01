// Provide a PrintTo overload for char8_t to avoid implicit conversions to char32_t
// inside GoogleTest printers which trigger -Wcharacter-conversion warnings on some compilers.
#include <cstdint>
#include <ostream>

void PrintTo(char8_t c, ::std::ostream *os) {
    // Print the numeric code unit value to avoid issues with streaming char8_t directly.
    // Use an unsigned integer for clarity.
    uint32_t v = static_cast<uint32_t>(c);
    *os << "char8_t(" << v << ")";
}
