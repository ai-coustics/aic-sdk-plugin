#pragma once

#include "AicModelInfoBox.h"
#include "juce_core/juce_core.h"

#include <aic.h>
#include <aic.hpp>
#include <array>
#include <cassert>
#include <juce_audio_processors/juce_audio_processors.h>

// Struct to hold model information
struct ModelInfo
{
    const char*    name;
    aic::ModelType modelType;
    int            windowLengthMs;
    int            modelDelayMs;
};

//==============================================================================
class AicDemoAudioProcessor final : public juce::AudioProcessor
{
  public:
    //==============================================================================
    AicDemoAudioProcessor();
    ~AicDemoAudioProcessor() override = default;

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

    const juce::StringArray getModelChoices() const
    {
        juce::StringArray choices;
        for (const auto& modelInfo : modelInfos)
        {
            choices.add(modelInfo.name);
        }
        return choices;
    }

    juce::AudioProcessorValueTreeState state;

    /**
     * @brief Checks if the current license key is valid.
     *
     * @return true if the license is valid and models can be created, false otherwise
     */
    bool isLicenseValid() const
    {
        return m_licenseValid.load();
    }

    /**
     * @brief Gets the expected path for the license file.
     *
     * @return String containing the full path to the expected license file location
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
     *
     * @return File object pointing to the license file location
     */
    juce::File getLicenseFile()
    {
        juce::File appDataDir =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);

        juce::File licenseFile = appDataDir.getChildFile("aic").getChildFile("aic-sdk-license.txt");

        return licenseFile;
    }

    /**
     * @brief Validates a license key by attempting to create a model with it.
     *
     * This method tests the provided license key by creating a temporary model
     * and checking if the creation succeeds.
     *
     * @param licenseKey The license key to validate
     * @return true if the license key is valid and can create models, false otherwise
     */
    bool validateLicenseKey(const juce::String& licenseKey);

    /**
     * @brief Saves a license key to the application's license file.
     *
     * Creates the necessary directory structure if it doesn't exist and
     * saves the license key to the standard location.
     *
     * @param licenseKey The license key to save
     * @return true if the license was saved successfully, false otherwise
     */
    bool saveLicenseKey(const juce::String& licenseKey);

    /**
     * @brief Loads and validates the license key from the application directory.
     *
     * Attempts to load the license key from the standard location and validate it.
     * Updates the internal license state accordingly.
     *
     * @return true if a valid license was found and loaded, false otherwise
     */
    bool loadAndValidateLicense();

    /**
     * @brief Forces recreation of the current model with the updated license.
     *
     * This method should be called after a license change to ensure the model
     * is recreated with the new license key and properly initialized.
     */
    void forceModelRecreation();

    aic::ui::ModelInfo getModelInfo() const
    {
        if (m_model && m_modelRunning)
        {
            // calculate outputDelay in ms
            auto outputDelayMs = static_cast<int>(
                juce::roundToInt((static_cast<double>(m_model->get_output_delay()) * 1000.0) /
                                 static_cast<double>(m_currentSampleRate))); // ms

            return aic::ui::ModelInfo(static_cast<int>(m_model->get_optimal_sample_rate()),
                                      modelInfos[m_activeModelIndex].windowLengthMs,
                                      modelInfos[m_activeModelIndex].modelDelayMs,
                                      static_cast<int>(m_model->get_optimal_num_frames()),
                                      outputDelayMs);
        }
        else
        {
            return aic::ui::ModelInfo(false);
        }
    }

    bool modelChanged() const
    {
        return m_modelChanged.load();
    }

    void acknowledgeModelChanged()
    {
        m_modelChanged.store(false);
    }

  private:
    /**
     * @brief Creates a new model instance with the current license key.
     *
     * Attempts to create a model of the specified type using the currently
     * stored license key. Updates the license validity state based on whether
     * model creation succeeds.
     *
     * @param index Index of the model type to create (will be clamped to valid range)
     */
    void createModel(size_t index)
    {
        index = static_cast<size_t>(
            juce::jlimit(0, static_cast<int>(m_numModels - 1), static_cast<int>(index)));

        // Only attempt to create model if we have a license key
        if (m_licenseKey.empty())
        {
            m_licenseValid.store(false);
            m_model        = nullptr;
            m_modelRunning = false;
            return;
        }

        auto [model, errorCode] = aic::AicModel::create(modelInfos[index].modelType, m_licenseKey);
        if (model && errorCode == aic::ErrorCode::Success)
        {
            m_licenseValid.store(true);
            m_model = std::move(model);
        }
        else
        {
            m_licenseValid.store(false);
            m_model        = nullptr;
            m_modelRunning = false;
        }
    }

    void initializeModel()
    {
        if (m_model)
        {
            auto errorCode =
                m_model->initialize(m_currentSampleRate, m_currentNumChannels, m_currentNumFrames);
            m_modelRunning = errorCode == aic::ErrorCode::Success;
            m_modelChanged.store(true);
        }
    }

    // Define all models here
    inline static const std::array<ModelInfo, 8> modelInfos = {
        {{"Quail L48", aic::ModelType::Quail_L48, 10, 30},
         {"Quail L16", aic::ModelType::Quail_L16, 10, 30},
         {"Quail L8", aic::ModelType::Quail_L8, 10, 30},
         {"Quail S48", aic::ModelType::Quail_S48, 10, 30},
         {"Quail S16", aic::ModelType::Quail_S16, 10, 30},
         {"Quail S8", aic::ModelType::Quail_S8, 10, 30},
         {"Quail XS", aic::ModelType::Quail_XS, 10, 10},
         {"Quail XXS", aic::ModelType::Quail_XXS, 10, 10}}};
    static constexpr size_t m_numModels = modelInfos.size();

    std::unique_ptr<aic::AicModel> m_model;

    std::string       m_licenseKey;
    std::atomic<bool> m_licenseValid = {false};

    size_t            m_activeModelIndex{0};
    uint32_t          m_currentSampleRate{0};
    uint16_t          m_currentNumChannels{0};
    size_t            m_currentNumFrames{0};
    std::atomic<bool> m_modelChanged{false};

    bool m_modelRunning{false};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicDemoAudioProcessor)
};
