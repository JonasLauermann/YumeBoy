#pragma once

#include <cstdint>
#include <utility>


enum class CPU_STATES : uint16_t {
    FetchOpcode         =       1,
    FetchExtOpcode      =  1 << 1,
    Execute             =  1 << 2,
    InterruptNOP1       =  1 << 3,  // https://gbdev.io/pandocs/Interrupts.html#interrupt-handling
    InterruptNOP2       =  1 << 4,
    InterruptPushPC1    =  1 << 5,
    InterruptPushPC2    =  1 << 6,
    InterruptSetPC      =  1 << 7,
    HaltMode            =  1 << 8,
    StopMode            =  1 << 9,

    Interruptable       = FetchOpcode | HaltMode | StopMode,
    Interrupting        = InterruptNOP1 | InterruptNOP2 | InterruptPushPC1 | InterruptPushPC2 | InterruptSetPC,
};


constexpr CPU_STATES operator&(CPU_STATES x, CPU_STATES y)
{
    return static_cast<CPU_STATES>(std::to_underlying(x) & std::to_underlying(y));
}

constexpr CPU_STATES operator|(CPU_STATES x, CPU_STATES y)
{
    return static_cast<CPU_STATES>(std::to_underlying(x) | std::to_underlying(y));
}

// Single templated spaceship operator for all comparisons (enum-to-int, int-to-enum, and enum-to-enum)
template <typename T, typename U>
constexpr std::strong_ordering operator<=>(T lhs, U rhs) requires
    (std::is_same_v<T, CPU_STATES> && std::is_integral_v<U>) ||    // Enum vs Int
    (std::is_integral_v<T> && std::is_same_v<U, CPU_STATES>) ||    // Int vs Enum
    (std::is_same_v<T, CPU_STATES> && std::is_same_v<U, CPU_STATES>)   // Enum vs Enum
{
    if constexpr (std::is_same_v<T, CPU_STATES> && std::is_same_v<U, CPU_STATES>) {
        // Enum-to-Enum comparison
        return std::to_underlying(lhs) <=> std::to_underlying(rhs);
    } else if constexpr (std::is_same_v<T, CPU_STATES>) {
        // Enum-to-Int comparison
        return std::to_underlying(lhs) <=> rhs;
    } else {
        // Int-to-Enum comparison
        return lhs <=> std::to_underlying(rhs);
    }
}

template <typename T, typename U>
constexpr bool operator!=(T lhs, U rhs) requires
    (std::is_same_v<T, CPU_STATES> && std::is_integral_v<U>) ||    // Enum vs Int
    (std::is_integral_v<T> && std::is_same_v<U, CPU_STATES>) ||    // Int vs Enum
    (std::is_same_v<T, CPU_STATES> && std::is_same_v<U, CPU_STATES>)   // Enum vs Enum
{
    if constexpr (std::is_same_v<T, CPU_STATES> && std::is_same_v<U, CPU_STATES>) {
        // Enum-to-Enum comparison
        return std::to_underlying(lhs) != std::to_underlying(rhs);
    } else if constexpr (std::is_same_v<T, CPU_STATES>) {
        // Enum-to-Int comparison
        return std::to_underlying(lhs) != rhs;
    } else {
        // Int-to-Enum comparison
        return lhs != std::to_underlying(rhs);
    }
}