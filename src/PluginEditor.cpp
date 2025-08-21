#include "PluginEditor.h"

#include "AicColours.h"
#include "AicModelInfoBox.h"
#include "BinaryData.h"
#include "PluginProcessor.h"

#include <juce_graphics/juce_graphics.h>

//==============================================================================
AicDemoAudioProcessorEditor::AicDemoAudioProcessorEditor(AicDemoAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p),
      modelSelectorAttachment(p.state, "model", modelSelector),
      enhancementAttachment(p.state, "enhancement", enhancementSlider)
{
    getLookAndFeel().setDefaultSansSerifTypeface(getFont());

    addAndMakeVisible(enhancementSlider);

    addAndMakeVisible(modelSelector);
    modelSelector.addItemList(processorRef.getModelChoices(), 1);
    modelSelector.setSelectedItemIndex(
        static_cast<int>(processorRef.state.getRawParameterValue("model")->load()));
    addAndMakeVisible(modelInfoBox);

    m_logo = juce::Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize);
    addAndMakeVisible(m_logo.get());

    startTimer(100);

    setResizable(false, false);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(454, 490);
}

AicDemoAudioProcessorEditor::~AicDemoAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void AicDemoAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(aic::ui::BLACK_0);

    g.setColour(aic::ui::BLACK_70);

    auto bounds = getLocalBounds();
    bounds.reduce(35.f, 36.f);

    g.setFont(14.f);
    g.drawText("License Active", bounds.removeFromTop(16), juce::Justification::centredRight);

    g.setFont(16.f);
    g.drawText("Model", bounds.removeFromTop(24), juce::Justification::centredLeft);

    bounds.removeFromTop(8.f);

    modelSelector.setBounds(bounds.removeFromTop(40));

    bounds.removeFromTop(8.f);

    modelInfoBox.setBounds(bounds.removeFromTop(160));

    bounds.removeFromTop(24.f);

    enhancementSlider.setBounds(bounds.removeFromTop(80));

    bounds.removeFromTop(24.f);

    m_logo->setTransformToFit(bounds.removeFromTop(20).removeFromLeft(100).toFloat(),
                              juce::RectanglePlacement::centred);
}

void AicDemoAudioProcessorEditor::resized() {}

void AicDemoAudioProcessorEditor::timerCallback()
{
    if (processorRef.modelChanged())
    {
        processorRef.acknowledgeModelChanged();
        updateModelInfo();
    }
}

void AicDemoAudioProcessorEditor::updateModelInfo()
{
    auto modelInfo = processorRef.getModelInfo();
    modelInfoBox.setModelInfo(modelInfo);
}
