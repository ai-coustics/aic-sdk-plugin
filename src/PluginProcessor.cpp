#include "PluginProcessor.h"

#include "BinaryData.h"
#include "PluginEditor.h"

#include <aic.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#if JUCE_WINDOWS
#include <malloc.h>
#endif
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace
{
constexpr std::size_t kNumModels = 2u;

const char* getModelRawData(const std::size_t modelIndex)
{
    return (modelIndex == 0u) ? BinaryData::sparrows48khz_aicmodel : BinaryData::sparrowl48khz_aicmodel;
}

size_t getModelDataSize(const std::size_t modelIndex)
{
    return static_cast<size_t>((modelIndex == 0u) ? BinaryData::sparrows48khz_aicmodelSize
                                                  : BinaryData::sparrowl48khz_aicmodelSize);
}

uint8_t* allocateAlignedBuffer(size_t alignment, size_t size)
{
#if JUCE_WINDOWS
    return static_cast<uint8_t*>(_aligned_malloc(size, alignment));
#else
    return static_cast<uint8_t*>(std::aligned_alloc(alignment, size));
#endif
}

void freeAlignedBuffer(uint8_t* p)
{
#if JUCE_WINDOWS
    _aligned_free(p);
#else
    std::free(p);
#endif
}
} // namespace

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

    const int initialModelIndex = juce::jlimit(0, 1, static_cast<int>(state.getRawParameterValue("model")->load()));
    m_requestedModelIndex.store(initialModelIndex);
    m_activeModelIndex.store(initialModelIndex);

    loadAndValidateLicense();

    for (std::size_t modelIndex = 0; modelIndex < kNumModels; ++modelIndex)
        loadEmbeddedModel(static_cast<int>(modelIndex));

    rebuildProcessors();
    updateActiveRuntimeState();

    m_modelChanged.store(true);
}

AicDemoAudioProcessor::~AicDemoAudioProcessor()
{
    state.removeParameterListener("model", this);
}

bool AicDemoAudioProcessor::loadEmbeddedModel(const int modelIndex)
{
    const std::size_t arrayIndex = toModelArrayIndex(modelIndex);
    const char*       rawData    = getModelRawData(arrayIndex);
    const size_t      dataSize   = getModelDataSize(arrayIndex);

    const size_t alignedSize = (dataSize + 63u) & ~size_t(63u);
    std::shared_ptr<uint8_t> buffer(allocateAlignedBuffer(64u, alignedSize), freeAlignedBuffer);

    if (!buffer)
    {
        DBG("Failed to allocate aligned model buffer");
        return false;
    }

    std::memcpy(buffer.get(), rawData, dataSize);

    auto modelResult = aic::Model::create_from_buffer(buffer.get(), dataSize);
    if (!modelResult.ok())
    {
        DBG("Failed to create model from buffer");
        return false;
    }

    m_modelBuffers[arrayIndex] = buffer;
    m_models[arrayIndex].emplace(std::move(modelResult.take()));

    return true;
}

void AicDemoAudioProcessor::rebuildProcessors()
{
    for (auto& processor : m_processors)
        processor.reset();

    m_processingNotAllowed.store(false);
    m_speechDetected.store(false);

    if (!isLicenseValid() || m_licenseKey.empty())
    {
        m_activeProcessorInitialized.store(false);
        m_activeOutputDelayMs.store(0);
        setLatencySamples(0);
        return;
    }

    for (std::size_t modelIndex = 0; modelIndex < kNumModels; ++modelIndex)
    {
        if (!m_models[modelIndex] || !m_modelBuffers[modelIndex])
            continue;

        auto processorResult = aic::Processor::create(*m_models[modelIndex], m_licenseKey);
        if (!processorResult.ok())
        {
            DBG("Failed to create processor (license may be invalid)");
            m_licenseValid.store(false);
            for (auto& processor : m_processors)
                processor.reset();
            return;
        }

        auto processor = processorResult.take();

        bool initialized = false;
        if (m_currentSampleRate > 0 && m_currentNumFrames > 0)
        {
            auto err = processor.initialize(m_currentSampleRate, m_currentNumChannels, m_currentNumFrames, true);
            initialized = (err == aic::ErrorCode::Success);
        }

        auto contextResult = processor.create_context();
        auto vadResult     = processor.create_vad_context();
        if (!contextResult.ok() || !vadResult.ok())
        {
            DBG("Failed to create processor contexts");
            continue;
        }

        auto bundle = std::make_unique<ProcessorBundle>(
            m_modelBuffers[modelIndex],
            std::move(processor),
            std::move(contextResult.take()),
            std::move(vadResult.take()));
        bundle->initialized = initialized;

        m_processors[modelIndex] = std::move(bundle);
    }

    updateActiveRuntimeState();
}

void AicDemoAudioProcessor::applyRequestedModelSwitch()
{
    const int requestedIndexInt = juce::jlimit(0, kMaxModelIndex, m_requestedModelIndex.load());
    const int activeIndexInt    = juce::jlimit(0, kMaxModelIndex, m_activeModelIndex.load());
    const std::size_t activeIndex    = toModelArrayIndex(activeIndexInt);

    if (requestedIndexInt == activeIndexInt)
        return;

    if (m_processors[activeIndex])
    {
        m_processors[activeIndex]->context.reset();
    }

    m_activeModelIndex.store(requestedIndexInt);

    m_processingNotAllowed.store(false);
    m_speechDetected.store(false);
    updateActiveRuntimeState();
    m_modelChanged.store(true);
}

