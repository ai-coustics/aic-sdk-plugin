#pragma once

#include "AicModelInfoBox.h"
#include "AicModelSelector.h"
#include "AicSlider.h"
#include "BinaryData.h"
#include "LicenseDialog.h"
#include "PluginProcessor.h"

//==============================================================================
class AicDemoAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                          public juce::ComboBox::Listener,
                                          public juce::Timer
{
  public:
    explicit AicDemoAudioProcessorEditor(AicDemoAudioProcessor&);
    ~AicDemoAudioProcessorEditor() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void timerCallback() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    auto getFont() -> juce::Typeface::Ptr
    {
        return juce::Typeface::createSystemTypefaceFor(BinaryData::milling_otf,
                                                       BinaryData::milling_otfSize);
    }

  private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AicDemoAudioProcessor& processorRef;

    aic::ui::AicModelSelector                              modelSelector;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment modelSelectorAttachment;

    aic::ui::AicModelInfoBox modelInfoBox;

    juce::Label                                          enhancementLabel{{}, "Enhancement Level"};
    aic::ui::AicSlider                                   enhancementSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment enhancementAttachment;

    std::unique_ptr<juce::Drawable> m_logo;

    // License dialog
    std::unique_ptr<aic::ui::LicenseDialog> m_licenseDialog;

    void updateModelInfo();

    /**
     * @brief Checks if a license is valid and shows the license dialog if needed.
     *
     * This method is called during initialization and periodically to ensure
     * the plugin has a valid license. If the license is invalid, it displays
     * a modal dialog prompting the user to enter a license key.
     */
    void checkLicenseAndShowDialog();

    /**
     * @brief Handles the license key validation callback from the dialog.
     *
     * This method is called when the user submits a license key through the
     * license dialog. It validates the key with the processor and returns
     * whether the validation was successful.
     *
     * @param licenseKey The license key entered by the user
     * @return true if the license key was valid and accepted, false otherwise
     */
    bool onLicenseKeySubmitted(const juce::String& licenseKey);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicDemoAudioProcessorEditor)
};
