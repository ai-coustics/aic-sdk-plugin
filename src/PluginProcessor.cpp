#include "PluginProcessor.h"

#include "PluginEditor.h"

#include <aic.hpp>
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
                 juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f),
             std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"noisegateenable", 1},
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

    // Create models using the new API
    for (size_t i = 0; i < numModels; ++i)
    {
        auto [model, erroCode] = aic::AicModel::create(modelInfos[i].modelType, m_licenseKey);
        if (erroCode == aic::ErrorCode::Success)
        {
            m_licenseValid.store(true);
            m_models[i] = std::move(model);
        }
        else
        {
            m_licenseValid.store(false);
            m_models[i] = nullptr;
        }
    }
}

const juce::StringArray AicDemoAudioProcessor::getModelChoices() const
{
    juce::StringArray choices;
    for (const auto& modelInfo : modelInfos)
    {
        choices.add(modelInfo.name);
    }
    return choices;
}

bool AicDemoAudioProcessor::validateAndSaveLicenseKey(const juce::String& licenseKey)
{
    std::string licenseKeyStr = licenseKey.toStdString();

    // Test license validity by trying to create a model
    auto [testModel, errorCode] = aic::AicModel::create(modelInfos[0].modelType, licenseKeyStr);

    if (errorCode != aic::ErrorCode::Success)
    {
        return false;
    }

    // License is valid, save it to file
    juce::File licenseFile = getLicenseFile();

    // Create the directory if it doesn't exist
    licenseFile.getParentDirectory().createDirectory();

    // Write the license key to file
    if (!licenseFile.replaceWithText(licenseKey))
    {
        DBG("Failed to save license file!");
        return false;
    }

    // Update internal license key
    m_licenseKey = licenseKeyStr;

    // Reinitialize all models with the new license
    for (size_t i = 0; i < numModels; ++i)
    {
        auto [model, erroCode] = aic::AicModel::create(modelInfos[i].modelType, m_licenseKey);
        if (erroCode == aic::ErrorCode::Success)
        {
            m_models[i] = std::move(model);
        }
        else
        {
            m_models[i] = nullptr;
        }
    }

    // Update license validity flag
    m_licenseValid.store(true);

    DBG("License key validated and saved successfully!");
    return true;
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
    // Initialize all valid models
    for (auto& model : m_models)
    {
        if (model)
        {
            model->initialize(static_cast<uint32_t>(sampleRate),
                              static_cast<uint16_t>(getTotalNumInputChannels()),
                              static_cast<size_t>(samplesPerBlock));
        }
    }

    current_sample_rate = sampleRate;
    m_prepareWasCalled.store(true);
}

void AicDemoAudioProcessor::releaseResources()
{
    // Models will be automatically destroyed when unique_ptrs go out of scope
}

void AicDemoAudioProcessor::reset()
{
    // Reset all valid models
    for (auto& model : m_models)
    {
        if (model)
        {
            model->reset();
        }
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
    int modelIndex = static_cast<int>(modelParameterValue->load());
    modelIndex     = juce::jlimit(0, static_cast<int>(numModels - 1), modelIndex);

    // Get the selected model
    auto& selectedModel = m_models[static_cast<size_t>(modelIndex)];

    if (!selectedModel)
    {
        // Model not available, just return (audio passes through unchanged)
        return;
    }

    // Set parameters for selected model
    selectedModel->set_parameter(aic::Parameter::EnhancementLevel,
                                 state.getRawParameterValue("enhancement")->load());

    selectedModel->set_parameter(
        aic::Parameter::VoiceGain,
        juce::Decibels::decibelsToGain(state.getRawParameterValue("voicegain")->load()));

    selectedModel->set_parameter(aic::Parameter::NoiseGateEnable,
                                 state.getRawParameterValue("noisegateenable")->load());

    selectedModel->process_planar(buffer.getArrayOfWritePointers(),
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
