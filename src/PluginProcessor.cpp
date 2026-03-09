#include "PluginProcessor.h"

#include "BinaryData.h"
#include "PluginEditor.h"

#include <aic.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
AicDemoAudioProcessor::AicDemoAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "state",
            {std::make_unique<juce::AudioParameterChoice>(
                 juce::ParameterID{"model", 1}, "Model",
                 juce::StringArray{"sparrow-s-48khz", "sparrow-l-48khz"}, 0),
             std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass", 1}, "Bypass",
                                                        false),
             std::make_unique<juce::AudioParameterFloat>(
                 juce::ParameterID{"enhancement", 1}, "Enhancement Level",
                 juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f),
             std::make_unique<juce::AudioParameterFloat>(
                 juce::ParameterID{"vad_speech_hold_duration", 1}, "VAD Speech Hold Duration",
                 juce::NormalisableRange<float>(0.0f, 48.0f), 0.03f),
             std::make_unique<juce::AudioParameterFloat>(
                 juce::ParameterID{"vad_sensitivity", 1}, "VAD Sensitivity",
                 juce::NormalisableRange<float>(1.0f, 15.0f), 6.0f),
             std::make_unique<juce::AudioParameterFloat>(
                 juce::ParameterID{"vad_minimum_speech_duration", 1}, "VAD Minimum Speech Duration",
                 juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f)})
{
    state.addParameterListener("model", this);

    // Load and validate license key
    loadAndValidateLicense();

    // Load the default model (index 0)
    loadModel(0);
}

AicDemoAudioProcessor::~AicDemoAudioProcessor()
{
    state.removeParameterListener("model", this);

    // Clean up any pending bundle that was never picked up
    delete m_pending.exchange(nullptr);
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

    // Rebuild processor pipeline at new audio settings
    if (m_modelIndex >= 0 && isLicenseValid())
    {
        loadModel(m_modelIndex);
    }
}

void AicDemoAudioProcessor::releaseResources()
{
}

void AicDemoAudioProcessor::reset()
{
    if (m_active && m_active->initialized)
    {
        m_active->context.reset();
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

    // Check for pending processor swap
    auto* pending = m_pending.exchange(nullptr);
    if (pending)
    {
        m_active.reset(pending);
        if (m_active->initialized)
        {
            setLatencySamples(static_cast<int>(m_active->context.get_output_delay()));
        }
    }

    if (!m_active || !m_active->initialized || !isLicenseValid())
    {
        // No processor available - audio passes through unchanged
        return;
    }

    // Set parameters via context
    m_active->context.set_parameter(aic::ProcessorParameter::Bypass,
                                    state.getRawParameterValue("bypass")->load());
    m_active->context.set_parameter(aic::ProcessorParameter::EnhancementLevel,
                                    state.getRawParameterValue("enhancement")->load());

    // VAD parameters
    m_active->vadContext.set_parameter(aic::VadParameter::SpeechHoldDuration,
                                       state.getRawParameterValue("vad_speech_hold_duration")->load());
    m_active->vadContext.set_parameter(aic::VadParameter::Sensitivity,
                                       state.getRawParameterValue("vad_sensitivity")->load());
    m_active->vadContext.set_parameter(aic::VadParameter::Sensitivity,
                                       state.getRawParameterValue("vad_minimum_speech_duration")->load());

    auto processing_result = m_active->processor.process_planar(
        buffer.getArrayOfWritePointers(), static_cast<uint16_t>(totalNumInputChannels),
        static_cast<size_t>(buffer.getNumSamples()));

    // Update model info box if state of processingNotAllowed changed
    bool currentProcessingNotAllowed = (processing_result == aic::ErrorCode::EnhancementNotAllowed);
    if (m_processingNotAllowed != currentProcessingNotAllowed)
    {
        m_processingNotAllowed = currentProcessingNotAllowed;
        m_modelChanged.store(true);
    }
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
    if (auto xmlState = state.copyState().createXml())
        copyXmlToBinary(*xmlState, destData);
}

void AicDemoAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
    {
        state.replaceState(juce::ValueTree::fromXml(*xmlState));

        int modelIndex = static_cast<int>(state.getRawParameterValue("model")->load());
        loadModel(modelIndex);
    }
}

void AicDemoAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "model")
    {
        loadModel(static_cast<int>(newValue));
    }
}

