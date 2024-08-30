#include "PPU.hpp"

#include "YumeBoy.hpp"

void PPU::LCD::push_pixel(Color c)
{
    assert(buffer_it != pixel_buffer.end());

    switch (c)
    {
    case Color::WHITE:
        *buffer_it++ = 233;
        *buffer_it++ = 239;
        *buffer_it++ = 236;
        *buffer_it++ = 255;
        break;

    case Color::LIGHT_GRAY:
        *buffer_it++ = 160;
        *buffer_it++ = 160;
        *buffer_it++ = 139;
        *buffer_it++ = 255;
        break;

    case Color::DARK_GRAY:
        *buffer_it++ = 85;
        *buffer_it++ = 85;
        *buffer_it++ = 104;
        *buffer_it++ = 255;
        break;

    case Color::BLACK:
        *buffer_it++ = 33;
        *buffer_it++ = 30;
        *buffer_it++ = 32;
        *buffer_it++ = 255;
        break;

    default:
        std::unreachable();
    }
}

void PPU::LCD::update_screen()
{
    assert(buffer_it == pixel_buffer.end());

    if (power_) [[likely]]
    {
        SDL_UpdateTexture(pixel_matrix_texture.get(), nullptr, pixel_buffer.data(), DISPLAY_WIDTH * sizeof(uint8_t) * 4);
    }
    else
    {
        SDL_UpdateTexture(pixel_matrix_texture.get(), nullptr, std::vector<uint8_t>(255).data(), DISPLAY_WIDTH * sizeof(uint8_t) * 4);
    }
    SDL_RenderCopy(renderer.get(), pixel_matrix_texture.get(), nullptr, nullptr);
    SDL_RenderPresent(renderer.get());

    buffer_it = pixel_buffer.begin();
}

void PPU::set_mode(PPU_Mode mode)
{
    switch (mode_)
    {
    case H_Blank:
    {
        assert(mode == V_Blank or mode == OAM_Scan);
        break;
    }
    case V_Blank:
    {
        assert(mode == OAM_Scan);
        break;
    }
    case OAM_Scan:
    {
        assert(mode == Pixel_Transfer);
        break;
    }
    case Pixel_Transfer:
    {
        assert(mode == H_Blank);
        break;
    }
    default:
        throw std::runtime_error("Unknown PPU mode");
    }
    mode_ = mode;
    STAT = (STAT & 0b11111100) | mode;
}

void PPU::PixelFetcher::bg_tick()
{
    switch (step++)
    {
    case 0:
    { // Fetch Tile Number
        // determine if a window or BG tile should be fetched
        fetch_window = p.LCDC & 1 << 5 and p.WY <= p.LY and p.WX <= fetcher_x;
        // determine which tile-map is in use based on LCDC bit 3 or 6 depending on if a window or BG tile is fetched
        uint16_t tile_map_addr = p.LCDC & 1 << (3 + 3 * int(fetch_window)) ? 0x9C00 : 0x9800;

        uint8_t x = fetch_window ? fetcher_x - ((p.WX - 7) / 8) : ((p.SCX / 8) + fetcher_x) & 0x1F;
        uint8_t y = fetch_window ? p.LY - (p.WY / 8) : (((p.LY + p.SCY) & 0xFF) / 8);
        assert(x < 32 and y < 32);

        uint16_t tile_id_addr = tile_map_addr + x + (y * 0x20);
        tile_id = p.vram_[tile_id_addr - VRAM_BEGIN];
        break;
    }
    case 1:
    { // Fetch Tile Data (Low)
        // determine which tile-map is in use based on LCDC bit 4
        uint16_t tile_data_addr = p.LCDC & 1 << 4 ? uint16_t(0x8000 + (tile_id << 4)) : uint16_t(0x9000 + (tile_id << 4));  // TODO shouldn't tile_id be signed for 9000-mode?
        uint8_t line_offset = fetch_window ? ((p.LY - p.WY) % 8) * 2 : ((p.LY + p.SCY) % 8) * 2;
        low_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset];
        break;
    }
    case 2:
    { // Fetch Tile Data (High)
        // determine which tile-map is in use based on LCDC bit 4
        uint16_t tile_data_addr = p.LCDC & 1 << 4 ? uint16_t(0x8000 + (tile_id << 4)) : uint16_t(0x9000 + (tile_id << 4));  // TODO shouldn't tile_id be signed for 9000-mode?
        uint8_t line_offset = fetch_window ? ((p.LY - p.WY) % 8) * 2 : ((p.LY + p.SCY) % 8) * 2;
        high_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset + 1];
        break;
    }
    case 3:
    { // Push to FIFO (if empty)
        // check if fifo is empty
        if (not p.BG_FIFO.empty())
        {
            // stay idle for now
            --step;
            return;
        }

        for (int i = 7; i >= 0; --i)
        {
            uint8_t color = (((high_data << 1) >> i) & 0b10) | ((low_data >> i) & 0b1);
            p.BG_FIFO.emplace(color, BG, false);
        }

        ++fetcher_x; // increment internal x
        break;
    }
    default:
        std::unreachable();
    }
    step %= 4;
}

