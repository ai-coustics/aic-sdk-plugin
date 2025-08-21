#include "PluginEditor.h"

#include "AicColours.h"
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
    setSize(454, 588);
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
    g.drawText("Model", 35, 160, 385, 24, juce::Justification::centredLeft);
    modelSelector.setBounds(35, 192, 385, 40);

    modelInfoBox.setTopLeftPosition(35, 240);

    auto sliderOverlap = 17;
    enhancementSlider.setTopLeftPosition(35 - sliderOverlap, 432);

    m_logo->setTransformToFit(juce::Rectangle<float>{35.f, 536.f, 100.f, 16.f},
                              juce::RectanglePlacement::centred);
}

void AicDemoAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void AicDemoAudioProcessorEditor::timerCallback()
{
    if (processorRef.wasPrepareCalled())
    {
        processorRef.acknowledgePrepareCall();
        updateModelInfo();
    }
}

void AicDemoAudioProcessorEditor::updateModelInfo()
{
    int selectedIndex = modelSelector.getSelectedItemIndex();

    auto modelInfo = processorRef.getModelInfo(selectedIndex);
    modelInfoBox.setModelInfo(modelInfo);
}
