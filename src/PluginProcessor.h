#pragma once

#include "AicModelInfoBox.h"
#include "juce_core/juce_core.h"

#include <aic.hpp>
#include <atomic>
#include <cassert>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <optional>

struct ProcessorBundle
{
    aic::Processor        processor;
    aic::ProcessorContext  context;
    aic::VadContext        vadContext;
    bool                  initialized = false;

    ProcessorBundle(aic::Processor p, aic::ProcessorContext c, aic::VadContext v)
        : processor(std::move(p)), context(std::move(c)), vadContext(std::move(v))
    {
    }
};

//==============================================================================
class AicDemoAudioProcessor final : public juce::AudioProcessor
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

    juce::AudioProcessorValueTreeState state;

    /**
     * @brief Checks if the current license key is valid.
     */
    bool isLicenseValid() const
    {
        return m_licenseValid.load();
    }

    /**
     * @brief Gets the expected path for the license file.
     */
    juce::String getExpectedLicensePath()
    {
        juce::File appDataDir =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);

        juce::File licenseFile = appDataDir.getChildFile("aic").getChildFile("aic-sdk-license.txt");

        return licenseFile.getFullPathName();
    }

    /**
     * @brief Gets the license file object.
     */
    juce::File getLicenseFile()
    {
        juce::File appDataDir =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);

        juce::File licenseFile = appDataDir.getChildFile("aic").getChildFile("aic-sdk-license.txt");

        return licenseFile;
    }

    /**
     * @brief Validates a license key by attempting to create a processor with it.
     */
    bool validateLicenseKey(const juce::String& licenseKey);

    /**
     * @brief Saves a license key to the application's license file.
     */
    bool saveLicenseKey(const juce::String& licenseKey);

    /**
     * @brief Loads and validates the license key from the application directory.
     */
    bool loadAndValidateLicense();

    /**
     * @brief Forces recreation of the current processor with the updated license.
     */
    void forceModelRecreation();

    /**
     * @brief Loads a model from a file path and creates the processor pipeline.
     *
     * This creates the Model, Processor, ProcessorContext, and VadContext
     * on the message thread, and posts them as pending for the audio thread.
     *
     * @param path Full path to the model file
     * @return true if the model was loaded and processor created successfully
     */
    bool loadModel(const juce::String& path);

    /**
     * @brief Gets the current model file path.
     */
    juce::String getModelPath() const
    {
        return m_modelPath;
    }

    /**
     * @brief Whether a model is currently loaded and active.
     */
    bool isModelLoaded() const
    {
        return m_active != nullptr && m_active->initialized;
    }

    aic::ui::ModelInfo getModelInfo() const
    {
        if (!m_licenseValid && m_licenseKey.empty())
        {
            return aic::ui::ModelInfo(aic::ui::ModelState::LicenseInactive);
        }

        if (!m_active || !m_active->initialized)
        {
            if (m_modelPath.isEmpty())
            {
                return aic::ui::ModelInfo(aic::ui::ModelState::NoModelLoaded);
            }
            else
            {
                return aic::ui::ModelInfo(aic::ui::ModelState::WrongAudioSettings);
            }
        }

        if (m_processingNotAllowed)
        {
            return aic::ui::ModelInfo(aic::ui::ModelState::ProcessingNotAllowed);
        }

        auto outputDelay   = m_active->context.get_output_delay();
        auto outputDelayMs = static_cast<int>(
            juce::roundToInt((static_cast<double>(outputDelay) * 1000.0) /
                              static_cast<double>(m_currentSampleRate)));

        auto modelId       = m_model ? m_model->get_id() : std::string("Unknown");
        auto optimalSr     = m_model ? static_cast<int>(m_model->get_optimal_sample_rate()) : 0;
        auto optimalFrames =
            m_model ? static_cast<int>(m_model->get_optimal_num_frames(m_currentSampleRate)) : 0;

        return aic::ui::ModelInfo(modelId, optimalSr, optimalFrames, outputDelayMs);
    }

    bool modelChanged() const
    {
        return m_modelChanged.load();
    }

    void acknowledgeModelChanged()
    {
        m_modelChanged.store(false);
    }

    juce::String getSdkVersion() const
    {
        return aic::get_sdk_version();
    }

    bool isSpeechDetected() const
    {
        if (m_active && m_active->initialized)
        {
            return m_active->vadContext.is_speech_detected();
        }
        return false;
    }

  private:
    // Active processor bundle — used on audio thread only
    std::unique_ptr<ProcessorBundle> m_active;

    // Pending processor bundle — set on message thread, picked up on audio thread
    std::atomic<ProcessorBundle*> m_pending{nullptr};

    // Model object — kept alive for the lifetime of the processor
    std::optional<aic::Model> m_model;

    // Current model file path
    juce::String m_modelPath;

    std::string       m_licenseKey;
    std::atomic<bool> m_licenseValid = {false};

    bool m_processingNotAllowed = {false};

    uint32_t          m_currentSampleRate{48000};
    uint16_t          m_currentNumChannels{2};
    size_t            m_currentNumFrames{480};
    std::atomic<bool> m_modelChanged{false};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicDemoAudioProcessor)
};
