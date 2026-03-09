#pragma once

#include "AicModelInfoBox.h"
#include "juce_core/juce_core.h"

#include <aic.hpp>
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <optional>

struct ProcessorBundle
{
    // Shared ownership of the 64-byte-aligned model buffer.
    // Keeps the buffer alive for the lifetime of the processor,
    // since aic::Processor retains a reference to the model data.
    std::shared_ptr<uint8_t> buffer;

    aic::Processor        processor;
    aic::ProcessorContext  context;
    aic::VadContext        vadContext;
    bool                  initialized = false;

    ProcessorBundle(std::shared_ptr<uint8_t> buf,
                    aic::Processor p, aic::ProcessorContext c, aic::VadContext v)
        : buffer(std::move(buf)), processor(std::move(p)),
          context(std::move(c)), vadContext(std::move(v))
    {
    }
};

//==============================================================================
class AicDemoAudioProcessor final : public juce::AudioProcessor,
                                    public juce::AudioProcessorValueTreeState::Listener
{
  public:
    //==============================================================================
    AicDemoAudioProcessor();
    ~AicDemoAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool                        hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool   acceptsMidi() const override;
    bool   producesMidi() const override;
    bool   isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int                getNumPrograms() override;
    int                getCurrentProgram() override;
    void               setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void               changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // This can fire on audio or message thread depending on host automation.
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState state;

    juce::StringArray getModelChoices() const
    {
        return {"sparrow-s-48khz", "sparrow-l-48khz"};
    }

    bool isLicenseValid() const { return m_licenseValid.load(); }

    juce::String getExpectedLicensePath()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("aic")
            .getChildFile("aic-sdk-license.txt")
            .getFullPathName();
    }

    juce::File getLicenseFile()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("aic")
            .getChildFile("aic-sdk-license.txt");
    }

    bool validateLicenseKey(const juce::String& licenseKey);
    bool saveLicenseKey(const juce::String& licenseKey);
    bool loadAndValidateLicense();
    void forceModelRecreation();

    int  getModelIndex() const { return m_requestedModelIndex.load(); }
    bool isModelLoaded() const { return m_activeProcessorInitialized.load(); }
    bool isLoading() const { return false; }

    aic::ui::ModelInfo getModelInfo() const
    {
        if (!m_licenseValid.load() && m_licenseKey.empty())
            return aic::ui::ModelInfo(aic::ui::ModelState::LicenseInactive);

        if (!m_activeProcessorInitialized.load())
            return aic::ui::ModelInfo(aic::ui::ModelState::WrongAudioSettings);

        if (m_processingNotAllowed.load())
            return aic::ui::ModelInfo(aic::ui::ModelState::ProcessingNotAllowed);

        const std::size_t activeIndex = toModelArrayIndex(m_activeModelIndex.load());
        auto      outputDelayMs = m_activeOutputDelayMs.load();

        auto optimalSr = m_models[activeIndex]
                             ? static_cast<int>(m_models[activeIndex]->get_optimal_sample_rate())
                             : 0;
        auto optimalFrames = m_models[activeIndex]
                                 ? static_cast<int>(
                                       m_models[activeIndex]->get_optimal_num_frames(m_currentSampleRate))
                                 : 0;
        const auto windowLengthMs = (m_currentSampleRate > 0u)
                                        ? juce::roundToInt((static_cast<double>(optimalFrames) * 1000.0) /
                                                           static_cast<double>(m_currentSampleRate))
                                        : 0;
        const auto modelDelayMs = juce::jmax(0, outputDelayMs - windowLengthMs);

        return aic::ui::ModelInfo(optimalSr, windowLengthMs, modelDelayMs, optimalFrames,
                                  outputDelayMs);
    }

    bool modelChanged() const { return m_modelChanged.load(); }
    void acknowledgeModelChanged() { m_modelChanged.store(false); }

    juce::String getSdkVersion() const { return aic::get_sdk_version(); }

    bool isSpeechDetected() const
    {
        return m_speechDetected.load();
    }

  private:
    static constexpr int kMaxModelIndex = 1;
    static std::size_t toModelArrayIndex(int index)
    {
        return static_cast<std::size_t>(juce::jlimit(0, kMaxModelIndex, index));
    }

    bool loadEmbeddedModel(int modelIndex);
    void rebuildProcessors();
    void applyRequestedModelSwitch();
    void updateActiveRuntimeState();

    std::array<std::optional<aic::Model>, 2>        m_models;
    std::array<std::shared_ptr<uint8_t>, 2>         m_modelBuffers;
    std::array<std::unique_ptr<ProcessorBundle>, 2> m_processors;
    std::atomic<int>                                m_requestedModelIndex{0};
    std::atomic<int>                                m_activeModelIndex{0};

    std::string       m_licenseKey;
    std::atomic<bool> m_licenseValid = {false};

    std::atomic<bool> m_processingNotAllowed = {false};
    std::atomic<bool> m_rebuildRequested{false};
    std::atomic<bool> m_activeProcessorInitialized{false};
    std::atomic<bool> m_speechDetected{false};
    std::atomic<int>  m_activeOutputDelayMs{0};

    uint32_t          m_currentSampleRate{48000};
    uint16_t          m_currentNumChannels{2};
    size_t            m_currentNumFrames{480};
    std::atomic<bool> m_modelChanged{false};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicDemoAudioProcessor)
};
