/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TestProjectWithCodexAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    TestProjectWithCodexAudioProcessorEditor (TestProjectWithCodexAudioProcessor&);
    ~TestProjectWithCodexAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TestProjectWithCodexAudioProcessor& audioProcessor;

    // UI components
    juce::Slider qSlider;
    juce::Slider driveSlider;
    juce::TabbedComponent modeTabs { juce::TabbedButtonBar::TabsAtTop };
    juce::Label qLabel;
    juce::Label driveLabel;
    juce::Label f0Label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestProjectWithCodexAudioProcessorEditor)
};
