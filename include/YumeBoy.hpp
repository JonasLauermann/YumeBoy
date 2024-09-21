#pragma once

#include "cpu/CPU.hpp"
#include "cpu/InterruptBus.hpp"
#include "cartridge/Cartridge.hpp"
#include "mmu/RAM.hpp"
#include "mmu/MemoryStub.hpp"
#include "mmu/MMU.hpp"
#include "mmu/DMA.hpp"
#include "ppu/LCD.hpp"
#include "ppu/PPU.hpp"
#include "joypad/Joypad.hpp"
#include "timer/Timer.hpp"
#include <memory>


/** Stores all components of the emulator and facilitates communication between components. */
class YumeBoy {
    int64_t ticks = 0;

    std::unique_ptr<MMU> mmu_;
    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<Cartridge> cartridge_;
    std::unique_ptr<PPU> ppu_;
    std::unique_ptr<LCD> lcd_;
    std::unique_ptr<MemorySTUB> audio_;
    std::unique_ptr<RAM> hram_;
    std::unique_ptr<RAM> wram_;
    std::unique_ptr<MemorySTUB> link_cable_;
    std::unique_ptr<Joypad> joypad_;
    std::unique_ptr<Timer> timer_;
    std::unique_ptr<InterruptBus> interrupts_;
    std::unique_ptr<DMA> dma_;
    std::unique_ptr<DMA_Memory> dma_memory_;

    public:
    explicit YumeBoy(const std::string& filepath, bool skip_bootrom) {
        mmu_ = std::make_unique<MMU>();
        dma_ = std::make_unique<DMA>(*mmu_);
        dma_memory_ = std::make_unique<DMA_Memory>(*mmu_, *dma_);
        mmu_->add(dma_.get());

        cpu_ = std::make_unique<CPU>(*dma_memory_, skip_bootrom);
        mmu_->add(cpu_.get());

        interrupts_ = std::make_unique<InterruptBus>(*cpu_);

        cartridge_ = CartridgeFactory::Create(filepath, skip_bootrom);
        mmu_->add(cartridge_.get());

        lcd_ = std::make_unique<LCD>("YumeBoy", LCD::DISPLAY_WIDTH * 4, LCD::DISPLAY_HEIGHT * 4);
        ppu_ = std::make_unique<PPU>(*lcd_, *dma_memory_, *interrupts_);
        mmu_->add(ppu_.get());

        audio_ = std::make_unique<MemorySTUB>("Audio", 0xFF10, 0xFF26);
        mmu_->add(audio_.get());

        hram_ = std::make_unique<RAM>(0xFF80, 0xFFFE);
        mmu_->add(hram_.get());

        wram_ = std::make_unique<RAM>(0xC000, 0xDFFF);
        mmu_->add(wram_.get());

        link_cable_ = std::make_unique<MemorySTUB>("Serial Data Transfer (Link Cable)", 0xFF01, 0xFF02);
        mmu_->add(link_cable_.get());

#ifndef NDEBUG
        joypad_ = std::make_unique<Joypad>(*this, *interrupts_);
#else
        joypad_ = std::make_unique<Joypad>(*interrupts_);
#endif
        mmu_->add(joypad_.get());

        timer_ = std::make_unique<Timer>(*interrupts_);
        mmu_->add(timer_.get());
    }

    ~YumeBoy() {
        SDL_Quit();
    }

    void tick() {
        ++ticks;

        if (ticks % 4 == 0) {
            cpu_->tick();
            dma_->tick();
        }

        if (ticks % 1000 == 0) { // SDL_PollEvent is expensive and updating the joypad state every tick is overkill
            joypad_->update_joypad_state();
        }

        ppu_->tick();
        timer_->tick();
    }

#ifndef NDEBUG
    void dump_tilemap() {
        // advance emulation until PPU is no longer in PIXEL_TRANSFER mode
        while ((mmu_->read_memory(0xFF41) & 0b11) == 3)
            tick();

        const size_t SIZE = 128 * 3 * 64 * 3;  // tiles per block * 3 blocks * num pixels per tile * 3 color channels (RGB)
        std::array<uint8_t, SIZE> image_data;

        std::ofstream tilemap_file;		//output stream object
        tilemap_file.open("tilemap.ppm");

        if (not tilemap_file.is_open())
        {
            std::cout << "Unable to create/open tilemap.ppm" << std::endl;
            return;
        }

        const int width = 16 * 8;
        const int height = 24 * 8;
        static_assert(SIZE == width * height * 3);

        //Image header - Need this to start the image properties
        tilemap_file << "P3" << std::endl;                      //Declare that you want to use ASCII colour values
        tilemap_file << width << " " << height << std::endl;    //Declare w & h
        tilemap_file << "255" << std::endl;                     //Declare max colour ID
        

        // get palette data
        uint8_t palette = mmu_->read_memory(0xFF48); // OBJ palette 0 data

        int pixel = 0;
        //Image Painter - sets the background and the diagonal line to the array
        for (unsigned int row = 0; row < height; row++) {
            for (unsigned int col = 0; col < width; col += 8) {
                // calculate current tile ID
                uint16_t tile_id = ((uint16_t(row) / 8) * 0x10) + (uint16_t(col) / 8);

                // fetch 2 bytes representing a single row of a single tile
                uint8_t tile_row = row % 8;
                auto tile_data_addr = uint16_t(0x8000 + (tile_id << 4));
                uint8_t lower_byte = mmu_->read_memory(tile_data_addr + (tile_row * 2));
                uint8_t higher_byte = mmu_->read_memory(tile_data_addr + (tile_row * 2) + 1);

                // convert to color using palette
                for (int i = 7; i >= 0; --i) {
                    uint8_t tile_color = (((higher_byte << 1) >> i) & 0b10) | ((lower_byte >> i) & 0b1);
                    uint8_t c = (palette >> (2 * tile_color)) & 0b11;
                    uint8_t r, g, b;

                    switch (c)
                    {
                    case 0: // WHITE
                        r = 233;
                        g = 239;
                        b = 236;
                        break;

                    case 1: // LIGHT_GRAY
                        r = 160;
                        g = 160;
                        b = 139;
                        break;

                    case 2: // DARK_GRAY
                        r = 85;
                        g = 85;
                        b = 104;
                        break;

                    case 3:  // BLACK
                        r = 33;
                        g = 30;
                        b = 32;
                        break;

                    default:
                        std::unreachable();
                    }

                    // write row data to array
                    image_data[(pixel + (7 - i)) * 3] = r;
                    image_data[(pixel + (7 - i)) * 3 + 1] = g;
                    image_data[(pixel + (7 - i)) * 3 + 2] = b;
                }
                pixel += 8;
            }
        }
        

        //Image Body - outputs image_data array to the .ppm file, creating the image
        for (int x = 0; x < SIZE; x += 3) {
            int r = image_data[x];		//Sets value as an integer, not a character value
            int g = image_data[x+1];		//Sets value as an integer, not a character value
            int b = image_data[x+2];		//Sets value as an integer, not a character value
            tilemap_file << r << " " << g << " " << b << " " << std::endl;		//Sets 3 bytes of colour to each pixel	
        }

        tilemap_file.close();
    }

    void screenshot() const {
        lcd_->screenshot("screenshot.bpm");
    }
#endif

};