bool AicDemoAudioProcessor::validateLicenseKey(const juce::String& licenseKey)
{
    if (licenseKey.trim().isEmpty())
    {
        return false;
    }

    // If we have a model loaded, validate by trying to create a processor
    if (m_model)
    {
        auto processorResult = aic::Processor::create(*m_model, licenseKey.toStdString());
        return processorResult.ok();
    }

    // No model loaded — accept the key optimistically.
    // It will be validated when a model is loaded.
    return true;
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

    // Write the license key to file
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

            if (licenseKey.isNotEmpty())
            {
                m_licenseKey = licenseKey.toStdString();

                // If we have a model, validate the license by trying to create a processor
                if (m_model)
                {
                    auto processorResult =
                        aic::Processor::create(*m_model, licenseKey.toStdString());
                    m_licenseValid.store(processorResult.ok());
                }
                else
                {
                    // No model loaded yet — accept the key optimistically
                    m_licenseValid.store(true);
                }
                return m_licenseValid.load();
            }
            else
            {
                DBG("Empty license key found in file!");
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

bool AicDemoAudioProcessor::loadModel(int modelIndex)
{
    modelIndex = juce::jlimit(0, 1, modelIndex);

    // Select embedded model data
    const char* rawData = nullptr;
    size_t      dataSize = 0;

    if (modelIndex == 0)
    {
        rawData  = BinaryData::sparrows48khz_aicmodel;
        dataSize = static_cast<size_t>(BinaryData::sparrows48khz_aicmodelSize);
    }
    else
    {
        rawData  = BinaryData::sparrowl48khz_aicmodel;
        dataSize = static_cast<size_t>(BinaryData::sparrowl48khz_aicmodelSize);
    }

    // Allocate a 64-byte-aligned copy of the model data.
    // The buffer must remain valid for the lifetime of the aic::Model object.
    size_t alignedSize = (dataSize + 63u) & ~size_t(63u);
    auto*  newBuf      = static_cast<uint8_t*>(std::aligned_alloc(64u, alignedSize));
    if (!newBuf)
    {
        DBG("Failed to allocate aligned model buffer");
        return false;
    }
    std::memcpy(newBuf, rawData, dataSize);

    // Create the model from the aligned buffer.
    // m_model.emplace() destroys the previous aic::Model (safe per SDK docs:
    // "It is safe to destroy the Model after creating the desired processors").
    auto modelResult = aic::Model::create_from_buffer(newBuf, dataSize);
    if (!modelResult.ok())
    {
        std::free(newBuf);
        DBG("Failed to create model from buffer");
        return false;
    }

    m_model.emplace(std::move(modelResult.take()));

    // The old aligned buffer is freed here; the old Model has already been destroyed above.
    m_alignedBuffer.reset(newBuf);

    m_modelIndex = modelIndex;

    // Need a license key to create the processor
    if (m_licenseKey.empty())
    {
        m_modelChanged.store(true);
        return false;
    }

    // Create processor with model + license
    auto processorResult = aic::Processor::create(*m_model, m_licenseKey);
    if (!processorResult.ok())
    {
        DBG("Failed to create processor (license may be invalid)");
        m_licenseValid.store(false);
        m_modelChanged.store(true);
        return false;
    }

    m_licenseValid.store(true);
    auto processor = processorResult.take();

    // Initialize if we have valid audio settings
    bool initialized = false;
    if (m_currentSampleRate > 0 && m_currentNumFrames > 0)
    {
        auto err = processor.initialize(m_currentSampleRate, m_currentNumChannels,
                                        m_currentNumFrames, true);
        initialized = (err == aic::ErrorCode::Success);
    }

    // Create contexts
    auto contextResult = processor.create_context();
    auto vadResult     = processor.create_vad_context();

    if (!contextResult.ok() || !vadResult.ok())
    {
        DBG("Failed to create processor contexts");
        return false;
    }

    // Build bundle and post as pending for audio thread
    auto bundle =
        std::make_unique<ProcessorBundle>(std::move(processor), std::move(contextResult.take()),
                                          std::move(vadResult.take()));
    bundle->initialized = initialized;

    auto* old = m_pending.exchange(bundle.release());
    delete old;

    m_modelChanged.store(true);
    return true;
}

void AicDemoAudioProcessor::forceModelRecreation()
{
    if (m_modelIndex >= 0 && isLicenseValid())
    {
        loadModel(m_modelIndex);
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AicDemoAudioProcessor();
}
