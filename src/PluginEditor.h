#pragma once

#include "AicModelInfoBox.h"
#include "AicModelSelector.h"
#include "AicSlider.h"
#include "BinaryData.h"
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

    void updateModelInfo();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicDemoAudioProcessorEditor)
};
