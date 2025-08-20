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
    qSlider.setValue (0.0f);
    qSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (qSlider);

    // Drive slider
    driveSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setRange (1.0f, 20.0f, 0.01f);
    driveSlider.setValue (1.0f);
    driveSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (driveSlider);

    // Mode tabs
    modeTabs.addTab ("BPF12", juce::Colours::lightgrey, new juce::Component(), true);
    modeTabs.addTab ("BPF24", juce::Colours::lightgrey, new juce::Component(), true);
    addAndMakeVisible (modeTabs);
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

    auto sliderArea = area.reduced (10);
    auto sliderWidth = sliderArea.getWidth() / 2;
    qSlider.setBounds (sliderArea.removeFromLeft (sliderWidth).reduced (10));
    driveSlider.setBounds (sliderArea.reduced (10));
}
