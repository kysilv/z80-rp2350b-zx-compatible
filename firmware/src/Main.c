#include <stdio.h>
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "pico/multicore.h"

#include "Pin_defs.h"

#include "MCLK.pio.h"
#include "CPUCLK.pio.h"
#include "IN_FE.pio.h"
#include "OUT_FE.pio.h"

#include "VGA.pio.h"
#include "activevideo.pio.h"
#include "VRAM_VGA.pio.h"

#include "INT.pio.h"
#include "Read_frame.pio.h"
#include "Read_line.pio.h"
#include "Read_VRAM.pio.h"

#define pixel_bytes_count 98304

#define half_border_count 48
#define screen_line_count 256

uint8_t vga_data_array[pixel_bytes_count] = {0};
uint8_t border = 0x7;

uint32_t vram_address = 0;

int ctrl_chan;
int data_chan;
int border_chan;

uint16_t currentline = 0;

struct {uint32_t ctrl; uint32_t len; const char *data; io_wo_32 *write;} control_blocks[4];

uint32_t data_line_ctrl;

void handle_VRAM_read() {
    PIO Read_VRAMpio = pio2;
    uint Read_VRAMsm = 3;
    uint8_t frames = 0;
    while (true) {
        uint32_t display_address = 0;
        for (uint32_t row=0; row<192; row++) {
            uint32_t line_addr = ((row & 0xC0) << 5) | ((row & 0x07) << 8) | ((row & 0x38) << 2);
            uint32_t rowattrbase = 6144+((row >> 3) << 5);
            for (uint8_t pixelbyte = 0; pixelbyte < 32; pixelbyte++) {
                pio_sm_put_blocking(Read_VRAMpio, Read_VRAMsm, line_addr+pixelbyte); 
                uint8_t pixels = pio_sm_get_blocking(Read_VRAMpio, Read_VRAMsm);
                pio_sm_put_blocking(Read_VRAMpio, Read_VRAMsm, rowattrbase+pixelbyte);
                uint8_t attr = pio_sm_get_blocking(Read_VRAMpio, Read_VRAMsm);
                uint8_t paper = (attr >> 3);
                uint8_t ink = attr & 0b00000111 | paper & 0b00001000;
                for (int8_t pixelindex=7; pixelindex>=0; pixelindex--) {
                    uint8_t pixel_bit = (pixels >> pixelindex) & 1;
                    if (pixel_bit ^ ((paper & (frames & 0b00010000)) >> 4)) {
                        vga_data_array[display_address] = ink;
                        vga_data_array[display_address+256] = ink;
                    } else {
                        vga_data_array[display_address] = paper;
                        vga_data_array[display_address+256] = paper;
                    }
                    display_address++;
                }
            }
            display_address+=256;
        }
        frames++;
    }
}

void dma_handler() {
    dma_hw->ints0 = 1u << data_chan;

    if (currentline < 96 || currentline >= 480) {
        control_blocks[1].ctrl = control_blocks[0].ctrl;
        control_blocks[1].data = &border;
    } else {
        control_blocks[1].ctrl = data_line_ctrl;
        control_blocks[1].data = &vga_data_array[vram_address];
        vram_address+=screen_line_count;
    }

    currentline++;
    if (currentline >= 575) {
        currentline = 0;
        vram_address = 0;
    }

    dma_channel_set_read_addr(ctrl_chan, &control_blocks[0], true);    
}

