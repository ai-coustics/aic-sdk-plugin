#pragma once

#include "AicModelInfoBox.h"

#include <aic.h>
#include <aic.hpp>
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>

// Struct to hold model information
struct ModelInfo
{
    const char*    name;
    aic::ModelType modelType;
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

    const juce::StringArray getModelChoices() const;

    juce::AudioProcessorValueTreeState state;

    bool isLicenseValid() const
    {
        return m_licenseValid.load();
    }

    juce::String getExpectedLicensePath()
    {
        juce::File appDataDir =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);

        juce::File licenseFile = appDataDir.getChildFile("aic").getChildFile("aic-sdk-license.txt");

        return licenseFile.getFullPathName();
    }

    juce::File getLicenseFile()
    {
        juce::File appDataDir =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);

        juce::File licenseFile = appDataDir.getChildFile("aic").getChildFile("aic-sdk-license.txt");

        return licenseFile;
    }

    void initializeModels();

    /**
     * @brief Validates and saves a new license key.
     *
     * This method attempts to validate the provided license key by creating
     * a test model instance. If validation succeeds, the license key is saved
     * to the license file and all models are reinitialized.
     *
     * @param licenseKey The license key string to validate
     * @return true if the license key is valid and was saved successfully, false otherwise
     */
    bool validateAndSaveLicenseKey(const juce::String& licenseKey);

    aic::ui::ModelUiInfo getModelInfo(int modelIndex) const
    {
        auto& selectedModel = m_models[static_cast<size_t>(modelIndex)];

        auto sample_rate  = static_cast<int>(selectedModel->get_optimal_sample_rate());
        auto num_frames   = static_cast<int>(selectedModel->get_optimal_num_frames());
        auto output_delay = static_cast<int>(((double) selectedModel->get_output_delay() * 1000.0) /
                                             current_sample_rate); // ms

        switch (modelInfos[static_cast<size_t>(modelIndex)].modelType)
        {
        case aic::ModelType::Quail_L48:
            return aic::ui::ModelUiInfo(sample_rate, 10, 30, num_frames, output_delay);
        case aic::ModelType::Quail_L16:
            return aic::ui::ModelUiInfo(sample_rate, 10, 30, num_frames, output_delay);
        case aic::ModelType::Quail_L8:
            return aic::ui::ModelUiInfo(sample_rate, 10, 30, num_frames, output_delay);
        case aic::ModelType::Quail_S48:
            return aic::ui::ModelUiInfo(sample_rate, 10, 30, num_frames, output_delay);
        case aic::ModelType::Quail_S16:
            return aic::ui::ModelUiInfo(sample_rate, 10, 30, num_frames, output_delay);
        case aic::ModelType::Quail_S8:
            return aic::ui::ModelUiInfo(sample_rate, 10, 30, num_frames, output_delay);
        case aic::ModelType::Quail_XS:
            return aic::ui::ModelUiInfo(sample_rate, 10, 10, num_frames, output_delay);
        case aic::ModelType::Quail_XXS:
            return aic::ui::ModelUiInfo(sample_rate, 10, 10, num_frames, output_delay);
        }
    }

    bool wasPrepareCalled() const
    {
        return m_prepareWasCalled.load();
    }

    void acknowledgePrepareCall()
    {
        m_prepareWasCalled.store(false);
    }

  private:
    std::atomic<bool> m_licenseValid = {false};

    // Define all models here
    inline static const std::array<ModelInfo, 8> modelInfos = {
        {{"Quail L48", aic::ModelType::Quail_L48},
         {"Quail L16", aic::ModelType::Quail_L16},
         {"Quail L8", aic::ModelType::Quail_L8},
         {"Quail S48", aic::ModelType::Quail_S48},
         {"Quail S16", aic::ModelType::Quail_S16},
         {"Quail S8", aic::ModelType::Quail_S8},
         {"Quail XS", aic::ModelType::Quail_XS},
         {"Quail XXS", aic::ModelType::Quail_XXS}}};

    static constexpr size_t numModels = modelInfos.size();

    // Array of unique_ptrs to aic::Model
    std::array<std::unique_ptr<aic::AicModel>, numModels> m_models;

    // Store license key as string
    std::string m_licenseKey;

    double            current_sample_rate;
    std::atomic<bool> m_prepareWasCalled{false};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicDemoAudioProcessor)
};
