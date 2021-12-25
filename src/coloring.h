#pragma once

/*
    This file contains some base pixel and color types.
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)  // nonstandard extension used : nameless struct/union
#endif

#include <array>
#include <cstdint>

//
// Color representation
// Really this should be some CIE generic representation
// Color is represented with values from 0..1, rather than
// being limited to the 0-255 range of a pixel.
//
// WAA - maybe alpha does not belong in color representation
//
struct ColorRgba {
    double red = 0.0f;
    double green = 0.0f;
    double blue = 0.0f;
    double alpha = 0.0f;

    ColorRgba() {}
    ColorRgba(const uint8_t r, const uint8_t g, const uint8_t b) :red((double)r / 255.0), green((double)g / 255.0), blue((double)b / 255.0), alpha(1.0) {}
    ColorRgba(double r, double g, double b, double a) :red(r), green(g), blue(b), alpha(a) {}
    ColorRgba(double r, double g, double b) :red(r), green(g), blue(b), alpha(1.0) {}

    ColorRgba operator * (double s) { return { red * s, green * s, blue * s, alpha }; }
    ColorRgba operator / (double s) { return { red / s, green / s, blue / s, alpha }; }
    ColorRgba operator + (const ColorRgba& rhs) { return { red + rhs.red, green + rhs.green, blue + rhs.blue, 1.0 }; }
    ColorRgba& operator += (const ColorRgba& rhs) {
        red += rhs.red;
        green += rhs.green;
        blue += rhs.blue;
        alpha += 1;
        return *this;
    }
};

/*

In this case, PixelRGBA has 4 byte sized(8 - bit) values to represent
each of the R, G, B, A values.  Some modern day monitors and graphics
systems in general, are capable of using 10 - bit or even 16 - bit values.
As this is not universally yet, we'll stick to 8-bit components.

If you were to set a PixelRGBA by its integer value, it would layout
on a little endian system as follows :

0xAARRGGBB
*/
typedef union PixelRGBA {
    uint32_t intValue;
    uint8_t data[4];
    struct {
        uint32_t blue : 8;
        uint32_t green : 8;
        uint32_t red : 8;
        uint32_t alpha : 8;
    };

    // default constructor, transparent
    PixelRGBA() :intValue(0) {}

    // Copy constructor
    PixelRGBA(const PixelRGBA& rhs) :red(rhs.red), blue(rhs.blue), green(rhs.green), alpha(rhs.alpha) {}
    PixelRGBA(const ColorRgba& rhs) :red(rhs.red * 255), blue(rhs.blue * 255), green(rhs.green * 255), alpha(rhs.alpha * 255) {}

    // Convenience constructors
    PixelRGBA(int r, int g, int b, int a) :red(r), green(g), blue(b), alpha(a) {}
    PixelRGBA(int r, int g, int b) :red(r), green(g), blue(b), alpha(255) {}
    PixelRGBA(uint32_t val) : intValue(val) {}

    // When you need a COLORREF to be compatible with 
    // some Windows GDI routines
    operator ColorRgba () { return { (double)red / 255.0,(double)green / 255.0,(double)blue / 255.0,(double)alpha / 255.0 }; }

    uint32_t toCOLORREF() { return red | (green << 8) | (blue << 16); }
    uint8_t lum() { return static_cast<uint8_t>((float)red * 0.2225f + (float)green * 0.7154f + (float)blue * 0.0721f); }

} PixelRGBA;
