
//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   lcd5110.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "lcd5110.h"

void Lcd5110::init(uint8_t bias, uint8_t contrast)
{

	spi_init(_spi, 3000000);
	gpio_set_function(_din, GPIO_FUNC_SPI);
	gpio_set_function(_clk, GPIO_FUNC_SPI);

	if (_ce) gpio_init(_ce);
	if (_ce) gpio_set_dir(_ce, GPIO_OUT);

	gpio_init(_dc);
	gpio_set_dir(_dc, GPIO_OUT);

	gpio_init(_rst);
	gpio_set_dir(_rst, GPIO_OUT);

	if (_light) gpio_init(_light);
	if (_light) gpio_set_dir(_light, GPIO_OUT);

	// reset pulse
	gpio_put(_rst, 0);
	sleep_ms(200);
	gpio_put(_rst, 1);

	// enxtened instruction
	cmd(PCD8544_EXTEND_INSTRUCTION);
	cmd(PCD8544_BIAS | bias);
	cmd(PCD8544_VOP | contrast);

	// switch to normal mode mode
	cmd(PCD8544_INSTRUCTION);
	cmd(PCD8544_NORMAL);
	cls();
}

void Lcd5110::backLight(bool on)
{
	if (_light) gpio_put(_light, !on);
}

void Lcd5110::cls()
{
	memset(&_buffer, 0, sizeof(_buffer));
	refresh();
}

void Lcd5110::refresh()
{

	for (auto page = 0; page < 6; page++)
	{
		cmd(PCD8544_YADDR | page);
		uint8_t col = 0;
		uint8_t maxcol = maxX - 1;
		cmd(PCD8544_XADDR | col);
		gpio_put(_dc, 1);
		if (_ce) gpio_put(_ce, 0);
		for (auto i = 0; i <= maxcol; i++)
		{
			spi_write_blocking(_spi, &_buffer[(maxX * page) + i], 1);
		}
		if (_ce) gpio_put(_ce, 1);
	}
	cmd(PCD8544_YADDR);
}

void Lcd5110::cmd(uint8_t cm)
{
	gpio_put(_dc, 0);
	if (_ce) gpio_put(_ce, 0);
	spi_write_blocking(_spi, &cm, 1);
	if (_ce) gpio_put(_ce, 1);
}

void Lcd5110::plot(int16_t x, int16_t y, bool bg)
{

	do
	{
		if ((x < 0) || (x >= maxX) || (y < 0) || (y >= maxY))
			break;

		x = maxX - 1 - x;
		y = maxY - 1 - y;

		if ((x < 0) || (x >= maxX) || (y < 0) || (y >= maxY))
			break;

		if (!bg)
			_buffer[x + (y / 8) * maxX] |= (1 << (y % 8));
		else
			_buffer[x + (y / 8) * maxX] &= ~(1 << (y % 8));

	} while (false);
}

void Lcd5110::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool bg)
{
	for (auto i = x; i < x + w; i++)
	{
		draw(i, y, i, y + h - 1, bg);
	}
}

void Lcd5110::draw(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool bg)
{
	int16_t ystep = 0;
	int16_t derivy = abs(y1 - y0) > abs(x1 - x0);

	if (derivy)
	{
		auto op = x0;
		x0 = y0;
		y0 = op;
		op = x1;
		x1 = y1;
		y1 = op;
	}

	if (x0 > x1)
	{
		auto op = x0;
		x0 = x1;
		x1 = op;
		op = y1;
		y0 = y1;
		y1 = op;
	}

	auto dx = x1 - x0;
	auto dy = abs(y1 - y0);

	auto err = dx / 2;

	if (y0 < y1)
	{
		ystep = 1;
	}
	else
	{
		ystep = -1;
	}

	for (; x0 <= x1; x0++)
	{
		if (derivy)
		{
			plot(y0, x0, bg);
		}
		else
		{
			plot(x0, y0, bg);
		}
		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}
}

void Lcd5110::charAt(int16_t x, int16_t y, unsigned char ch, bool bg, uint8_t size)
{

	do
	{
		if (!_lcdFont)
			break;
		if (x >= maxX)
			break;
		if (y >= maxY)
			break;
		if ((x + (_wg + 1) * size - 1) < 0)
			break;
		if ((y + _hg * size - 1) < 0)
			break;
		if (ch < _firstChar)
			break;
		if (ch >= (_firstChar + _fntsz))
			break;
		auto c = ch - _firstChar;
		auto bytesPerColumn = ceil((float)_hg / 8);
		auto po = c * uint8_t(_wg * bytesPerColumn);

		for (int8_t i = 0; i < _wg; i++)
		{
			if (i != 0)
				po += 1;
			uint8_t column = _lcdFont[po];
			for (int8_t j = 0; j < _hg; j++, column >>= 1)
			{
				if (j != 0 && j % 8 == 0)
				{
					// next byte
					po++;
					column = _lcdFont[po];
				}

				if (column & 1)
				{

					if (size == 1)
					{
						plot(x + i, y + j, bg);
					}
					else
					{
						fillRect(x + i * size, y + j * size, size, size, bg);
					}
				}
				else
				{
					if (size == 1)
					{
						plot(x + i, y + j, !bg);
					}
					else
					{
						fillRect(x + i * size, y + j * size, size, size, !bg);
					}
				}
			}
		}

	} while (false);
}

void Lcd5110::putc(uint8_t c)
{
	do
	{
		if (c == '\n')
		{
			_x = 0;
			_y += _hg * _printSize;
			break;
		}
		if (c == '\r')
			break;
		charAt(_x, _y, c, false, _printSize);
		_x += _wg * _printSize + 1;
	} while (false);
}

Lcd5110 &Lcd5110::print(const char *strz)
{
	for (uint8_t i = 0; strz[i] != '\0'; i++)
	{
		putc((uint8_t)strz[i]);
	}
	return *this;
}

Lcd5110 &Lcd5110::print(int32_t number)
{
	char str[16];
	sprintf(str, "%d", number);
	print(str);
	return *this;
}

Lcd5110 &Lcd5110::at(int16_t x, int16_t y)
{
	_x = x;
	_y = y;
	return *this;
}