void PPU::PixelFetcher::sprite_tick()
{
    // TODO: consider 8x16 sprites
    switch (step++)
    {
    case 0:
    { // Fetch Tile Number
        tile_id = oam_entry.tile_id;
        break;
    }
    case 1:
    { // Fetch Tile Data (Low)
        auto tile_data_addr = uint16_t(0x8000 + (tile_id << 4));
        uint8_t line_offset = oam_entry.flags & 1 << 5 ? (7 - ((p.LY + oam_entry.y) % 8)) * 2 : ((p.LY + oam_entry.y) % 8) * 2;
        assert(line_offset < 8);
        low_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset];
        break;
    }
    case 2:
    { // Fetch Tile Data (High)
        auto tile_data_addr = uint16_t(0x8000 + (tile_id << 4));
        uint8_t line_offset = oam_entry.flags & 1 << 5 ? (7 - ((p.LY + oam_entry.y) % 8)) * 2 : ((p.LY + oam_entry.y) % 8) * 2;
        assert(line_offset < 8);
        high_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset + 1];
        break;
    }
    case 3:
    { // Push to FIFO (skip pixel if already filled)
        for (int i = 0; i < 8; ++i)
        {
            // flip pixels vertically if flag is set
            int j = oam_entry.flags & (1 << 6) ? i : 7 - i;
            assert(j >= 0);
            uint8_t color = (((high_data << 1) >> j) & 0b10) | ((low_data >> j) & 0b1);
            ColorPallet pallet = oam_entry.flags & 1 << 4 ? S1 : S0;

            // skip pixel if another sprite already occupies the pixel or if it is off-screen
            if (p.Sprite_FIFO.size() > i or oam_entry.x + i < 8)
                continue;
            p.Sprite_FIFO.emplace(color, pallet, oam_entry.flags & 1 << 7);
        }

        // set mode back to BG/Window fetching
        assert(step == 4);
        step = 0;
        fetch_sprite_ = false;
        p.pixel_fifo_stopped = false;

        break;
    }
    default:
        std::unreachable();
    }
    step %= 4;
}

void PPU::PixelFetcher::fetch_sprite(OAM_entry entry)
{
    assert(not fetch_sprite_);
    // reset fetcher steps and switch mode
    oam_entry = entry;
    step = 0;
    fetch_sprite_ = true;
    p.pixel_fifo_stopped = true;
}

void PPU::PixelFetcher::reset()
{
    fetcher_x = 0;
    step = 0;
    fetch_sprite_ = false;
    fetch_window = false;
}

void PPU::next_scanline()
{
    ++LY;
    if (LYC == LY) {
        STAT |= 1 << 2;
    } else {
        STAT &= 0b11111011;
    }
}

bool PPU::STATE_interrupt_signal()
{
    // LYC == LY interrupt
    if ((STAT & (1 << 6)) and (STAT & (1 << 2))) [[unlikely]]
        return true;
    
    // Mode 2 interrupt
    if ((STAT & (1 << 5)) and (STAT & 0b11) == 2) [[unlikely]]
        return true;
    
    // Mode 1 interrupt
    if ((STAT & (1 << 4)) and (STAT & 0b11) == 1) [[unlikely]]
        return true;
    
    // Mode 0 interrupt
    if ((STAT & (1 << 3)) and (STAT & 0b11) == 0) [[unlikely]]
        return true;
    
    return false;
}

void PPU::OAM_transfer()
{
    // https://hacktix.github.io/GBEDG/dma/
    // TODO currently OAM transfer is instant => include t_cycles

    for (uint8_t lower_addr = 0; lower_addr <= 0x9F; ++lower_addr) {
        uint16_t source_addr = uint16_t((DMA << 8) | lower_addr);
        uint16_t dest_addr = 0xFE00 | lower_addr;

        uint8_t value = yume_boy_.read_memory(source_addr);
        yume_boy_.write_memory(dest_addr, value);
    }
}

