#include "ppu/PPU.hpp"

#include "YumeBoy.hpp"

void PPU::set_mode(PPU_STATES new_state)
{
    #ifndef NDEBUG
    switch (state)
    {
    case PPU_STATES::HBlank:
    {
        assert(new_state ==  PPU_STATES::VBlank or new_state ==  PPU_STATES::OAMScan);
        break;
    }
    case PPU_STATES::VBlank:
    {
        assert(new_state ==  PPU_STATES::OAMScan);
        break;
    }
    case PPU_STATES::OAMScan:
    {
        assert(new_state == PPU_STATES::PixelTransfer);
        break;
    }
    case PPU_STATES::PixelTransfer:
    {
        assert(new_state ==  PPU_STATES::HBlank);
        break;
    }
    default:
        std::unreachable();
    }
    #endif
    state = new_state;
    STAT = (STAT & 0b11111100) | std::to_underlying(new_state);
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

bool PPU::STATE_interrupt_signal() const
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

uint8_t PPU::read_vram(uint16_t addr)
{
    assert(VRAM_BEGIN <= addr and addr <= VRAM_END);
    // VRAM is inaccessible during pixel transfer
    if (state == PPU_STATES::PixelTransfer)
        return 0xFF;
    return vram_[addr - VRAM_BEGIN];
}

void PPU::write_vram(uint16_t addr, uint8_t value)
{
    assert(VRAM_BEGIN <= addr and addr <= VRAM_END);
    // VRAM is inaccessible during pixel transfer
    if (state == PPU_STATES::PixelTransfer)
        return;
    vram_[addr - VRAM_BEGIN] = value;
}

uint8_t PPU::read_oam_ram(uint16_t addr)
{
    assert(OAM_RAM_BEGIN <= addr and addr <= OAM_RAM_END);
    // OAM RAM is inaccessible during OAM scan and pixel transfer
    if (state == PPU_STATES::PixelTransfer or state == PPU_STATES::OAMScan)
        return 0xFF;
    return oam_ram_[addr - OAM_RAM_BEGIN];
}

void PPU::write_oam_ram(uint16_t addr, uint8_t value)
{
    assert(OAM_RAM_BEGIN <= addr and addr <= OAM_RAM_END);
    // OAM RAM is inaccessible during OAM scan and pixel transfer
    if (state == PPU_STATES::PixelTransfer or state == PPU_STATES::OAMScan)
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
        std::unreachable();
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
        std::unreachable();
    }
}

void PPU::h_blank_tick()
{
    if (scanline_time_ == 456)
    {
        /* Move to next scanline and switch mode */
        next_scanline();
        if (LY == 144)
        {
            set_mode(PPU_STATES::VBlank);
            interrupts.request_interrupt(InterruptBus::INTERRUPT::V_BLANK_INTERRUPT);
            lcd.update_screen();
        }
        else
        {
            assert(LY < 144);
            set_mode(PPU_STATES::OAMScan);
        }
        scanline_time_ = 0;
        fifo_pushed_pixels = 0;
    }
}

void PPU::v_blank_tick()
{
    assert(scanline_time_ <= 456);
    if (scanline_time_ == 456)
    {
        /* Move to next scanline and switch mode if necessary */
        assert(144 <= LY and LY <= 153);
        next_scanline();
        if (LY == 154)
        {
            LY = 0;
            if (LYC == LY) {
                STAT |= 1 << 2;
            } else {
                STAT &= 0b11111011;
            }
            set_mode(PPU_STATES::OAMScan);
        }
        scanline_time_ = 0;
    }
}