void AicDemoAudioProcessor::updateActiveRuntimeState()
{
    const std::size_t activeIndex = toModelArrayIndex(m_activeModelIndex.load());
    if (m_processors[activeIndex] && m_processors[activeIndex]->initialized)
    {
        m_activeProcessorInitialized.store(true);
        const auto outputDelay = m_processors[activeIndex]->context.get_output_delay();
        const auto outputDelayMs = static_cast<int>(
            juce::roundToInt((static_cast<double>(outputDelay) * 1000.0) /
                             static_cast<double>(m_currentSampleRate)));
        m_activeOutputDelayMs.store(outputDelayMs);
        setLatencySamples(static_cast<int>(outputDelay));
        return;
    }

    m_activeProcessorInitialized.store(false);
    m_activeOutputDelayMs.store(0);
    setLatencySamples(0);
}

//==============================================================================
const juce::String AicDemoAudioProcessor::getName() const { return JucePlugin_Name; }

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

double AicDemoAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int  AicDemoAudioProcessor::getNumPrograms() { return 1; }
int  AicDemoAudioProcessor::getCurrentProgram() { return 0; }
void AicDemoAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }

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

    rebuildProcessors();
    updateActiveRuntimeState();

    m_modelChanged.store(true);
}

void AicDemoAudioProcessor::releaseResources() {}

void AicDemoAudioProcessor::reset()
{
    for (auto& processor : m_processors)
    {
        if (processor && processor->initialized)
        {
            processor->context.reset();
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

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (m_rebuildRequested.exchange(false))
    {
        rebuildProcessors();
        updateActiveRuntimeState();
        m_modelChanged.store(true);
    }

    applyRequestedModelSwitch();

    const std::size_t activeIndex = toModelArrayIndex(m_activeModelIndex.load());
    auto*     active      = m_processors[activeIndex].get();

    if (!isLicenseValid() || !active || !active->initialized)
    {
        m_speechDetected.store(false);
        return;
    }

    active->context.set_parameter(aic::ProcessorParameter::Bypass,
                                  state.getRawParameterValue("bypass")->load());
    active->context.set_parameter(aic::ProcessorParameter::EnhancementLevel,
                                  state.getRawParameterValue("enhancement")->load());

    active->vadContext.set_parameter(aic::VadParameter::SpeechHoldDuration,
                                     state.getRawParameterValue("vad_speech_hold_duration")->load());
    active->vadContext.set_parameter(aic::VadParameter::Sensitivity,
                                     state.getRawParameterValue("vad_sensitivity")->load());
    active->vadContext.set_parameter(aic::VadParameter::MinimumSpeechDuration,
                                     state.getRawParameterValue("vad_minimum_speech_duration")->load());

    auto result = active->processor.process_planar(
        buffer.getArrayOfWritePointers(), static_cast<uint16_t>(totalNumInputChannels),
        static_cast<size_t>(buffer.getNumSamples()));

    m_speechDetected.store(active->vadContext.is_speech_detected());

    const bool notAllowed = (result == aic::ErrorCode::EnhancementNotAllowed);
    if (m_processingNotAllowed.load() != notAllowed)
    {
        m_processingNotAllowed.store(notAllowed);
        m_modelChanged.store(true);
    }
}

//==============================================================================
bool AicDemoAudioProcessor::hasEditor() const { return true; }

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
        const int modelIndex = juce::jlimit(0, 1, static_cast<int>(state.getRawParameterValue("model")->load()));
        m_requestedModelIndex.store(modelIndex);
        m_modelChanged.store(true);
    }
}

void AicDemoAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID != "model")
        return;

    m_requestedModelIndex.store(juce::jlimit(0, 1, static_cast<int>(newValue)));
    m_modelChanged.store(true);
}

//==============================================================================
bool AicDemoAudioProcessor::validateLicenseKey(const juce::String& licenseKey)
{
    const auto trimmed = licenseKey.trim();
    if (trimmed.isEmpty())
        return false;

    const std::string key = trimmed.toStdString();

    for (const auto& model : m_models)
    {
        if (!model)
            continue;

        auto result = aic::Processor::create(*model, key);
        if (!result.ok())
            return false;
    }

    return true;
}

bool AicDemoAudioProcessor::saveLicenseKey(const juce::String& licenseKey)
{
    juce::File licenseFile = getLicenseFile();
    auto       parentDir   = licenseFile.getParentDirectory();

    if (!parentDir.exists())
    {
        auto result = parentDir.createDirectory();
        if (result.failed())
        {
            DBG("Failed to create license directory: " + result.getErrorMessage());
            return false;
        }
    }

    if (licenseFile.exists())
        licenseFile.deleteFile();

    juce::FileOutputStream stream(licenseFile);
    if (!stream.openedOk())
    {
        DBG("Failed to open license file for writing!");
        return false;
    }

    stream.writeText(licenseKey, false, false, nullptr);
    stream.flush();
    return true;
}

bool AicDemoAudioProcessor::loadAndValidateLicense()
{
    juce::File licenseFile = getLicenseFile();

    if (!licenseFile.existsAsFile())
    {
        DBG("License file not found!");
        m_licenseKey.clear();
        m_licenseValid.store(false);
        return false;
    }

    juce::FileInputStream stream(licenseFile);
    if (!stream.openedOk())
    {
        DBG("Failed to open license file!");
        m_licenseKey.clear();
        m_licenseValid.store(false);
        return false;
    }

    juce::String licenseKey = stream.readEntireStreamAsString().trim();
    if (licenseKey.isEmpty())
    {
        DBG("Empty license key found in file!");
        m_licenseKey.clear();
        m_licenseValid.store(false);
        return false;
    }

    m_licenseKey = licenseKey.toStdString();
    m_licenseValid.store(validateLicenseKey(licenseKey));

    return m_licenseValid.load();
}

void AicDemoAudioProcessor::forceModelRecreation()
{
    m_rebuildRequested.store(true);
    m_modelChanged.store(true);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AicDemoAudioProcessor();
}
