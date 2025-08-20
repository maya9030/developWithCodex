/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TestProjectWithCodexAudioProcessorEditor::TestProjectWithCodexAudioProcessorEditor (TestProjectWithCodexAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    // Q slider
    qSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    qSlider.setRange (0.0f, 1.0f, 0.001f);
    qSlider.onValueChange = [this] { audioProcessor.setQ ((float) qSlider.getValue()); };
    qSlider.setValue (0.0f);
    qSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 60, 20);
    addAndMakeVisible (qSlider);
    qLabel.setText ("Q", juce::dontSendNotification);
    qLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (qLabel);


    // Drive slider
    driveSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setRange (0.0f, 20.0f, 0.01f);
    driveSlider.onValueChange = [this] { audioProcessor.setDrive ((float) driveSlider.getValue()); };
    driveSlider.setValue (0.0f);
    driveSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 60, 20);
    addAndMakeVisible (driveSlider);
    driveLabel.setText ("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (driveLabel);

    // Mode tabs
    modeTabs.addTab ("BPF12", juce::Colours::lightgrey, new juce::Component(), true);
    modeTabs.addTab ("BPF24", juce::Colours::lightgrey, new juce::Component(), true);
    modeTabs.onCurrentTabChanged = [this] (int index, const juce::String&) { audioProcessor.setMode (index); };
    addAndMakeVisible (modeTabs);

    // Fundamental frequency label
    f0Label.setJustificationType (juce::Justification::centred);
    f0Label.setText ("F0: 0.00 Hz", juce::dontSendNotification);
    addAndMakeVisible (f0Label);

    startTimerHz (30);
}

TestProjectWithCodexAudioProcessorEditor::~TestProjectWithCodexAudioProcessorEditor()
{
}

//==============================================================================
void TestProjectWithCodexAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void TestProjectWithCodexAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto tabHeight = 30;
    modeTabs.setBounds (area.removeFromTop (tabHeight));

    auto freqHeight = 20;
    f0Label.setBounds (area.removeFromBottom (freqHeight));

    auto sliderArea = area.reduced (10);
    auto sliderWidth = sliderArea.getWidth() / 2;

    auto qArea = sliderArea.removeFromLeft (sliderWidth).reduced (10);
    auto labelHeight = 20;
    qLabel.setBounds (qArea.removeFromBottom (labelHeight));
    qSlider.setBounds (qArea);

    auto dArea = sliderArea.reduced (10);
    driveLabel.setBounds (dArea.removeFromBottom (labelHeight));
    driveSlider.setBounds (dArea);
}

void TestProjectWithCodexAudioProcessorEditor::timerCallback()
{
    auto freq = audioProcessor.getCurrentF0();
    f0Label.setText ("F0: " + juce::String (freq, 2) + " Hz", juce::dontSendNotification);
}
