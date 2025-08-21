#include "LicenseDialog.h"

#include "AicColours.h"

namespace aic
{
namespace ui
{

LicenseDialog::LicenseDialog(LicenseCallback callback) : m_licenseCallback(std::move(callback))
{
    setupComponents();
}

void LicenseDialog::setupComponents()
{
    // Title label
    m_titleLabel.setText("License Required", juce::dontSendNotification);
    m_titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    m_titleLabel.setJustificationType(juce::Justification::centred);
    m_titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(m_titleLabel);

    // Instruction label
    m_instructionLabel.setText(
        "Please enter your AIC SDK license key to continue using this plugin.",
        juce::dontSendNotification);
    m_instructionLabel.setFont(juce::Font(14.0f));
    m_instructionLabel.setJustificationType(juce::Justification::centred);
    m_instructionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(m_instructionLabel);

    // License key text editor
    m_licenseKeyEditor.setMultiLine(false);
    m_licenseKeyEditor.setReturnKeyStartsNewLine(false);
    m_licenseKeyEditor.setReadOnly(false);
    m_licenseKeyEditor.setScrollbarsShown(false);
    m_licenseKeyEditor.setCaretVisible(true);
    m_licenseKeyEditor.setPopupMenuEnabled(true);
    m_licenseKeyEditor.setFont(
        juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain));
    m_licenseKeyEditor.setColour(juce::TextEditor::backgroundColourId, aic::ui::BLACK_70);
    m_licenseKeyEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    m_licenseKeyEditor.setColour(juce::TextEditor::highlightColourId, aic::ui::BLUE_50);
    m_licenseKeyEditor.setColour(juce::TextEditor::outlineColourId, aic::ui::BLACK_60);
    m_licenseKeyEditor.setColour(juce::TextEditor::focusedOutlineColourId, aic::ui::BLUE_50);
    addAndMakeVisible(m_licenseKeyEditor);

    // OK button
    m_okButton.setButtonText("OK");
    m_okButton.setColour(juce::TextButton::buttonColourId, aic::ui::BLUE_50);
    m_okButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    m_okButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    m_okButton.addListener(this);
    addAndMakeVisible(m_okButton);

    // Cancel button
    m_cancelButton.setButtonText("Cancel");
    m_cancelButton.setColour(juce::TextButton::buttonColourId, aic::ui::BLACK_60);
    m_cancelButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    m_cancelButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    m_cancelButton.addListener(this);
    addAndMakeVisible(m_cancelButton);

    // Set dialog size
    setSize(400, 200);
}

void LicenseDialog::paint(juce::Graphics& g)
{
    // Fill background with semi-transparent dark color
    g.fillAll(aic::ui::BLACK_100);

    // Draw border
    g.setColour(aic::ui::BLACK_60);
    g.drawRect(getLocalBounds(), 1);
}

void LicenseDialog::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Title
    m_titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(10); // spacing

    // Instruction text
    m_instructionLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(15); // spacing

    // License key input
    m_licenseKeyEditor.setBounds(area.removeFromTop(25));
    area.removeFromTop(20); // spacing

    // Buttons
    auto buttonArea  = area.removeFromBottom(30);
    auto buttonWidth = 80;
    auto spacing     = 10;

    m_cancelButton.setBounds(buttonArea.removeFromRight(buttonWidth));
    buttonArea.removeFromRight(spacing);
    m_okButton.setBounds(buttonArea.removeFromRight(buttonWidth));
}

void LicenseDialog::buttonClicked(juce::Button* button)
{
    if (button == &m_okButton)
    {
        handleOkAction();
    }
    else if (button == &m_cancelButton)
    {
        handleCancelAction();
    }
}

bool LicenseDialog::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::returnKey)
    {
        handleOkAction();
        return true;
    }
    else if (key == juce::KeyPress::escapeKey)
    {
        handleCancelAction();
        return true;
    }

    return false;
}

void LicenseDialog::handleOkAction()
{
    auto licenseKey = m_licenseKeyEditor.getText();

    if (licenseKey.trim().isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "Invalid License Key",
                                               "Please enter a valid license key.", "OK");
        return;
    }

    if (m_licenseCallback && m_licenseCallback(licenseKey))
    {
        closeDialog();
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon, "Invalid License Key",
            "The license key you entered is not valid. Please check and try again.", "OK");
        m_licenseKeyEditor.selectAll();
        m_licenseKeyEditor.grabKeyboardFocus();
    }
}

void LicenseDialog::handleCancelAction()
{
    closeDialog();
}

void LicenseDialog::showDialog(juce::Component* parentComponent)
{
    if (parentComponent == nullptr)
        return;

    // Center the dialog on the parent component
    auto parentBounds = parentComponent->getLocalBounds();
    auto dialogBounds =
        juce::Rectangle<int>(getWidth(), getHeight()).withCentre(parentBounds.getCentre());
    setBounds(dialogBounds);

    // Add as modal component
    parentComponent->addAndMakeVisible(this);

    // Focus the license input
    focusLicenseInput();

    // Make it modal
    enterModalState(true);
}

void LicenseDialog::closeDialog()
{
    if (auto* parent = getParentComponent())
    {
        parent->removeChildComponent(this);
    }

    exitModalState(0);
}

void LicenseDialog::focusLicenseInput()
{
    m_licenseKeyEditor.grabKeyboardFocus();
}

} // namespace ui
} // namespace aic
