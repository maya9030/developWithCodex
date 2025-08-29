/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <vector>
#include <array>

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

    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (spec.numChannels, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
    oversampler->initProcessing (static_cast<size_t> (samplesPerBlock));

    cutoffSmooth.reset (sampleRate, 0.05);
    qSmooth.reset (sampleRate, 0.05);
    driveSmooth.reset (sampleRate, 0.05);
    cutoffSmooth.setCurrentAndTargetValue (0.0f);
    qSmooth.setCurrentAndTargetValue (0.0f);
    driveSmooth.setCurrentAndTargetValue (0.0f);

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
        cutoffSmooth.setTargetValue (f0);
    else
        cutoffSmooth.setTargetValue (0.0f);

    currentF0 = f0 > 0.0f ? f0 : 0.0f;

    qSmooth.setTargetValue (q.load());
    driveSmooth.setTargetValue (drive.load());

    juce::dsp::AudioBlock<float> block (buffer);
    auto oversampledBlock = oversampler->processSamplesUp (block);

    auto numSamples = oversampledBlock.getNumSamples();
    auto numChannels = oversampledBlock.getNumChannels();

    std::vector<float*> channelPtrs (numChannels);
    for (size_t i = 0; i < numSamples; ++i)
    {
        auto cutoff = cutoffSmooth.getNextValue();
        auto qVal = qSmooth.getNextValue();
        auto driveVal = driveSmooth.getNextValue();
        ladder.setCutoffFrequencyHz (cutoff);
        ladder.setResonance (qVal);
        float dB = std::pow (10.0f, driveVal / 10.0f);
        ladder.setDrive (dB);

        for (size_t ch = 0; ch < numChannels; ++ch)
            channelPtrs[ch] = oversampledBlock.getChannelPointer (ch) + i;

        juce::dsp::AudioBlock<float> singleSampleBlock (channelPtrs.data(), numChannels, 1);
        juce::dsp::ProcessContextReplacing<float> context (singleSampleBlock);
        ladder.process (context);
    }

    oversampler->processSamplesDown (block);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
        pushNextSampleIntoFifo (buffer.getReadPointer (0)[i]);
}

void TestProjectWithCodexAudioProcessor::setQ (float newQ)
{
    q = newQ;
}

void TestProjectWithCodexAudioProcessor::setDrive (float newDrive)
{
    drive = newDrive;
}

void TestProjectWithCodexAudioProcessor::setMode (int modeIndex)
{
    if (modeIndex == 0)
        ladder.setMode (juce::dsp::LadderFilter<float>::Mode::BPF12);
    else
        ladder.setMode (juce::dsp::LadderFilter<float>::Mode::BPF24);
}

float TestProjectWithCodexAudioProcessor::getCurrentF0() const
{
    return currentF0.load();
}

bool TestProjectWithCodexAudioProcessor::getFFTData (std::vector<float>& data)
{
    if (nextFFTBlockReady)
    {
        data.assign (fftData.begin(), fftData.begin() + fftSize / 2);
        nextFFTBlockReady = false;
        return true;
    }

    return false;
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

void TestProjectWithCodexAudioProcessor::pushNextSampleIntoFifo (float sample)
{
    if (fifoIndex == fftSize)
    {
        if (! nextFFTBlockReady)
        {
            std::copy (fifo.begin(), fifo.end(), fftData.begin());
            window.multiplyWithWindowingTable (fftData.data(), fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform (fftData.data());
            nextFFTBlockReady = true;
        }

        fifoIndex = 0;
    }

    fifo[(size_t) fifoIndex++] = sample;
}
