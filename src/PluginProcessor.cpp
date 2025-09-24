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
                                                        "Noise Gate Enable", false)})
{
    // Load and validate license key
    loadAndValidateLicense();

    // Only create model if license is valid
    if (isLicenseValid())
    {
        createModel(m_activeModelIndex);
    }
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

    if (m_model)
    {
        setLatencySamples((int) m_model->get_output_delay());
    }
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
        // Only create model if we have a valid license
        if (isLicenseValid())
        {
            /// This is not real-time safe and will lead to clicks.
            /// Though we want to show the RAM usage of each model and they are not meant to be
            /// changed in real-time. When loading all models prior, the RAM usage is way higher
            /// than necessary.
            createModel(m_activeModelIndex);
            initializeModel();
        }
    }

    if (!m_model || !m_modelIsInitialized || !isLicenseValid())
    {
        // Model is nullptr, not running, or license invalid - audio passes through unchanged
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

bool AicDemoAudioProcessor::validateLicenseKey(const juce::String& licenseKey)
{
    if (licenseKey.trim().isEmpty())
    {
        return false;
    }

    // Test the license key by attempting to create a model
    auto [testModel, errorCode] =
        aic::AicModel::create(modelInfos[0].modelType, licenseKey.toStdString());
    return testModel != nullptr && errorCode == aic::ErrorCode::Success;
}

bool AicDemoAudioProcessor::saveLicenseKey(const juce::String& licenseKey)
{
    juce::File licenseFile = getLicenseFile();

    // Create directory if it doesn't exist
    auto parentDir = licenseFile.getParentDirectory();
    if (!parentDir.exists())
    {
        auto result = parentDir.createDirectory();
        if (result.failed())
        {
            DBG("Failed to create license directory: " + result.getErrorMessage());
            return false;
        }
    }

    // Write the license key to file (delete existing file first to ensure overwrite)
    if (licenseFile.exists())
    {
        licenseFile.deleteFile();
    }

    juce::FileOutputStream stream(licenseFile);
    if (stream.openedOk())
    {
        stream.writeText(licenseKey, false, false, nullptr);
        stream.flush();
        return true;
    }
    else
    {
        DBG("Failed to open license file for writing!");
        return false;
    }
}

bool AicDemoAudioProcessor::loadAndValidateLicense()
{
    juce::File licenseFile = getLicenseFile();

    if (licenseFile.existsAsFile())
    {
        juce::FileInputStream stream(licenseFile);
        if (stream.openedOk())
        {
            juce::String licenseKey = stream.readEntireStreamAsString().trim();

            if (validateLicenseKey(licenseKey))
            {
                m_licenseKey = licenseKey.toStdString();
                m_licenseValid.store(true);
                return true;
            }
            else
            {
                DBG("Invalid license key found in file!");
                m_licenseValid.store(false);
                return false;
            }
        }
        else
        {
            DBG("Failed to open license file!");
            m_licenseValid.store(false);
            return false;
        }
    }
    else
    {
        DBG("License file not found!");
        m_licenseValid.store(false);
        return false;
    }
}

void AicDemoAudioProcessor::forceModelRecreation()
{
    if (isLicenseValid())
    {
        // Get current model index
        auto currentModelIndex = static_cast<size_t>(state.getRawParameterValue("model")->load());

        // Recreate the model
        createModel(currentModelIndex);

        // Reinitialize if we have valid audio parameters
        if (m_currentSampleRate > 0 && m_currentNumChannels > 0 && m_currentNumFrames > 0)
        {
            initializeModel();
        }
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AicDemoAudioProcessor();
}
