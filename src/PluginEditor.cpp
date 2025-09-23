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
                      { return handleLicenseValidation(licenseKey); },
                      processorRef.isLicenseValid())
{
    // Set up close callback for license dialog to hide overlay when closed
    m_licenseDialog.setCloseCallback([this]() { hideModalOverlay(); });

    getLookAndFeel().setDefaultSansSerifTypeface(getFont());

    // Set up license button
    addAndMakeVisible(m_licenseButton);
    m_licenseButton.onClick = [this]()
    {
        showModalOverlay();
        m_licenseDialog.showDialog(this);
        m_licenseDialog.toFront(false);
    };
    updateLicenseButton();

    addAndMakeVisible(enhancementSlider);

    addAndMakeVisible(modelSelector);
    modelSelector.addItemList(processorRef.getModelChoices(), 1);
    modelSelector.setSelectedItemIndex(
        static_cast<int>(processorRef.state.getRawParameterValue("model")->load()));

    updateModelInfo();
    addAndMakeVisible(modelInfoBox);

    m_logo =
        juce::Drawable::createFromImageData(BinaryData::aic_logo_svg, BinaryData::aic_logo_svgSize);
    addAndMakeVisible(m_logo.get());

    // Check if license is valid and show dialog if needed
    if (!processorRef.isLicenseValid())
    {
        // Use a timer to show the license dialog after the component is fully constructed
        juce::Timer::callAfterDelay(100,
                                    [this]()
                                    {
                                        showModalOverlay();
                                        m_licenseDialog.showDialog(this);
                                        m_licenseDialog.toFront(false);
                                    });
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

    auto bounds = getLocalBounds();
    bounds.reduce(35.f, 33.f);

    // Position license button
    m_licenseButton.setBounds(bounds.removeFromTop(16).removeFromRight(120));

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

    enhancementSlider.setBounds(bounds.removeFromTop(54.f).expanded(7, 0));

    bounds.removeFromTop(24.f);

    m_logo->setTransformToFit(bounds.removeFromTop(20).removeFromLeft(100).toFloat(),
                              juce::RectanglePlacement::centred);
}

void AicDemoAudioProcessorEditor::resized()
{
    // Update modal overlay bounds if it's currently visible
    if (m_modalOverlay && m_modalOverlay->isVisible())
    {
        m_modalOverlay->setBounds(getLocalBounds());
    }
}

void AicDemoAudioProcessorEditor::timerCallback()
{
    static bool lastLicenseState    = processorRef.isLicenseValid();
    bool        currentLicenseState = processorRef.isLicenseValid();

    // Check if license state changed
    if (lastLicenseState != currentLicenseState)
    {
        lastLicenseState = currentLicenseState;
        updateLicenseButton(); // Update button text and color
        repaint();             // Repaint to update license status display

        // If license became invalid, show the dialog
        if (!currentLicenseState)
        {
            showModalOverlay();
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

void AicDemoAudioProcessorEditor::showModalOverlay()
{
    if (!m_modalOverlay)
    {
        m_modalOverlay = std::make_unique<ModalOverlay>();
    }

    addAndMakeVisible(m_modalOverlay.get());
    m_modalOverlay->setBounds(getLocalBounds());
    m_modalOverlay->toBack(); // Keep overlay behind other components
}

void AicDemoAudioProcessorEditor::hideModalOverlay()
{
    if (m_modalOverlay)
    {
        removeChildComponent(m_modalOverlay.get());
    }
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

                // Update license button text and color
                updateLicenseButton();

                // Trigger a repaint to update the license status display
                repaint();

                m_licenseDialog.setLicenseActive(true);

                return true; // License accepted, close dialog
            }
        }
    }
    m_licenseDialog.setLicenseActive(false);
    return false; // License invalid or save failed, keep dialog open
}

void AicDemoAudioProcessorEditor::updateLicenseButton()
{
    juce::String buttonText = processorRef.isLicenseValid() ? "License Active" : "License Invalid";
    juce::Colour buttonColor =
        processorRef.isLicenseValid() ? aic::ui::BLACK_70 : juce::Colours::red;

    m_licenseButton.setButtonText(buttonText);
    m_licenseButton.setColour(juce::TextButton::textColourOffId, buttonColor);
    m_licenseButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    m_licenseButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);

    // Make it look like a clickable text link
    m_licenseButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
}
