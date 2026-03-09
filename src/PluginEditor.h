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

    bool m_speechDetected = false;
    bool m_lastLicenseState = false;

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

    void showModalOverlay();
    void hideModalOverlay();

    bool handleLicenseValidation(const juce::String& licenseKey);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicDemoAudioProcessorEditor)
};
