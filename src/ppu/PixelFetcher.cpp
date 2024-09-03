#include "ppu/PixelFetcher.hpp"

#include "ppu/PPU.hpp"


void PixelFetcher::tick()
{
    switch (state)
    {
    case FETCHER_STATES::FetchBGTileNo: {
        // determine if a window or BG tile should be fetched
        fetch_window = p.LCDC & 1 << 5 and p.WY <= p.LY and p.WX <= fetcher_x;
        // determine which tile-map is in use based on LCDC bit 3 or 6 depending on if a window or BG tile is fetched
        uint16_t tile_map_addr = p.LCDC & 1 << (3 + 3 * int(fetch_window)) ? 0x9C00 : 0x9800;

        uint8_t x = fetch_window ? fetcher_x - ((p.WX - 7) / 8) : ((p.SCX / 8) + fetcher_x) & 0x1F;
        uint8_t y = fetch_window ? p.LY - (p.WY / 8) : (((p.LY + p.SCY) & 0xFF) / 8);
        assert(x < 32 and y < 32);

        uint16_t tile_id_addr = tile_map_addr + x + (y * 0x20);
        tile_id = p.vram_[tile_id_addr - VRAM_BEGIN];

        state = FETCHER_STATES::FetchBGTileDataLow;
        break;
    }
    
    case FETCHER_STATES::FetchBGTileDataLow: {
        // determine which tile-map is in use based on LCDC bit 4
        uint16_t tile_data_addr = p.LCDC & 1 << 4 ? uint16_t(0x8000 + (tile_id << 4)) : uint16_t(0x9000 + (tile_id << 4));  // TODO shouldn't tile_id be signed for 9000-mode?
        uint8_t line_offset = fetch_window ? ((p.LY - p.WY) % 8) * 2 : ((p.LY + p.SCY) % 8) * 2;
        low_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset];

        state = FETCHER_STATES::FetchBGTileDataHigh;
        break;
    }

    case FETCHER_STATES::FetchBGTileDataHigh: {
        // determine which tile-map is in use based on LCDC bit 4
        uint16_t tile_data_addr = p.LCDC & 1 << 4 ? uint16_t(0x8000 + (tile_id << 4)) : uint16_t(0x9000 + (tile_id << 4));  // TODO shouldn't tile_id be signed for 9000-mode?
        uint8_t line_offset = fetch_window ? ((p.LY - p.WY) % 8) * 2 : ((p.LY + p.SCY) % 8) * 2;
        high_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset + 1];

        state = FETCHER_STATES::PushToBGFIFO;
        break;
    }

    case FETCHER_STATES::PushToBGFIFO: {
        // check if fifo is empty
        if (not p.BG_FIFO.empty())
        {
            // stay idle for now
            break;
        }

        for (int i = 7; i >= 0; --i)
        {
            uint8_t color = (((high_data << 1) >> i) & 0b10) | ((low_data >> i) & 0b1);
            p.BG_FIFO.emplace(color, BG, false);
        }

        ++fetcher_x; // increment internal x
        state = FETCHER_STATES::FetchBGTileNo;
        break;
    }

    case FETCHER_STATES::FetchSpriteTileNo: {
        tile_id = oam_entry->tile_id;

        state = FETCHER_STATES::FetchSpriteTileDataLow;
        break;
    }

    case FETCHER_STATES::FetchSpriteTileDataLow: {
        auto tile_data_addr = uint16_t(0x8000 + (tile_id << 4));
        uint8_t line_offset = oam_entry->flags & (1 << 5) ? (7 - ((p.LY + oam_entry->y) % 8)) * 2 : ((p.LY + oam_entry->y) % 8) * 2;
        low_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset];

        state = FETCHER_STATES::FetchSpriteTileDataHigh;
        break;
    }

    case FETCHER_STATES::FetchSpriteTileDataHigh: {
        auto tile_data_addr = uint16_t(0x8000 + (tile_id << 4));
        uint8_t line_offset = oam_entry->flags & 1 << 5 ? (7 - ((p.LY + oam_entry->y) % 8)) * 2 : ((p.LY + oam_entry->y) % 8) * 2;
        high_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset + 1];

        state = FETCHER_STATES::PushToSpriteFIFO;
        break;
    }

    case FETCHER_STATES::PushToSpriteFIFO: {
        for (int i = 0; i < 8; ++i)
        {
            // flip pixels vertically if flag is set
            int j = oam_entry->flags & (1 << 6) ? i : 7 - i;
            assert(j >= 0);
            uint8_t color = (((high_data << 1) >> j) & 0b10) | ((low_data >> j) & 0b1);
            ColorPallet pallet = oam_entry->flags & 1 << 4 ? S1 : S0;

            // skip pixel if another sprite already occupies the pixel or if it is off-screen
            if (p.Sprite_FIFO.size() > i or oam_entry->x + i < 8)
                continue;
            p.Sprite_FIFO.emplace(color, pallet, oam_entry->flags & 1 << 7);
        }

        // set mode back to BG/Window fetching
        pixel_fifo_stopped = false;
        state = FETCHER_STATES::FetchBGTileNo;

        break;
    }
    
    default:
        std::unreachable();
    }
}

void PixelFetcher::fetch_sprite(std::unique_ptr<OAM_entry> entry)
{
    assert((state & FETCHER_STATES::FetchingBGTile) != 0 and entry);
    state = FETCHER_STATES::FetchSpriteTileNo;
    oam_entry = std::move(entry);
    pixel_fifo_stopped = true;
}

void PixelFetcher::reset()
{
    state = FETCHER_STATES::FetchBGTileNo;
    fetcher_x = 0;
    fetch_window = false;
    oam_entry = nullptr;
    tile_id = 0;
    low_data = 0;
    high_data = 0;
    pixel_fifo_stopped = false;
}
