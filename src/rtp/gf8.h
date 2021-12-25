#pragma once

#include <limits>
#include <cstdint>

/// (*) Evariste Galois 1811 (Bourg-La-Reine, near Paris) - 1832 (Paris)
/// 
struct GF8
{
private:
    int32_t i;

public:
    static uint8_t gf_log[UCHAR_MAX + 1];
    static uint8_t gf_exp[(UCHAR_MAX + 1) * 2];
    static uint8_t inverse[UCHAR_MAX + 1];

private:
    static GF8 empty;
    /*
        static GF8()
        {
            GF8.empty = new GF8(0);
            empty.i = -1;

            #region Initialize Reference Tables
            UInt32 primitive_polynomial = 285;

            UInt32 mask = 1;
            gf_exp[8] = 0;

            for (byte i = 0; i < byte.MaxValue; i++)
            {
                gf_exp[i] = (byte)mask;
                gf_log[mask] = (byte)i;

                mask <<= 1;
                if ((mask & 256) != 0)
                    mask = mask ^ primitive_polynomial;
            }

            // set the extended gf_exp values for fast multiply
            for (UInt32 i = 0; i < byte.MaxValue; i++)
                gf_exp[i + byte.MaxValue] = gf_exp[i];

            inverse[0] = 0;
            inverse[1] = 1;

            for (UInt32 i = 2; i <= byte.MaxValue; i++)
                inverse[i] = gf_exp[byte.MaxValue - gf_log[i]];
            #endregion
        }
        */




public:
    GF8(uint8_t i) :i(i) {}

    uint8_t value()
    {
        return (uint8_t)i;
    }

    GF8 Power(uint8_t exponent)
        {
            if (value() == 0)
                return *this;

            if (exponent == 0)
                this = 1;

            GF8 original = this;

            for (int i = 1; i < exponent; i++)
                this *= original;

            return this;
        }


        override std::string toString()
        {
            if (i == -1)
                return "GF8.Empty";
            else
                return i.ToString(CultureInfo.InvariantCulture);
        }

        static operator GF8(Int32 i)
        {
            if (i < 0 || i > byte.MaxValue)
                throw new ArgumentException();

            return GF8((byte)i);
        }

        static operator GF8(uint8_t i)
        {
            return new GF8(i);
        }

        static GF8 operator + (GF8 a, GF8 b)
        {
            return GF8.Add(a.Value, b.Value);
        }

        static GF8 operator -(GF8 a, GF8 b)
        {
            return GF8.Add(a.Value, b.Value);
        }

        static GF8 operator *(GF8 a, GF8 b)
        {
            return GF8.Multiply(a.Value, b.Value);
        }

        static GF8 operator /(GF8 a, GF8 b)
        {
            return GF8.Divide(a.Value, b.Value);
        }

        static GF8 Empty
        {
            return GF8::empty;
        }

        static uint8_t Add(uint8_t a, uint8_t b)
        {
            return (uint8_t)(a ^ b);
        }

        static uint8_t Multiply(uint8_t a, uint8_t b)
        {
            if (a == 0 || b == 0) return 0;

            return gf_exp[unchecked(gf_log[a] + gf_log[b])];
        }

        static uint8_t Divide(uint8_t numerator, uint8_t denominator)
        {
            if (denominator == 0)
                throw new DivideByZeroException();

            if (numerator == 0)
                return 0;

            return gf_exp[unchecked(gf_log[numerator] + gf_log[inverse[denominator]])];
        }

        static uint8_t Modnn(Int32 x)
        {
            while (x >= byte.MaxValue)
            {
                x -= byte.MaxValue;
                x = (byte)((x >> 8) + (x & byte.MaxValue));
            }
            return (byte)x;
        }

        static uint8_t Power(byte x, UInt32 exponent)
        {
            if (x == 0)
                return 0;

            if (exponent == 0)
                return 1;

            uint8_t ret = x;
            for (int i = 1; i < exponent; i++)
                ret = Multiply(ret, x);

            return ret;
        }

    }
