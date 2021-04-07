#pragma once

const size_t LCD_X = 160;
const size_t LCD_Y = 128;

class Display {
  public:
    Display();
    void Show();
    void Fill(uint16_t color);
    void FillRect(size_t x, size_t y, size_t w, size_t h, uint16_t color);
    void Rect(size_t x, size_t y, size_t w, size_t h, uint16_t color);
    void SetPixel(size_t x, size_t y, uint16_t color);
    void Text(const std::string &s, size_t x0, size_t y0, uint16_t color);
    void TextLarge(const std::string &s, size_t x0, size_t y0, uint16_t color);

    const uint16_t red = 0x07E0;
    const uint16_t green = 0x001f;
    const uint16_t blue = 0xf800;
    const uint16_t white = 0xffff;

  private:
    std::array<uint8_t, LCD_X * LCD_Y * 2> _framebuffer;
};