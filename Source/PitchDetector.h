#pragma once
#include <vector>
#include <cmath>

// Deteccion de tono YIN (implementacion propia)
class PitchDetector
{
public:
    void prepare(int maxTau)
    {
        diff.assign((size_t) maxTau + 2, 0.0f);
        cmnd.assign((size_t) maxTau + 2, 1.0f);
    }

    // x debe tener al menos W + tauMax muestras. Devuelve Hz (0 si no hay tono).
    float detect(const float* x, int W, int tauMin, int tauMax, double sr, float threshold, float& confidence)
    {
        for (int tau = 1; tau <= tauMax; ++tau)
        {
            float sum = 0.0f;
            for (int j = 0; j < W; ++j)
            {
                const float d = x[j] - x[j + tau];
                sum += d * d;
            }
            diff[(size_t) tau] = sum;
        }

        cmnd[0] = 1.0f;
        float running = 0.0f;
        for (int tau = 1; tau <= tauMax; ++tau)
        {
            running += diff[(size_t) tau];
            cmnd[(size_t) tau] = running > 1e-12f ? diff[(size_t) tau] * (float) tau / running : 1.0f;
        }

        int found = -1;
        for (int t = tauMin; t < tauMax; ++t)
        {
            if (cmnd[(size_t) t] < threshold)
            {
                while (t + 1 < tauMax && cmnd[(size_t) t + 1] < cmnd[(size_t) t]) ++t;
                found = t;
                break;
            }
        }

        if (found < 2 || found >= tauMax) { confidence = 0.0f; return 0.0f; }

        const float s0 = cmnd[(size_t) found - 1], s1 = cmnd[(size_t) found], s2 = cmnd[(size_t) found + 1];
        const float denom = 2.0f * (2.0f * s1 - s2 - s0);
        const float better = (float) found + (std::abs(denom) > 1e-9f ? (s2 - s0) / denom : 0.0f);

        confidence = 1.0f - s1;
        return (float) (sr / better);
    }

private:
    std::vector<float> diff, cmnd;
};
