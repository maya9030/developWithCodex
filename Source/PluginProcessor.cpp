/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
TestProjectWithCodexAudioProcessor::TestProjectWithCodexAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

TestProjectWithCodexAudioProcessor::~TestProjectWithCodexAudioProcessor()
{
}

//==============================================================================
const juce::String TestProjectWithCodexAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TestProjectWithCodexAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TestProjectWithCodexAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TestProjectWithCodexAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TestProjectWithCodexAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TestProjectWithCodexAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TestProjectWithCodexAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TestProjectWithCodexAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TestProjectWithCodexAudioProcessor::getProgramName (int index)
{
    return {};
}

void TestProjectWithCodexAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TestProjectWithCodexAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());

    ladder.prepare (spec);
    ladder.setMode (juce::dsp::LadderFilter<float>::Mode::BPF12);
    ladder.reset();

    pitchTracker.setSampleRate (static_cast<float> (sampleRate));
}

void TestProjectWithCodexAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TestProjectWithCodexAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TestProjectWithCodexAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Pitch detection from first channel
    float f0 = pitchTracker.getPitch (buffer.getReadPointer (0), buffer.getNumSamples());
    if (f0 > 0.0f)
        ladder.setCutoffFrequencyHz (f0);

    ladder.setResonance (q.load());
    float dB = std::pow (10.0f, drive.load() / 10.0f);
    ladder.setDrive (dB);

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    ladder.process (context);
}

void TestProjectWithCodexAudioProcessor::setQ (float newQ)
{
    q = newQ;
}

void TestProjectWithCodexAudioProcessor::setDrive (float newDrive)
{
    drive = newDrive;
}

//==============================================================================
bool TestProjectWithCodexAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TestProjectWithCodexAudioProcessor::createEditor()
{
    return new TestProjectWithCodexAudioProcessorEditor (*this);
}

//==============================================================================
void TestProjectWithCodexAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TestProjectWithCodexAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TestProjectWithCodexAudioProcessor();
}
