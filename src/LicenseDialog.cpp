#include "LicenseDialog.h"

#include "AicColours.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace aic
{
namespace ui
{

class TextEditorLnf : public juce::LookAndFeel_V4
{
};

LicenseDialog::LicenseDialog(LicenseCallback callback) : m_licenseCallback(std::move(callback))
{
    setupComponents();
}

void LicenseDialog::setupComponents()
{
    // Title label
    m_titleLabel.setText("License Required", juce::dontSendNotification);
    m_titleLabel.setFont(
        juce::Font(juce::Font::getDefaultSansSerifFontName(), 18.0f, juce::Font::plain));
    m_titleLabel.setJustificationType(juce::Justification::left);
    m_titleLabel.setColour(juce::Label::textColourId, aic::ui::BLACK_100);
    addAndMakeVisible(m_titleLabel);

    // Instruction label
    m_instructionLabel.setText("Please enter your AIC SDK license key to continue using this "
                               "plugin. Contact: info@ai-coustics.com.",
                               juce::dontSendNotification);
    m_instructionLabel.setFont(
        juce::Font(juce::Font::getDefaultSansSerifFontName(), 16.0f, juce::Font::plain));
    m_instructionLabel.setJustificationType(juce::Justification::left);
    m_instructionLabel.setColour(juce::Label::textColourId, aic::ui::BLACK_60);
    addAndMakeVisible(m_instructionLabel);

    m_licenseLabel.setText("License Key", juce::dontSendNotification);
    m_licenseLabel.setFont(
        juce::Font(juce::Font::getDefaultSansSerifFontName(), 16.0f, juce::Font::plain));
    m_licenseLabel.setJustificationType(juce::Justification::left);
    m_licenseLabel.setColour(juce::Label::textColourId, aic::ui::BLACK_100);
    addAndMakeVisible(m_licenseLabel);

    // License key text editor
    m_licenseKeyEditor.setMultiLine(false);
    m_licenseKeyEditor.setReturnKeyStartsNewLine(false);
    m_licenseKeyEditor.setReadOnly(false);
    m_licenseKeyEditor.setScrollbarsShown(false);
    m_licenseKeyEditor.setCaretVisible(true);
    m_licenseKeyEditor.setPopupMenuEnabled(true);
    m_licenseKeyEditor.setFont(
        juce::Font(juce::Font::getDefaultSansSerifFontName(), 16.0f, juce::Font::plain));
    m_licenseKeyEditor.setColour(juce::TextEditor::backgroundColourId, aic::ui::BLACK_0);
    m_licenseKeyEditor.setColour(juce::TextEditor::textColourId, aic::ui::BLACK_100);
    m_licenseKeyEditor.setColour(juce::TextEditor::highlightedTextColourId, aic::ui::BLACK_100);
    m_licenseKeyEditor.setColour(juce::TextEditor::highlightColourId, aic::ui::BLUE_10);
    m_licenseKeyEditor.setColour(juce::TextEditor::outlineColourId, aic::ui::BLACK_20);
    m_licenseKeyEditor.setColour(juce::TextEditor::focusedOutlineColourId, aic::ui::BLACK_70);
    addAndMakeVisible(m_licenseKeyEditor);

    // OK button
    m_okButton.setButtonText("Activate License");
    m_okButton.setColour(juce::TextButton::buttonColourId, aic::ui::BLACK_100);
    m_okButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    m_okButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    m_okButton.addListener(this);
    addAndMakeVisible(m_okButton);

    // Set dialog size
    setSize(415, 300);
}

void LicenseDialog::paint(juce::Graphics& g)
{
    // Fill background with solid color to make dialog visible over overlay
    auto bounds = getLocalBounds();
    g.setColour(aic::ui::BLACK_0);
    g.fillRoundedRectangle(bounds.toFloat(), 16.f);
    g.setColour(aic::ui::BLACK_15);
    g.drawRoundedRectangle(bounds.toFloat(), 16.f, 1.f);
}

void LicenseDialog::resized()
{
    auto area = getLocalBounds().reduced(24);

    // Title
    m_titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(8); // spacing

    // Instruction text
    m_instructionLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(24); // spacing

    // License key input
    m_licenseLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);
    m_licenseKeyEditor.setBounds(area.removeFromTop(25));
    area.removeFromTop(20); // spacing

    // Buttons
    auto buttonArea = area.removeFromBottom(40);
    m_okButton.setBounds(buttonArea.removeFromRight(147));
}

void LicenseDialog::buttonClicked(juce::Button* button)
{
    if (button == &m_okButton)
    {
        handleOkAction();
    }
}

bool LicenseDialog::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::returnKey)
    {
        handleOkAction();
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
                                               "Invalid License Key", "Please enter a license key.",
                                               "OK");
        return;
    }

    if (m_licenseCallback && m_licenseCallback(licenseKey))
    {
        closeDialog();
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "Invalid License Key",
                                               "The license key you entered is not valid.", "OK");
        m_licenseKeyEditor.selectAll();
        m_licenseKeyEditor.grabKeyboardFocus();
    }
}

void LicenseDialog::setCloseCallback(CloseCallback callback)
{
    m_closeCallback = std::move(callback);
}

void LicenseDialog::handleCancelAction()
{
    closeDialog();
}

void LicenseDialog::showDialog(juce::Component* parentComponent)
{
    if (parentComponent == nullptr)
        return;

    // Get the top-level component (usually the plugin window)
    auto* topLevel = parentComponent->getTopLevelComponent();
    if (topLevel == nullptr)
        topLevel = parentComponent;

    // Center the dialog on screen coordinates
    auto screenBounds = topLevel->getScreenBounds();
    auto dialogBounds =
        juce::Rectangle<int>(getWidth(), getHeight()).withCentre(screenBounds.getCentre());

    // Convert to local coordinates of the parent
    dialogBounds = parentComponent->getLocalArea(nullptr, dialogBounds);
    setBounds(dialogBounds);

    // Add as child component
    parentComponent->addAndMakeVisible(this);

    // Focus the license input
    focusLicenseInput();

    // This will deactivate the component below
    enterModalState(true);
}

void LicenseDialog::closeDialog()
{
    if (auto* parent = getParentComponent())
    {
        parent->removeChildComponent(this);
    }

    exitModalState(0);

    // Call the close callback if set
    if (m_closeCallback)
    {
        m_closeCallback();
    }
}

void LicenseDialog::focusLicenseInput()
{
    m_licenseKeyEditor.grabKeyboardFocus();
}

} // namespace ui
} // namespace aic
