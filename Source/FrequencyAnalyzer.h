#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class FrequencyAnalyzer : public juce::Component, private juce::Timer
{
public:
    explicit FrequencyAnalyzer (TestProjectWithCodexAudioProcessor& p);

    void paint (juce::Graphics& g) override;
    void timerCallback() override;

private:
    TestProjectWithCodexAudioProcessor& processor;
    juce::Path spectrumPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyAnalyzer)
};

