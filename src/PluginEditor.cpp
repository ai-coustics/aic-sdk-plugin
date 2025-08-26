#include "PluginEditor.h"

#include "AicColours.h"
#include "AicModelInfoBox.h"
#include "BinaryData.h"
#include "PluginProcessor.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include <juce_graphics/juce_graphics.h>

//==============================================================================
AicDemoAudioProcessorEditor::AicDemoAudioProcessorEditor(AicDemoAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p),
      modelSelectorAttachment(p.state, "model", modelSelector),
      enhancementAttachment(p.state, "enhancement", enhancementSlider),
      m_licenseDialog([this](const juce::String& licenseKey)
                      { return handleLicenseValidation(licenseKey); })
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

    // Check if license is valid and show dialog if needed
    if (!processorRef.isLicenseValid())
    {
        // Use a timer to show the license dialog after the component is fully constructed
        juce::Timer::callAfterDelay(100, [this]() { m_licenseDialog.showDialog(this); });
    }

    startTimer(100);

    setResizable(false, false);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(454, 468);
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
    bounds.reduce(35.f, 33.f);

    g.setFont(14.f);
    juce::String licenseStatus =
        processorRef.isLicenseValid() ? "License Active" : "License Invalid";
    juce::Colour licenseColor =
        processorRef.isLicenseValid() ? aic::ui::BLACK_70 : juce::Colours::red;
    g.setColour(licenseColor);
    g.drawText(licenseStatus, bounds.removeFromTop(16), juce::Justification::centredRight);

    g.setColour(aic::ui::BLACK_70);
    g.setFont(16.f);
    g.drawText("Model", bounds.removeFromTop(24), juce::Justification::centredLeft);

    bounds.removeFromTop(8.f);

    modelSelector.setBounds(bounds.removeFromTop(40));

    bounds.removeFromTop(8.f);

    modelInfoBox.setBounds(bounds.removeFromTop(160));

    bounds.removeFromTop(24.f);

    g.setFont(16.f);
    g.drawText("Enhancement Level", bounds.removeFromTop(24), juce::Justification::centredLeft);

    enhancementSlider.setBounds(bounds.removeFromTop(54.f));

    bounds.removeFromTop(24.f);

    m_logo->setTransformToFit(bounds.removeFromTop(20).removeFromLeft(100).toFloat(),
                              juce::RectanglePlacement::centred);
}

void AicDemoAudioProcessorEditor::resized() {}

void AicDemoAudioProcessorEditor::timerCallback()
{
    static bool lastLicenseState    = processorRef.isLicenseValid();
    bool        currentLicenseState = processorRef.isLicenseValid();

    // Check if license state changed
    if (lastLicenseState != currentLicenseState)
    {
        lastLicenseState = currentLicenseState;
        repaint(); // Repaint to update license status display

        // If license became invalid, show the dialog
        if (!currentLicenseState)
        {
            m_licenseDialog.showDialog(this);
        }
    }

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

bool AicDemoAudioProcessorEditor::handleLicenseValidation(const juce::String& licenseKey)
{
    // Trim whitespace from the license key
    juce::String trimmedKey = licenseKey.trim();

    if (trimmedKey.isEmpty())
    {
        return false; // Empty license key is invalid
    }

    // Validate the license key using the processor
    if (processorRef.validateLicenseKey(trimmedKey))
    {
        // License is valid, save it and update the processor's license
        if (processorRef.saveLicenseKey(trimmedKey))
        {
            // Reload and validate the license in the processor
            if (processorRef.loadAndValidateLicense())
            {
                // Force recreation of the current model with the new license
                processorRef.forceModelRecreation();

                // Force update the model info since we now have a valid license
                updateModelInfo();

                // Trigger a repaint to update the license status display
                repaint();

                return true; // License accepted, close dialog
            }
        }
    }

    return false; // License invalid or save failed, keep dialog open
}
