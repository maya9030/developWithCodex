/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MPMtracker.h"
#include <array>
#include <vector>

//==============================================================================
/**
*/
class TestProjectWithCodexAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TestProjectWithCodexAudioProcessor();
    ~TestProjectWithCodexAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Parameter setters used by the editor
    void setQ (float newQ);
    void setDrive (float newDrive);
    void setMode (int modeIndex);

    float getCurrentF0() const;
    bool getFFTData (std::vector<float>& data);

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    juce::dsp::LadderFilter<float> ladder;
    MPMtracker pitchTracker;
    std::atomic<float> q { 0.0f };
    std::atomic<float> drive { 0.0f };
    std::atomic<float> currentF0 { 0.0f };
    juce::LinearSmoothedValue<float> cutoffSmooth;
    juce::LinearSmoothedValue<float> qSmooth;
    juce::LinearSmoothedValue<float> driveSmooth;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    // FFT data for frequency analysis
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    std::array<float, fftSize> fifo{};
    std::array<float, fftSize * 2> fftData{};
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    juce::dsp::FFT forwardFFT { fftOrder };
    juce::dsp::WindowingFunction<float> window { fftSize, juce::dsp::WindowingFunction<float>::hann };
    void pushNextSampleIntoFifo (float sample);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestProjectWithCodexAudioProcessor)
};
