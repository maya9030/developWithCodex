#include "MPMtracker.h"
#include <algorithm>

MPMtracker::MPMtracker(float sr) : sampleRate(sr) {}

void MPMtracker::setSampleRate(float newSampleRate)
{
    sampleRate = newSampleRate;
}

float MPMtracker::getSampleRate() const
{
    return sampleRate;
}

float MPMtracker::getPitch(const float* samples, int numSamples)
{
    if (samples == nullptr || numSamples < 2)
        return 0.0f;

    const int maxTau = numSamples / 2;
    if (maxTau <= 1)
        return 0.0f;

    std::vector<float> nsdf(static_cast<size_t>(maxTau), 0.0f);

    for (int tau = 0; tau < maxTau; ++tau)
    {
        float acf = 0.0f;
        float norm = 0.0f;

        for (int i = 0; i < numSamples - tau; ++i)
        {
            const float x1 = samples[i];
            const float x2 = samples[i + tau];
            acf += x1 * x2;
            norm += x1 * x1 + x2 * x2;
        }

        nsdf[static_cast<size_t>(tau)] = (norm > 0.0f) ? (2.0f * acf / norm) : 0.0f;
    }

    int tauMax = 0;
    float maxVal = -1.0f;
    bool pastZero = false;

    for (int tau = 1; tau < maxTau - 1; ++tau)
    {
        const float val = nsdf[static_cast<size_t>(tau)];

        if (!pastZero && val < 0.0f)
            pastZero = true;

        if (pastZero && val > nsdf[static_cast<size_t>(tau - 1)] &&
            val >= nsdf[static_cast<size_t>(tau + 1)] && val > maxVal)
        {
            maxVal = val;
            tauMax = tau;
        }
    }

    if (tauMax == 0 || maxVal <= 0.0f)
        return 0.0f;

    float betterTau = static_cast<float>(tauMax);

    if (tauMax > 0 && tauMax < maxTau - 1)
    {
        const float s0 = nsdf[static_cast<size_t>(tauMax - 1)];
        const float s1 = nsdf[static_cast<size_t>(tauMax)];
        const float s2 = nsdf[static_cast<size_t>(tauMax + 1)];
        const float denom = 2.0f * (2.0f * s1 - s2 - s0);

        if (denom != 0.0f)
            betterTau += (s2 - s0) / denom;
    }

    return sampleRate / betterTau;
}

