#pragma once

#include <vector>

/**
    Calculates the fundamental frequency of a signal using the
    McLeod Pitch Method (MPM).
*/
class MPMtracker
{
public:
    /** Creates an MPMtracker with the given sample rate. */
    explicit MPMtracker(float sampleRate = 44100.0f);

    /** Sets the sample rate to use for pitch estimation. */
    void setSampleRate(float newSampleRate);

    /** Returns the current sample rate. */
    float getSampleRate() const;

    /**
        Estimates the fundamental frequency of the provided buffer.
        
        @param samples     Pointer to the audio samples.
        @param numSamples  Number of samples in the buffer.
        @returns Estimated pitch in Hz or 0.0f if no pitch was found.
    */
    float getPitch(const float* samples, int numSamples);

private:
    float sampleRate;
};

