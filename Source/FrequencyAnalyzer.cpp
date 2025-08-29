#include "FrequencyAnalyzer.h"

FrequencyAnalyzer::FrequencyAnalyzer (TestProjectWithCodexAudioProcessor& p)
    : processor (p)
{
    startTimerHz (30);
}

void FrequencyAnalyzer::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colours::green);
    g.strokePath (spectrumPath, juce::PathStrokeType (1.0f));
}

void FrequencyAnalyzer::timerCallback()
{
    std::vector<float> data;
    if (processor.getFFTData (data))
    {
        spectrumPath.clear();
        auto width = static_cast<float> (getWidth());
        auto height = static_cast<float> (getHeight());
        auto binWidth = width / (float) data.size();

        for (size_t i = 0; i < data.size(); ++i)
        {
            auto level = juce::Decibels::gainToDecibels (data[i]) - 100.0f;
            auto y = juce::jmap (level, -100.0f, 0.0f, height, 0.0f);
            if (i == 0)
                spectrumPath.startNewSubPath (0.0f, y);
            else
                spectrumPath.lineTo ((float) i * binWidth, y);
        }

        repaint();
    }
}