uint8_t PPU::read_vram(uint16_t addr)
{
    assert(VRAM_BEGIN <= addr and addr <= VRAM_END);
    // VRAM is inaccessible during pixel transfer
    if (mode_ == Pixel_Transfer)
        return 0xFF;
    return vram_[addr - VRAM_BEGIN];
}

void PPU::write_vram(uint16_t addr, uint8_t value)
{
    assert(VRAM_BEGIN <= addr and addr <= VRAM_END);
    // VRAM is inaccessible during pixel transfer
    if (mode_ == Pixel_Transfer)
        return;
    vram_[addr - VRAM_BEGIN] = value;
}

uint8_t PPU::read_oam_ram(uint16_t addr)
{
    assert(OAM_RAM_BEGIN <= addr and addr <= OAM_RAM_END);
    // OAM RAM is inaccessible during OAM scan and pixel transfer
    if (mode_ == Pixel_Transfer or mode_ == OAM_Scan)
        return 0xFF;
    return oam_ram_[addr - OAM_RAM_BEGIN];
}

void PPU::write_oam_ram(uint16_t addr, uint8_t value)
{
    assert(OAM_RAM_BEGIN <= addr and addr <= OAM_RAM_END);
    // OAM RAM is inaccessible during OAM scan and pixel transfer
    if (mode_ == Pixel_Transfer or mode_ == OAM_Scan)
        return;
    oam_ram_[addr - OAM_RAM_BEGIN] = value;
}

uint8_t PPU::read_lcd_register(uint16_t addr) const
{
    assert(LCD_REG_BEGIN <= addr and addr <= LCD_REG_END);
    switch (addr)
    {
    case 0xFF40:
        return LCDC;
    case 0xFF41:
        return STAT;
    case 0xFF42:
        return SCY;
    case 0xFF43:
        return SCX;
    case 0xFF44:
        return LY;
    case 0xFF45:
        return LYC;
    case 0xFF46:
        return DMA;
    case 0xFF47:
        return BGP;
    case 0xFF48:
        return OBP0;
    case 0xFF49:
        return OBP1;
    case 0xFF4A:
        return WY;
    case 0xFF4B:
        return WX;
    default:
        throw std::runtime_error("Unknown LCD register address");
    }
}

void PPU::write_lcd_register(uint16_t addr, uint8_t value)
{
    assert(LCD_REG_BEGIN <= addr and addr <= LCD_REG_END);
    switch (addr)
    {
    case 0xFF40:
        lcd.power(value & 1 << 7);
        LCDC = value;
        break;
    case 0xFF41:
        STAT = (value & 0b11111000) | (STAT & 0b111); // Bits 0-3 are not writable
        break;
    case 0xFF42:
        SCY = value;
        break;
    case 0xFF43:
        SCX = value;
        break;
    case 0xFF44:
        // LY is not writeable
        break;
    case 0xFF45:
        LYC = value;
        break;
    case 0xFF46:
        DMA = value;
        OAM_transfer();
        break;
    case 0xFF47:
        BGP = value;
        break;
    case 0xFF48:
        OBP0 = value;
        break;
    case 0xFF49:
        OBP1 = value;
        break;
    case 0xFF4A:
        WY = value;
        break;
    case 0xFF4B:
        WX = value;
        break;
    default:
        throw std::runtime_error("Unknown LCD register address");
    }
}

void PPU::h_blank_tick()
{
    dot();
    if (scanline_time_ == 456)
    {
        /* Move to next scanline and switch mode */
        next_scanline();
        if (LY == 144)
        {
            set_mode(V_Blank);
            yume_boy_.request_interrupt(YumeBoy::INTERRUPT::V_BLANK_INTERRUPT);
            lcd.update_screen();
        }
        else
        {
            assert(LY < 144);
            set_mode(OAM_Scan);
        }
        scanline_time_ = 0;
        fifo_pushed_pixels = 0;
    }
}

void PPU::v_blank_tick()
{
    dot();
    if (scanline_time_ == 456)
    {
        /* Move to next scanline and switch mode if necessary */
        assert(144 <= LY and LY <= 153);
        next_scanline();
        if (LY == 154)
        {
            LY = 0;
            set_mode(OAM_Scan);
        }
        scanline_time_ = 0;
    }
}

