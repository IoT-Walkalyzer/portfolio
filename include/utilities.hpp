#ifndef PRESSUREPLATE_UTILITIES_HPP
#define PRESSUREPLATE_UTILITIES_HPP

// Includes
#include <array>
#include <type_traits>
#include <cstdint>

// Methods
template<typename Input = int>
std::array<char, sizeof(Input)> integral_to_bytes(const Input& input) {
    constexpr uint8_t size = sizeof(Input);
    std::array<char, size> bytes;

    for (uint8_t i = 0; i < (uint8_t)size; ++i)
        bytes[(size - 1) - i] = (char)((input >> (i * 8)) & 0xFF);
    return bytes;
}

template<typename Output = int>
Output bytes_to_integral(const std::array<char, sizeof(Output)>& bytes) {
    Output output;

    for (char byte : bytes)
        output = (output << 8) | ((Output)byte & 0xFF);
    return output;
}

#endif
