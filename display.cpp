#include <array>

#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "display.hpp"

#define SPI_PORT spi1

#define PIN_CS 9
#define PIN_SCK 10
#define PIN_MOSI 11
#define PIN_RST 12
#define PIN_DC 8

#define LCD_X_Adjust 1
#define LCD_Y_Adjust 2

static inline void cs_select_on() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_select_off() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

static inline void dc_data() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_DC, 1);
    asm volatile("nop \n nop \n nop");
}

static inline void dc_command() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_DC, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void reset_display() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_RST, 0);
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_RST, 1);
    asm volatile("nop \n nop \n nop");
}

static void command(uint8_t c) {
    uint8_t buf[] = {c};

    dc_command();
    cs_select_on();
    spi_write_blocking(SPI_PORT, buf, 1);
    cs_select_off();
}

static void data(uint8_t d) {
    uint8_t buf[] = {d};

    dc_data();
    cs_select_on();
    spi_write_blocking(SPI_PORT, buf, 1);
    cs_select_off();
}

static void init_reg() {
    command(0xB1);
    data(0x01);
    data(0x2C);
    data(0x2D);

    command(0xB2);
    data(0x01);
    data(0x2C);
    data(0x2D);

    command(0xB3);
    data(0x01);
    data(0x2C);
    data(0x2D);
    data(0x01);
    data(0x2C);
    data(0x2D);

    // Column inversion
    command(0xB4);
    data(0x07);

    // ST7735R Power Sequence
    command(0xC0);
    data(0xA2);
    data(0x02);
    data(0x84);
    command(0xC1);
    data(0xC5);

    command(0xC2);
    data(0x0A);
    data(0x00);

    command(0xC3);
    data(0x8A);
    data(0x2A);
    command(0xC4);
    data(0x8A);
    data(0xEE);

    command(0xC5); // VCOM
    data(0x0E);

    // ST7735R Gamma Sequence
    command(0xe0);
    data(0x0f);
    data(0x1a);
    data(0x0f);
    data(0x18);
    data(0x2f);
    data(0x28);
    data(0x20);
    data(0x22);
    data(0x1f);
    data(0x1b);
    data(0x23);
    data(0x37);
    data(0x00);
    data(0x07);
    data(0x02);
    data(0x10);

    command(0xe1);
    data(0x0f);
    data(0x1b);
    data(0x0f);
    data(0x17);
    data(0x33);
    data(0x2c);
    data(0x29);
    data(0x2e);
    data(0x30);
    data(0x30);
    data(0x39);
    data(0x3f);
    data(0x00);
    data(0x07);
    data(0x03);
    data(0x10);

    // Enable test command
    command(0xF0);
    data(0x01);

    // Disable ram power save mode
    command(0xF6);
    data(0x00);

    // 65k mode
    command(0x3A);
    data(0x05);
}

static void set_windows(uint8_t Xstart, uint8_t Ystart, uint8_t Xend,
                        uint8_t Yend) {
    // set the X coordinates
    command(0x2A);
    data(0x00); // Set the horizontal starting point to the high octet
    data((Xstart & 0xff) +
         LCD_X_Adjust); // Set the horizontal starting point to the low octet
    data(0x00);         // Set the horizontal end to the high octet
    data(((Xend - 1) & 0xff) +
         LCD_X_Adjust); // Set the horizontal end to the low octet

    // set the Y coordinates
    command(0x2B);
    data(0x00);
    data((Ystart & 0xff) + LCD_Y_Adjust);
    data(0x00);
    data(((Yend - 1) & 0xff) + LCD_Y_Adjust);

    command(0x2C);
}

static void init_display() {
    spi_init(SPI_PORT, 40000000);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // bi_decl(bi_2pins_with_func(PIN_MOSI, PIN_SCK, GPIO_FUNC_SPI));

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);

    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_RST, 1);

    reset_display();
    init_reg();

    // Scan_dir == U2D_R2L
    command(0x36);                     // MX, MY, RGB mode
    data((0X00 | 0x40 | 0x20) & 0xf7); // RGB color filter panel
    sleep_ms(200);

    command(0x11);
    sleep_ms(120);

    // Turn on the LCD display
    command(0x29);
}

Display::Display() { init_display(); }

void Display::Fill(uint16_t color) {
    // the screen controller is big-endian
    const uint8_t lower = (color >> 8) & 0xff;
    const uint8_t upper = color & 0xff;

    for (size_t i = 0; i < _framebuffer.size(); i += 2) {
        this->_framebuffer[i] = upper;
        this->_framebuffer[i + 1] = lower;
    }
}

void Display::FillRect(size_t x, size_t y, size_t w, size_t h, uint16_t color) {
    if (h < 1 || w < 1 || x + w <= 0 || y + h <= 0 || y >= LCD_Y ||
        x >= LCD_X) {
        return;
    }

    w = std::min(w, LCD_X - x);
    h = std::min(h, LCD_Y - y);

    size_t d = (x + (y * LCD_X)) * 2;
    while (h--) {
        for (unsigned int ww = w; ww; --ww) {
            const uint8_t lower = (color >> 8) & 0xff;
            const uint8_t upper = color & 0xff;

            this->_framebuffer[d] = upper;
            this->_framebuffer[d + 1] = lower;
            d += 2;
        }
        d += (LCD_X - w) * 2;
    }
}

void Display::Rect(size_t x, size_t y, size_t w, size_t h, uint16_t color) {
    this->FillRect(x, y, w, 1, color);
    this->FillRect(x, y + h - 1, w, 1, color);
    this->FillRect(x, y, 1, h, color);
    this->FillRect(x + w - 1, y, 1, h, color);
}

void Display::Show() {
    set_windows(0, 0, LCD_X, LCD_Y);
    dc_data();
    cs_select_on();
    spi_write_blocking(SPI_PORT, this->_framebuffer.data(),
                       this->_framebuffer.size());
    cs_select_off();
}
