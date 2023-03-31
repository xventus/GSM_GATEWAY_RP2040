//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   lcd5110.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>
#include "hardware/gpio.h"
#include "hardware/spi.h"


/**
 * @brief Class for basic operation of the Nokia 5110 graphic display
 * 
 */
class Lcd5110
{

public:
    // LCD5110 screen size
    static const uint8_t maxX{84}; ///< Number of pixels per display width
    static const uint8_t maxY{48}; ///< Number of pixels per display height

    /**
     * @brief ctor
     * 
     * @param spi - SPI e.g. spi0, spi1 ...
     * @param rst - RST pin 
     * @param ce  - CE pin, can be omitted by setting to 0, then CE must be connected to GND 
     * @param dc  - DC pin, Data / Command Selection
     * @param din - DIN pin, Data Input
     * @param clk - CLK pin, clock
     * @param light - LIGHT (BL) pin can be omitted by setting to 0, Backlight Supply it is advisable to reduce with a resistor of 220 to 330 ohms
     */
    explicit Lcd5110(spi_inst_t *spi,
            uint8_t rst = 8,
            uint8_t ce = 12,
            uint8_t dc = 11,
            uint8_t din = 7,
            uint8_t clk = 6,
            uint8_t light = 10) : _spi{spi},
                                  _rst(rst),
                                  _ce(ce),
                                  _dc(dc),
                                  _din(din),
                                  _clk(clk),
                                  _light(light)
    {
    }

    /**
     * @brief object initialization, pin setting, enablement
     * 
     * @param bias - determines how pixels are too intense, determines how much voltage is connected to the diplay
     *               value 3-5
     * @param contrast - contrast setting controls the difference between the dark and light pixels 0-127
     * 
     */
    void init(uint8_t bias = 4, uint8_t contrast = 60);

    /**
     * @brief adds the font for rendering to the internal buffer. 
     *        It is always necessary to assign a font table before using the function to output text. 
     * 
     * @param lcdFont - A general byte array that describes a bitmap font. 
     *                  The individual bytes in sequence contain the pixels of the font column. 
     *                  If the column is larger than 8 pixels, an additional byte is taken. 
     *                  For example, for a 10 pixel high font, one column takes up 2 bytes. 
     * @param hg - glyph height in pixels
     * @param wg -  glyph width in pixels
     * @param firstChar - character (ASCII value) of the character that is at the first position in the table
     * @param sz - Table size measured in glyph counts. For example, for a table containing a number from 0 to 9, the size would be 10
     */
    void withFont(const unsigned char *lcdFont, uint8_t hg, uint8_t wg, unsigned char firstChar, uint8_t sz)
    {
        _lcdFont = lcdFont;
        _hg = hg;
        _wg = wg;
        _firstChar = firstChar;
        _fntsz = sz;
    }

    /**
     * @brief backlight control
     * 
     * @param on true - ON, false - OFF
     */
    void backLight(bool on);

    /**
     * @brief deletes the contents of the rendering buffer
     * 
     */
    void cls();

    /**
     * @brief Draw a point in the display buffer
     * 
     * @param x  - X pixel coordinates
     * @param y - Y pixel coordinates
     * @param bg - background, if the pixel is to be rendered black it is necessary to set false, if the pixel is to be rendered white then to true (background color) 
     */
    void plot(int16_t x, int16_t y, bool bg = false);

    /**
     * @brief fill in the area from the given position, given by the width and height
     * 
     * @param x - X pixel coordinates
     * @param y - Y pixel coordinates
     * @param w - area width in pixels
     * @param h - area height in pixels
     * @param bg - background, if the pixel is to be rendered black it is necessary to set false, if the pixel is to be rendered white then to true (background color)
     */
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool bg = false);

    /**
     * @brief draw line from point to point
     * 
     * @param x0 - X initial coordinates of the point
     * @param y0 - Y initial coordinates of the point
     * @param x1 - X end point coordinate
     * @param y1 - Y end point coordinate
     * @param bg - background, if the pixel is to be rendered black it is necessary to set false, if the pixel is to be rendered white then to true (background color)
     */
    void draw(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool bg = false);
    
    /**
     * @brief draw a character at the position
     * 
     * @param x -  X pixel coordinates
     * @param y -  Y pixel coordinates
     * @param c -  ASCII character
     * @param bg - background, if the pixel is to be rendered black it is necessary to set false, if the pixel is to be rendered white then to true (background color)
     * @param size - the value 1 is the default value corresponding to the size from the glyph table. A value of 2 or more determines a multiple of the pixel size. 
     */
    void charAt(int16_t x, int16_t y, unsigned char c, bool bg = false, uint8_t size = 1);
    
    /**
     * @brief draw a character into the buffer at the position given by the previous text output, e.g. function at, print...
     * 
     * @param c ASCII character
     */
    void putc(uint8_t c);

    /**
     * @brief Presets the character size for the output 
     * 
     * @param sz - the value 1 is the default value corresponding to the size from the glyph table. A value of 2 or more determines a multiple of the pixel size. 
     * @return Lcd5110& 
     */
    Lcd5110 &withSize(uint8_t sz)
    {
        _printSize = sz;
        return *this;
    }

    /**
     * @brief Prints text to the buffer at the last known position
     * 
     * @param strz - zero-terminated character ascii string
     * @return Lcd5110& 
     */
    Lcd5110 &print(const char *strz);
    
    /**
     * @brief Set the character print position to the coordinates
     * 
     * @param x - X pixel coordinates
     * @param y - Y pixel coordinates
     * @return Lcd5110& 
     */
    Lcd5110 &at(int16_t x, int16_t y);
    
    /**
     * @brief Prints an integer into the buffer at the last known position
     * 
     * @param number  - number
     * @return Lcd5110& 
     */
    Lcd5110 &print(int32_t number);

    /**
     * @brief transfers the contents of the internal buffer to the display (custom display)
     * 
     */
    void refresh();

private:

    void cmd(uint8_t cm);

private:

// extend mode
#define PCD8544_EXTEND_INSTRUCTION 0x21
#define PCD8544_BIAS 0x10
#define PCD8544_VOP 0x80

// normal mode
#define PCD8544_INSTRUCTION 0x20
#define PCD8544_NORMAL 0x0C
#define PCD8544_YADDR 0x40
#define PCD8544_XADDR 0x80

    const unsigned char *_lcdFont{nullptr}; ///< glyphs table
    spi_inst_t *_spi{nullptr};              ///< SPI interface
    uint8_t _rst;                           ///< RST pin
    uint8_t _ce;                            ///< CE pin
    uint8_t _dc;                            ///< DC pin
    uint8_t _din;                           ///< DIN pin
    uint8_t _clk;                           ///< CLK pin
    uint8_t _light;                         ///< LIGHT pin 
    uint8_t _buffer[maxX * maxY / 8];       ///< internal buffer (image)
    uint8_t _x{0};                          ///< last known X coordinates of the character print
    uint8_t _y{0};                          ///< last known Y coordinates of the character print
    uint8_t _hg{0};                         ///< glyph height
    uint8_t _wg{0};                         ///< glyph width
    unsigned char _firstChar{' '};          ///< firts character in the font table
    uint8_t _fntsz{0};                      ///< glyph table size (number of glyphs) 
    uint8_t _printSize{1};                  ///< preffered print magnification
};
