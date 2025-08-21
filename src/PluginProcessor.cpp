#include "PluginProcessor.h"

#include "PluginEditor.h"

#include <aic.hpp>
#include <cstdint>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
AicDemoAudioProcessor::AicDemoAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "state",
            {std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"model", 1}, "Model",
                                                          getModelChoices(), 0),
             std::make_unique<juce::AudioParameterFloat>(
                 juce::ParameterID{"enhancement", 1}, "Enhancement Level",
                 juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f),
             std::make_unique<juce::AudioParameterFloat>(
                 juce::ParameterID{"voicegain", 1}, "Voice Gain",
                 juce::NormalisableRange<float>(-12.0f, 12.0f), 1.0f),
             std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"noisegateenable", 0},
                                                        "Noise Gate Enable", true)})
{
    // Load license key
    juce::File licenseFile = getLicenseFile();

    if (licenseFile.existsAsFile())
    {
        juce::FileInputStream stream(licenseFile);
        if (stream.openedOk())
        {
            m_licenseKey = stream.readEntireStreamAsString().toStdString();
        }
        else
        {
            DBG("Failed to open license file!");
        }
    }
    else
    {
        DBG("License file not found!");
    }

    createModel(m_activeModelIndex);
}

//==============================================================================
const juce::String AicDemoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AicDemoAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AicDemoAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AicDemoAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AicDemoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AicDemoAudioProcessor::getNumPrograms()
{
    return 1;
}

int AicDemoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AicDemoAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AicDemoAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AicDemoAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AicDemoAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    m_currentSampleRate  = static_cast<uint32_t>(sampleRate);
    m_currentNumChannels = static_cast<uint16_t>(getTotalNumInputChannels());
    m_currentNumFrames   = static_cast<size_t>(samplesPerBlock);

    initializeModel();
}

void AicDemoAudioProcessor::releaseResources()
{
    // Models will be automatically destroyed when unique_ptrs go out of scope
}

void AicDemoAudioProcessor::reset()
{
    if (m_model)
    {
        m_model->reset();
    }
}

bool AicDemoAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AicDemoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer&         midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto                    totalNumInputChannels  = getTotalNumInputChannels();
    auto                    totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels that don't have input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get parameter values in a real-time safe way
    auto modelParameterValue = state.getRawParameterValue("model");

    // Convert model parameter to index
    size_t modelIndex = static_cast<size_t>(modelParameterValue->load());

    // update model if model changed
    if (m_activeModelIndex != modelIndex)
    {
        m_activeModelIndex = modelIndex;
        createModel(m_activeModelIndex);
        initializeModel();
    }

    if (!m_model)
    {
        // Model is nullptr, just return (audio passes through unchanged)
        return;
    }

    // Set parameters for selected model
    m_model->set_parameter(aic::Parameter::EnhancementLevel,
                           state.getRawParameterValue("enhancement")->load());

    m_model->set_parameter(
        aic::Parameter::VoiceGain,
        juce::Decibels::decibelsToGain(state.getRawParameterValue("voicegain")->load()));

    m_model->set_parameter(aic::Parameter::NoiseGateEnable,
                           state.getRawParameterValue("noisegateenable")->load());

    m_model->process_planar(buffer.getArrayOfWritePointers(),
                            static_cast<uint16_t>(totalNumInputChannels),
                            static_cast<size_t>(buffer.getNumSamples()));
}

//==============================================================================
bool AicDemoAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AicDemoAudioProcessor::createEditor()
{
    return new AicDemoAudioProcessorEditor(*this);
}

//==============================================================================
void AicDemoAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Store an XML representation of our state
    if (auto xmlState = state.copyState().createXml())
        copyXmlToBinary(*xmlState, destData);
}

void AicDemoAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore the plugin's state from the XML representation
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
        state.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AicDemoAudioProcessor();
}
