#pragma once
#include <vector>
#include <cmath>

// Cambio de tono en tiempo real: dos cabezas de lectura con crossfade (ventana fija).
class PitchShifter
{
public:
    void prepare(int windowSamples)
    {
        win = (float) windowSamples;
        int n = 1;
        while (n < windowSamples * 2 + 16) n <<= 1;
        buf.assign((size_t) n, 0.0f);
        mask = n - 1;
        w = 0;
        phase = 0.0;
    }

    float process(float in, float ratio)
    {
        buf[(size_t) w] = in;
        phase += (1.0 - (double) ratio) / (double) win;
        phase -= std::floor(phase);
        double p2 = phase + 0.5;
        p2 -= std::floor(p2);

        const float twoPi = 6.28318530718f;
        const float g1 = 0.5f - 0.5f * std::cos(twoPi * (float) phase);
        const float g2 = 0.5f - 0.5f * std::cos(twoPi * (float) p2);

        const float out = g1 * read(phase * (double) win + 2.0) + g2 * read(p2 * (double) win + 2.0);
        w = (w + 1) & mask;
        return out;
    }

private:
    float read(double delay) const
    {
        const double pos = (double) w - delay;
        const int i0 = (int) std::floor(pos);
        const float frac = (float) (pos - (double) i0);
        const float a = buf[(size_t) (i0 & mask)];
        const float b = buf[(size_t) ((i0 + 1) & mask)];
        return a + frac * (b - a);
    }

    std::vector<float> buf;
    int mask = 0, w = 0;
    float win = 1323.0f;
    double phase = 0.0;
};

class DelayLine
{
public:
    void prepare(int delaySamples)
    {
        d = delaySamples;
        int n = 1;
        while (n < delaySamples + 8) n <<= 1;
        buf.assign((size_t) n, 0.0f);
        mask = n - 1;
        w = 0;
    }
    float process(float in)
    {
        buf[(size_t) w] = in;
        const float out = buf[(size_t) ((w - d) & mask)];
        w = (w + 1) & mask;
        return out;
    }
private:
    std::vector<float> buf;
    int mask = 0, w = 0, d = 0;
};
