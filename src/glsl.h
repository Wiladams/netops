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

#include "coloring.h"
#include "maths.h"

// 
// Pixel coordinates represent a location within a framebuffer of pixels.
// These are the values used to place a pixel value into that frame, so
// they are integer values.
//
struct PixelCoord {
    int x;
    int y;

    PixelCoord() : x(0), y(0) {}
    PixelCoord(const int x, const int y) :x(x), y(y) {}
};

//
// TextureCoord
// Used to get a value out of a sampler.  These values
// range from 0.0 .. 1.0 and are floating point numbers.
// This reflects the parametric nature of those samplers.
struct TextureCoord {
    double s;
    double t;
};

// The ISample interface is meant to support a generic interface
// for generating color values based on parameters.
// Doing this makes it easy to create color values for different
// rendering situations, without limiting ourselves to bitmaps.
//
// You can create a sample to return a certain type using the 
// teamplate class Sample<T>.
//
// struct SolidColorSample2D : ISample2D<PixelRGBA>
//

// A 1 dimensional sampler
// good for when you're sampling across a linear thing
// like a line or curve.
template <typename T>
struct ISample1D
{
    virtual T getValue(double u, const PixelCoord& p) const = 0;
    virtual T operator()(double u, const PixelCoord& p) const
    {
        return getValue(u, p);
    }
};

// A 2 dimentional sampler
// good for filling in an entire picture frame
template <typename T>
struct ISample2D
{
    virtual T getValue(double u, double v, const PixelCoord& p) const = 0;
    virtual T operator()(double u, double v, const PixelCoord& p) const
    {
        return getValue(u, v, p);
    }
};

// A 3 dimensional sampler
template <typename T>
struct ISample3D
{
    virtual T getValue(double u, double v, double w, const PixelCoord& p) const = 0;
    virtual T operator()(double u, double v, double w, const PixelCoord& p) const
    {
        return getValue(u, v, w, p);
    }
};

template <typename T>
struct ISampleRGBA :
    public ISample1D<T>,
    public ISample2D<T>,
    public ISample3D<T>
{

};


// 
// GrayConverter
// Convert an RGB value to a grayscale value
class GrayConverter
{
    // Based on old NTSC
    //static float redcoeff = 0.299f;
    //static float greencoeff = 0.587f;
    //static float bluecoeff = 0.114f;

    // New CRT and HDTV phosphors
    const float redcoeff = 0.2225f;
    const float greencoeff = 0.7154f;
    const float bluecoeff = 0.0721f;

    // use lookup tables to save ourselves from multiplications later
    std::array<uint8_t, 256> redfactor;
    std::array<uint8_t, 256> greenfactor;
    std::array<uint8_t, 256> bluefactor;

public:
    GrayConverter() :GrayConverter(0.2225f, 0.7154f, 0.0721f) {}

    // Create an instance, initializing with the coefficients you desire
    GrayConverter(float rcoeff, float gcoeff, float bcoeff)
        :redcoeff(rcoeff), greencoeff(gcoeff), bluecoeff(bcoeff)
    {
        // construct lookup tables
        for (int counter = 0; counter < 256; counter++)
        {
            redfactor[counter] = (uint8_t)maths::Min(56, (int)maths::Floor((counter * redcoeff) + 0.5f));
            greenfactor[counter] = (uint8_t)maths::Min(181, (int)maths::Floor((counter * greencoeff) + 0.5f));
            bluefactor[counter] = (uint8_t)maths::Min(18, (int)maths::Floor((counter * bluecoeff) + 0.5f));
        }
    }

    uint8_t toLuminance(uint8_t r, uint8_t g, uint8_t b) const
    {
        return redfactor[r] + greenfactor[g] + bluefactor[b];
    }

    uint8_t toLuminance(const PixelRGBA p) const
    {
        return redfactor[p.red] + greenfactor[p.green] + bluefactor[p.blue];
    }
};


// Some useful functions
// return a pixel value from a ISample2D based on the texture coordinates
// this is purely a convenience to match what you can do in OpenGL GLSL language
inline PixelRGBA texture2D(ISample2D<PixelRGBA> &tex0, const TextureCoord& st) noexcept
{
    return tex0.getValue(st.s, st.t, {});
}
