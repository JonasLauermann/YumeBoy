#pragma once

#include <cstdint>
#include <utility>


enum class PPU_STATES : uint8_t {
    HBlank              =  0,
    VBlank              =  1,
    OAMScan             =  2,
    PixelTransfer       =  3,
};

/* https://hacktix.github.io/GBEDG/ppu/#the-pixel-fifo */
enum class FETCHER_STATES : uint16_t {  
    FetchBGTileNo             =       1,
    FetchBGTileDataLow        =  1 << 1,
    FetchBGTileDataHigh       =  1 << 2,
    PushToBGFIFO              =  1 << 3,
    
    FetchSpriteTileNo         =  1 << 4,
    FetchSpriteTileDataLow    =  1 << 5,
    FetchSpriteTileDataHigh   =  1 << 6,
    PushToSpriteFIFO          =  1 << 7,

    FetchingBGTile = FetchBGTileNo | FetchBGTileDataLow | FetchBGTileDataHigh | PushToBGFIFO,
    FetchingSpriteTile = FetchSpriteTileNo | FetchSpriteTileDataLow | FetchSpriteTileDataHigh | PushToSpriteFIFO,
};


constexpr FETCHER_STATES operator&(FETCHER_STATES x, FETCHER_STATES y)
{
    return static_cast<FETCHER_STATES>(std::to_underlying(x) & std::to_underlying(y));
}

constexpr FETCHER_STATES operator|(FETCHER_STATES x, FETCHER_STATES y)
{
    return static_cast<FETCHER_STATES>(std::to_underlying(x) | std::to_underlying(y));
}

template <typename T, typename U>
constexpr bool operator!=(T lhs, U rhs) requires
    (std::is_same_v<T, FETCHER_STATES> && std::is_integral_v<U>) ||    // Enum vs Int
    (std::is_integral_v<T> && std::is_same_v<U, FETCHER_STATES>) ||    // Int vs Enum
    (std::is_same_v<T, FETCHER_STATES> && std::is_same_v<U, FETCHER_STATES>)   // Enum vs Enum
{
    if constexpr (std::is_same_v<T, FETCHER_STATES> && std::is_same_v<U, FETCHER_STATES>) {
        // Enum-to-Enum comparison
        return std::to_underlying(lhs) != std::to_underlying(rhs);
    } else if constexpr (std::is_same_v<T, FETCHER_STATES>) {
        // Enum-to-Int comparison
        return std::to_underlying(lhs) != rhs;
    } else {
        // Int-to-Enum comparison
        return lhs != std::to_underlying(rhs);
    }
}