void PPU::oam_scan_tick()
{
    /* OAM Scan takes 2 T-cycles per OAM entry. */
    if (scanline_time_ % 2 != 0) return;

    /* Scan the OAM RAM. The CPU has no access to the OAM RAM in this mode. */
    if (oam_pointer == OAM_RAM_BEGIN)
        scanline_sprites.clear();

    auto e = std::make_unique<OAM_entry>(
        oam_ram_[oam_pointer - OAM_RAM_BEGIN],
        oam_ram_[oam_pointer - OAM_RAM_BEGIN + 1],
        oam_ram_[oam_pointer - OAM_RAM_BEGIN + 2],
        oam_ram_[oam_pointer - OAM_RAM_BEGIN + 3]);
    oam_pointer += 4;

    // check if the OAM entry is visible on the current scanline
    uint8_t sprite_height = 8 + (8 * ((LCDC >> 2) & 1));
    if ((scanline_sprites.size() < 10) and (LY + 16 >= e->y and LY + 16 < e->y + sprite_height))
    {
        scanline_sprites.push_back(std::move(e));
    }

    if (oam_pointer == OAM_RAM_END + 1)
    {
        assert(scanline_time_ == 80);
        set_mode(PPU_STATES::PixelTransfer);
        oam_pointer = OAM_RAM_BEGIN;
    }
}

void PPU::pixel_transfer_tick()
{
    // the pixel fetcher is two times slower than the rest of the ppu
    if (scanline_time_ % 2 == 0)
        fetcher.tick();

    // stop if the pixel fifo is empty or stopped
    if (BG_FIFO.empty() or fetcher.fifo_stopped())
        return;

    // if a sprite is at the current x position, switch the fetcher into sprite fetch mode
    if (auto it = std::find_if(
            scanline_sprites.begin(),
            scanline_sprites.end(),
            [&](const std::unique_ptr<OAM_entry>& e)
            { return e->x <= fifo_pushed_pixels + 9; }); // TODO not entirely sure why we have to offset by 9, seems like a bug elsewhere (handle special case where sprite is cut off at the left edge of the LCD)
        it != scanline_sprites.end())
    {
        fetcher.fetch_sprite(std::move(*it));  // TODO what if sprites are disabled?
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
        set_mode(PPU_STATES::HBlank);
    }
}

// TODO: rewrite PPU as state machine to allow for t-cycle precision
void PPU::tick() {
    // skip if LCD is turned off
    if (not (LCDC & 1 << 7)) return;

    ++scanline_time_;

    // save current STAT register for rising edge detector (https://gbdev.io/pandocs/Interrupt_Sources.html#int-48--stat-interrupt)
    bool old_STATE_signal = STATE_interrupt_signal();

    // Determine current state
    switch (state) {
        using enum PPU_STATES;
        case HBlank:
            h_blank_tick();
            break;
        case VBlank:
            v_blank_tick();
            break;
        case OAMScan: 
            oam_scan_tick();
            break;
        case PixelTransfer:
            pixel_transfer_tick();
            break;
        default:
            std::unreachable();
    }

    // check for STAT interrupts (rising edge detector)
    if (not old_STATE_signal and STATE_interrupt_signal())
        interrupts.request_interrupt(InterruptBus::INTERRUPT::STAT_INTERRUPT);
}

bool PPU::contains_address(uint16_t addr) const
{
    return (0x8000 <= addr and addr <= 0x9FFF) or (0xFE00 <= addr and addr <= 0xFE9F) or (0xFF40 <= addr and addr <= 0xFF45) or (0xFF47 <= addr and addr <= 0xFF4B);
}

uint8_t PPU::read_memory(uint16_t addr)
{
    if (0x8000 <= addr and addr <= 0x9FFF)
        return read_vram(addr);
    else if (0xFE00 <= addr and addr <= 0xFE9F)
        return read_oam_ram(addr);
    else if ((0xFF40 <= addr and addr <= 0xFF45) or (0xFF47 <= addr and addr <= 0xFF4B))
        return read_lcd_register(addr);
    else
        std::unreachable();
}


void PPU::write_memory(uint16_t addr, uint8_t value)
{
    if (0x8000 <= addr and addr <= 0x9FFF)
        return write_vram(addr, value);
    else if (0xFE00 <= addr and addr <= 0xFE9F)
        return write_oam_ram(addr, value);
    else if ((0xFF40 <= addr and addr <= 0xFF45) or (0xFF47 <= addr and addr <= 0xFF4B))
        return write_lcd_register(addr, value);
    else
        std::unreachable();
}