void PPU::oam_scan_tick()
{
    /* Scan the OAM RAM. The CPU has no access to the OAM RAM in this mode. */
    if (oam_pointer == OAM_RAM_BEGIN)
        scanline_sprites.clear();

    dot();
    OAM_entry e{
        oam_ram_[oam_pointer - OAM_RAM_BEGIN],
        oam_ram_[oam_pointer - OAM_RAM_BEGIN + 1],
        oam_ram_[oam_pointer - OAM_RAM_BEGIN + 2],
        oam_ram_[oam_pointer - OAM_RAM_BEGIN + 3]};
    oam_pointer += 4;

    // check if the OAM entry is visible on the current scanline
    if ((LY + 16 >= e.y and e.y + 8 + (8 * ((LCDC >> 2) & 1)) > LY + 16) and (scanline_sprites.size() < 10))
    {
        scanline_sprites.push_back(e);
    }

    if (oam_pointer == OAM_RAM_END + 1)
    {
        assert(scanline_time_ == 40);
        set_mode(Pixel_Transfer);
        oam_pointer = OAM_RAM_BEGIN;
    }
}

void PPU::pixel_transfer_tick()
{
    dot();

    // the pixel fetcher is two times slower than the rest of the ppu
    if (scanline_time_ % 2 == 0)
        fetcher.tick();

    // stop if the pixel fifo is empty or stopped
    if (BG_FIFO.empty() or pixel_fifo_stopped)
        return;

    // if a sprite is at the current x position, switch the fetcher into sprite fetch mode
    if (auto it = std::find_if(
            scanline_sprites.begin(),
            scanline_sprites.end(),
            [&](OAM_entry e)
            { return e.x <= fifo_pushed_pixels + 9; }); // not entirely sure why we have to offset by 9, seems like a bug elsewhere
        it != scanline_sprites.end())
    {
        fetcher.fetch_sprite(*it);
        scanline_sprites.erase(it);
    }

    // if scanline is at the beginning, skip pixels based on SCX offset
    if (fifo_pushed_pixels == 0)
    {
        for (int i = 0; i < SCX % 8; ++i)
            BG_FIFO.pop();
    }

    Pixel px = BG_FIFO.front();
    BG_FIFO.pop();
    assert((px.color & 0b11) == px.color);

    // merge the BG and sprite pixels if a sprite pixel is available
    if (not Sprite_FIFO.empty())
    {
        Pixel sp = Sprite_FIFO.front();
        Sprite_FIFO.pop();

        // check if sprite pixel is not transparent and that sprites are enabled
        assert((sp.color & 0b11) == sp.color);
        if (sp.color > 0 and LCDC & 1 << 1)
        {
            // check bg priority of sprite
            px = sp.bg_priority and px.color > 0 ? px : sp;
        }
    }

    // if BG is disabled, make BG pixels use color ID 0
    if (not(LCDC & 1) and px.pallet == BG)
        px.color = 0;

    // retrieve color from pallet and push to LCD
    assert(px.pallet == BG or px.color != 0);
    uint8_t c;
    switch (px.pallet)
    {
    case BG:
    {
        c = (BGP >> (2 * px.color)) & 0b11;
        break;
    }
    case S0:
    {
        c = (OBP0 >> (2 * px.color)) & 0b11;
        break;
    }
    case S1:
    {
        c = (OBP1 >> (2 * px.color)) & 0b11;
        break;
    }
    default:
        std::unreachable();
    }

    assert(c <= 3);
    lcd.push_pixel((LCD::Color)c);

    // if the last pixel of the scanline was pushed, move on to H-Blank mode
    if (++fifo_pushed_pixels == 160)
    {
        fetcher.reset();
        while (not BG_FIFO.empty())
            BG_FIFO.pop();
        while (not Sprite_FIFO.empty())
            Sprite_FIFO.pop();
        set_mode(H_Blank);
    }
}

uint32_t PPU::tick() {
    // skip if LCD is turned off
    if (not (LCDC & 1 << 7)) return 1;

    /* set time of current tick to 0. */
    tick_time_ = 0;

    // save current STAT register for rising edge detector (https://gbdev.io/pandocs/Interrupt_Sources.html#int-48--stat-interrupt)
    bool old_STATE_signal = STATE_interrupt_signal();

    // Determine current mode_
    switch (mode_) {
        case H_Blank:
            h_blank_tick();
            break;
        case V_Blank:
            v_blank_tick();
            break;
        case OAM_Scan: 
            oam_scan_tick();
            break;
        case Pixel_Transfer:
            pixel_transfer_tick();
            break;
        default:
            throw std::runtime_error("Unknown PPU mode");
    }

    // check for STAT interrupts (rising edge detector)
    if (not old_STATE_signal and STATE_interrupt_signal())
        yume_boy_.request_interrupt(YumeBoy::INTERRUPT::STAT_INTERRUPT);

    return tick_time_;
}
