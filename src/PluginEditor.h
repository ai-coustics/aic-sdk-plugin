#pragma once

#include "AicModelInfoBox.h"
#include "AicModelSelector.h"
#include "AicSlider.h"
#include "BinaryData.h"
#include "LicenseDialog.h"
#include "PluginProcessor.h"

//==============================================================================
class AicDemoAudioProcessorEditor final : public juce::AudioProcessorEditor, public juce::Timer
{
  public:
    explicit AicDemoAudioProcessorEditor(AicDemoAudioProcessor&);
    ~AicDemoAudioProcessorEditor() override;

    void timerCallback() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    auto getFont() -> juce::Typeface::Ptr
    {
        return juce::Typeface::createSystemTypefaceFor(BinaryData::aic_font_otf,
                                                       BinaryData::aic_font_otfSize);
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

    aic::ui::LicenseDialog m_licenseDialog;

    juce::TextButton                m_licenseButton;
    std::unique_ptr<juce::Drawable> m_logo;

    // Modal overlay component for dimming background when dialog is shown
    class ModalOverlay : public juce::Component
    {
      public:
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colours::black.withAlpha(0.6f));
        }
    };

    std::unique_ptr<ModalOverlay> m_modalOverlay;

    void updateModelInfo();
    void updateLicenseButton();

    /**
     * @brief Shows a modal overlay that darkens the background.
     *
     * Creates and displays a semi-transparent overlay component that covers
     * the entire editor area, providing visual focus for modal dialogs.
     */
    void showModalOverlay();

    /**
     * @brief Hides the modal overlay.
     *
     * Removes the overlay component from the editor, returning the UI
     * to its normal appearance.
     */
    void hideModalOverlay();

    /**
     * @brief Handles license key validation when user submits a key through the dialog.
     *
     * This method is called as a callback from the LicenseDialog when the user
     * enters a license key and clicks OK. It validates the key using the processor,
     * saves it if valid, and updates the UI accordingly.
     *
     * @param licenseKey The license key entered by the user
     * @return true if the license key was valid and accepted, false otherwise
     */
    bool handleLicenseValidation(const juce::String& licenseKey);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicDemoAudioProcessorEditor)
};
