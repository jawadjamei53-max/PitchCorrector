#pragma once
#include <juce_core/juce_core.h>

namespace Scales
{
    inline const juce::StringArray keyNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    inline const juce::StringArray scaleNames { "Chromatic", "Major", "Minor", "Harmonic Minor",
                                                "Pentatonic Major", "Pentatonic Minor", "Dorian", "Mixolydian" };

    inline const int* intervals(int scale, int& count)
    {
        static const int chromatic[] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
        static const int major[]     = { 0,2,4,5,7,9,11 };
        static const int minor[]     = { 0,2,3,5,7,8,10 };
        static const int harmonic[]  = { 0,2,3,5,7,8,11 };
        static const int pentMaj[]   = { 0,2,4,7,9 };
        static const int pentMin[]   = { 0,3,5,7,10 };
        static const int dorian[]    = { 0,2,3,5,7,9,10 };
        static const int mixo[]      = { 0,2,4,5,7,9,10 };
        switch (scale)
        {
            case 1: count = 7;  return major;
            case 2: count = 7;  return minor;
            case 3: count = 7;  return harmonic;
            case 4: count = 5;  return pentMaj;
            case 5: count = 5;  return pentMin;
            case 6: count = 7;  return dorian;
            case 7: count = 7;  return mixo;
            default: count = 12; return chromatic;
        }
    }

    // allowed[pc] = true si la nota (0=C..11=B) esta en la escala
    inline void buildMask(int key, int scale, bool* allowed)
    {
        for (int i = 0; i < 12; ++i) allowed[i] = false;
        int n = 0;
        const int* iv = intervals(scale, n);
        for (int i = 0; i < n; ++i) allowed[(key + iv[i]) % 12] = true;
    }
}