int main()
{
    set_sys_clock_khz(154000, true);

    gpio_init(UCLK); 
    gpio_set_dir(UCLK, true);
    gpio_put(UCLK, true);  // set UCLK = 1 (CLK = 0)

    gpio_init(Z80Reset); // init CPU RESET gpio
    gpio_set_drive_strength(Z80Reset, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_dir(Z80Reset, true); // direction = OUT
    gpio_put(Z80Reset, false); // set CPU RESET active

    gpio_init(UCS); 
    gpio_set_dir(UCS, true);
    gpio_put(UCS, true);  // set UCS = 1

    gpio_init(UBUS); 
    gpio_set_dir(UBUS, true);
    gpio_put(UBUS, true);  // set UBUS = 1
/*
    gpio_init(EAR); 
    gpio_set_drive_strength(EAR, GPIO_DRIVE_STRENGTH_4MA);
    gpio_set_dir(EAR, true); // direction = OUT
    gpio_put(EAR, false); // 

    gpio_init(MIC); 
    gpio_set_drive_strength(MIC, GPIO_DRIVE_STRENGTH_4MA);
    gpio_set_dir(MIC, true); // direction = OUT
    gpio_put(MIC, false); // 

    gpio_init(VGA_R); 
    gpio_set_drive_strength(VGA_R, GPIO_DRIVE_STRENGTH_8MA);
    gpio_set_dir(VGA_R, true); // direction = OUT
    gpio_put(VGA_R, false); // 

    gpio_init(VGA_B); 
    gpio_set_drive_strength(VGA_B, GPIO_DRIVE_STRENGTH_8MA);
    gpio_set_dir(VGA_B, true); // direction = OUT
    gpio_put(VGA_B, false); // 

    gpio_init(VGA_G); 
    gpio_set_drive_strength(VGA_G, GPIO_DRIVE_STRENGTH_8MA);
    gpio_set_dir(VGA_G, true); // direction = OUT
    gpio_put(VGA_G, false); // 

    gpio_init(BRIGHT); 
    gpio_set_drive_strength(BRIGHT, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_dir(BRIGHT, true); // direction = OUT
    gpio_put(BRIGHT, false); // 
*/

    PIO MCLKpio = pio0;
    uint MCLKsm = 0;
    uint MCLKoffset = 0;

    uint CPUCLKsm = 1;
    uint CPUCLKoffset = 1;

    uint IN_FEsm = 2;
    uint IN_FEoffset = 12;

    uint OUT_FEsm = 3;
    uint OUT_FEoffset = 22;

    PIO VGApio = pio1;

    uint VGA_HSYNCsm = 0;
    uint VGA_VSYNCsm = 1;
    uint VGAoffset = 0;

    uint activevideosm = 2;
    uint activevideooffset = 10;

    uint VRAM_VGAsm = 3;
    uint VRAM_VGAoffset = 22;

    PIO Read_VRAMpio = pio2;

    uint INTsm = 0;
    uint INToffset = 0;

    uint Read_framesm = 1;
    uint Read_frameoffset = 8;
    
    uint Read_linesm = 2;
    uint Read_lineoffset = 15;

    uint Read_VRAMsm = 3;
    uint Read_VRAMoffset = 26;
    
    bool success;
//    hard_assert(success); 
    success = pio_set_gpio_base(MCLKpio, 16);
    success = pio_set_gpio_base(VGApio, 16);
    success = pio_set_gpio_base(Read_VRAMpio, 0);
    success = pio_add_program_at_offset(MCLKpio, &MCLK_program, MCLKoffset);
    success = pio_add_program_at_offset(MCLKpio, &CPUCLK_program, CPUCLKoffset);
    success = pio_add_program_at_offset(MCLKpio, &IN_FE_program, IN_FEoffset);
    success = pio_add_program_at_offset(MCLKpio, &OUT_FE_program, OUT_FEoffset);

    success = pio_add_program_at_offset(VGApio, &VGA_program, VGAoffset);    
    success = pio_add_program_at_offset(VGApio, &activevideo_program, activevideooffset);        
    success = pio_add_program_at_offset(VGApio, &VRAM_VGA_program, VRAM_VGAoffset);        

    success = pio_add_program_at_offset(Read_VRAMpio, &INT_program, INToffset);
    success = pio_add_program_at_offset(Read_VRAMpio, &Read_frame_program, Read_frameoffset);        
    success = pio_add_program_at_offset(Read_VRAMpio, &Read_line_program, Read_lineoffset);        
    success = pio_add_program_at_offset(Read_VRAMpio, &Read_VRAM_program, Read_VRAMoffset);    

    ctrl_chan = dma_claim_unused_channel(true);
    data_chan = dma_claim_unused_channel(true);
    border_chan = dma_claim_unused_channel(true);

    dma_channel_config cb = dma_channel_get_default_config(border_chan);
    channel_config_set_transfer_data_size(&cb, DMA_SIZE_8);
    channel_config_set_read_increment(&cb, false);
    channel_config_set_write_increment(&cb, false);
    channel_config_set_dreq(&cb, DREQ_PIO0_RX3);
    
     dma_channel_configure(
        border_chan,                 // Channel to be configured
        &cb,                        // The configuration we just created
        &border,            // write address (border color)
        &MCLKpio->rxf[OUT_FEsm],          // read address (VRAM_VGA PIO TX FIFO)
        dma_encode_endless_transfer_count(),                    // Number of transfers; in this case each is 1 byte.
        true                     // Start immediately, why not
    );

    dma_channel_config c0 = dma_channel_get_default_config(data_chan);
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_8);
    channel_config_set_write_increment(&c0, false);
    channel_config_set_dreq(&c0, DREQ_PIO1_TX3);                     
    channel_config_set_chain_to(&c0, ctrl_chan);
    channel_config_set_irq_quiet(&c0, true);

    channel_config_set_read_increment(&c0, true);                       
    data_line_ctrl = channel_config_get_ctrl_value(&c0);
    channel_config_set_read_increment(&c0, false);
    control_blocks[0].ctrl = channel_config_get_ctrl_value(&c0);
    control_blocks[1].ctrl = channel_config_get_ctrl_value(&c0);
    control_blocks[2].ctrl = channel_config_get_ctrl_value(&c0);
    control_blocks[3].ctrl = channel_config_get_ctrl_value(&c0);

    control_blocks[0].len = half_border_count;
    control_blocks[1].len = screen_line_count;
    control_blocks[2].len = half_border_count;
    control_blocks[3].len = 0;

    control_blocks[0].data = &border;
    control_blocks[1].data = &border;
    control_blocks[2].data = &border;
    control_blocks[3].data = &border;

    control_blocks[0].write = &VGApio->txf[VRAM_VGAsm];
    control_blocks[1].write = &VGApio->txf[VRAM_VGAsm];    
    control_blocks[2].write = &VGApio->txf[VRAM_VGAsm];    
    control_blocks[3].write = NULL;

    dma_channel_configure(
        data_chan,                 // Channel to be configured
        &c0,                        // The configuration we just created
        &VGApio->txf[VRAM_VGAsm],          // write address (VRAM_VGA PIO TX FIFO)
        &border,            // The initial read address (border color)
        half_border_count,                    // Number of transfers; in this case each is 1 byte.
        false                       // Don't start immediately.
    );

    dma_channel_config c1 = dma_channel_get_default_config(ctrl_chan);  
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);            
    channel_config_set_read_increment(&c1, true);                       
    channel_config_set_write_increment(&c1, true);                      
    channel_config_set_ring(&c1, true, 4);

    dma_channel_configure(
        ctrl_chan,                 
        &c1,                       
        &dma_hw->ch[data_chan].al2_ctrl,
        &control_blocks[0],
        4,
        false                       
    );

    dma_channel_set_irq0_enabled(data_chan, true);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    CPUCLK_program_init(MCLKpio, CPUCLKsm, CPUCLKoffset, UCLK, UBUS);
    VGA_program_init(VGApio, VGA_HSYNCsm, VGAoffset, HSYNC);
    pio_sm_put_blocking(VGApio, VGA_HSYNCsm, 207-1); //hsync high in 7MHz clocks
    pio_sm_put_blocking(VGApio, VGA_HSYNCsm, 17-1); // hsync low  in 7MHz clocks
    VGA_program_init(VGApio, VGA_VSYNCsm, VGAoffset, VSYNC);
    pio_sm_put_blocking(VGApio, VGA_VSYNCsm,  (69888-112*5)*2-1);
    pio_sm_put_blocking(VGApio, VGA_VSYNCsm, 112*5*2-1); 
    activevideo_program_init(VGApio, activevideosm, activevideooffset);
    pio_sm_put_blocking(VGApio, activevideosm, 38-1); //39 lines to skip for vertical backporch
    pio_sm_put_blocking(VGApio, activevideosm, 575-1); //576 lines in frame
    VRAM_VGA_program_init(VGApio, VRAM_VGAsm, VRAM_VGAoffset, VGA_B);
    pio_sm_put_blocking(VGApio, VRAM_VGAsm, 352-1); //352 pixels in line

    INT_program_init(Read_VRAMpio, INTsm, INToffset, INT);
    pio_sm_put_blocking(Read_VRAMpio, INTsm, 891); //892 (891 = ~14330 T in total before 1st pixel, which is better than 892)
    Read_frame_program_init(Read_VRAMpio, Read_framesm, Read_frameoffset);
    pio_sm_put_blocking(Read_VRAMpio, Read_framesm, 192-1); //192 lines
    Read_line_program_init(Read_VRAMpio, Read_linesm, Read_lineoffset, UCS, URD);
    Read_VRAM_program_init(Read_VRAMpio, Read_VRAMsm, Read_VRAMoffset, URD, Y0, Q0);    

    IN_FE_program_init(MCLKpio, IN_FEsm, IN_FEoffset, Q0, IORQ);
    OUT_FE_program_init(MCLKpio, OUT_FEsm, OUT_FEoffset, CWE, Q0, MIC);
    
    MCLK_program_init(MCLKpio, MCLKsm, MCLKoffset);
    
    multicore_launch_core1(handle_VRAM_read);
    dma_start_channel_mask((1u << ctrl_chan));    

    MCLKpio->ctrl = 0b101111111110000111100001111;

    sleep_ms(3245);

    gpio_put(Z80Reset, true); // set CPU RESET inactive

    while (true) {}

}
