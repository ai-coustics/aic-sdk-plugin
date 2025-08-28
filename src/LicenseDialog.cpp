#include "LicenseDialog.h"

#include "AicColours.h"
#include "BinaryData.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

namespace aic
{
namespace ui
{
//==============================================================================
// ErrorDialog Implementation

ErrorDialog::ErrorDialog(const juce::String& title, const juce::String& message)
{
    setupComponents();
    m_titleLabel.setText(title, juce::dontSendNotification);
    m_messageLabel.setText(message, juce::dontSendNotification);
}

void ErrorDialog::setupComponents()
{
    m_alertTriangle =
        juce::Drawable::createFromImageData(BinaryData::alert_svg, BinaryData::alert_svgSize);
    addAndMakeVisible(m_alertTriangle.get());

    // Title label
    m_titleLabel.setFont(
        juce::Font(juce::Font::getDefaultSansSerifFontName(), 18.0f, juce::Font::plain));
    m_titleLabel.setJustificationType(juce::Justification::left);
    m_titleLabel.setColour(juce::Label::textColourId, aic::ui::BLACK_0);
    addAndMakeVisible(m_titleLabel);

    // Message label
    m_messageLabel.setFont(
        juce::Font(juce::Font::getDefaultSansSerifFontName(), 16.0f, juce::Font::plain));
    m_messageLabel.setJustificationType(juce::Justification::left);
    m_messageLabel.setColour(juce::Label::textColourId, aic::ui::BLACK_15);
    m_messageLabel.setMinimumHorizontalScale(1.0f); // Allow text wrapping
    addAndMakeVisible(m_messageLabel);

    // OK button
    m_okButton.setButtonText("OK");
    m_okButton.setColour(juce::TextButton::buttonColourId, aic::ui::BLACK_0);
    m_okButton.setColour(juce::TextButton::textColourOffId, aic::ui::BLACK_100);
    m_okButton.setColour(juce::TextButton::textColourOnId, aic::ui::BLACK_100);
    m_okButton.addListener(this);
    addAndMakeVisible(m_okButton);

    // Set dialog size
    setSize(350, 200);
}

void ErrorDialog::paint(juce::Graphics& g)
{
    // Fill background with solid color to make dialog visible over overlay
    auto bounds = getLocalBounds();
    g.setColour(aic::ui::BLACK_100);
    g.fillRoundedRectangle(bounds.toFloat(), 16.f);
    g.setColour(aic::ui::BLACK_0);
    g.drawRoundedRectangle(bounds.toFloat(), 16.f, 1.f);
}

void ErrorDialog::resized()
{
    auto area = getLocalBounds().reduced(24);

    auto firstLine = area.removeFromTop(24);
    // Alert Triangle
    m_alertTriangle->setTransformToFit(firstLine.removeFromLeft(24).reduced(0.f, 4.f).toFloat(),
                                       juce::RectanglePlacement::centred);
    firstLine.removeFromLeft(8.f);
    // Title
    m_titleLabel.setBounds(firstLine);
    area.removeFromTop(8); // spacing

    // Message text - calculate required height for multiline text
    auto messageArea = area.removeFromTop(60);
    m_messageLabel.setBounds(messageArea);

    // Get actual text height and adjust if needed
    auto font       = m_messageLabel.getFont();
    auto textWidth  = static_cast<float>(messageArea.getWidth());
    auto textHeight = font.getStringWidth(m_messageLabel.getText()) / textWidth * font.getHeight();
    if (textHeight > static_cast<float>(messageArea.getHeight()))
    {
        // Adjust dialog size if text is too long
        auto extraHeight = static_cast<int>(textHeight) - messageArea.getHeight();
        setSize(getWidth(), getHeight() + extraHeight);
    }

    area.removeFromTop(24); // spacing

    // Button
    m_okButton.setBounds(area.removeFromTop(40).removeFromRight(80));
}

void ErrorDialog::buttonClicked(juce::Button* button)
{
    if (button == &m_okButton)
    {
        closeDialog();
    }
}

bool ErrorDialog::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::returnKey || key == juce::KeyPress::escapeKey)
    {
        closeDialog();
        return true;
    }

    return false;
}

void ErrorDialog::showDialog(juce::Component* parentComponent)
{
    if (parentComponent == nullptr)
        return;

    // Center the dialog within the parent component's bounds
    // This avoids coordinate system conversion issues with scaled plugins
    auto parentBounds = parentComponent->getLocalBounds();
    auto dialogBounds =
        juce::Rectangle<int>(getWidth(), getHeight()).withCentre(parentBounds.getCentre());
    setBounds(dialogBounds);

    // Add as child component
    parentComponent->addAndMakeVisible(this);

    // This will deactivate the component below
    enterModalState(true);
}

void ErrorDialog::setCloseCallback(CloseCallback callback)
{
    m_closeCallback = std::move(callback);
}

void ErrorDialog::closeDialog()
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

//==============================================================================
// LicenseDialog Implementation

LicenseDialog::LicenseDialog(LicenseCallback callback, bool isLicenseActive)
    : m_licenseCallback(std::move(callback)), m_isLicenseActive(isLicenseActive)
{
    setupComponents();
}

void LicenseDialog::setupComponents()
{
    m_alertTriangle =
        juce::Drawable::createFromImageData(BinaryData::alert_svg, BinaryData::alert_svgSize);
    addAndMakeVisible(m_alertTriangle.get());

    // Title label
    if (m_isLicenseActive)
    {
        m_titleLabel.setText("Update License Key", juce::dontSendNotification);
    }
    else
    {
        m_titleLabel.setText("License Required", juce::dontSendNotification);
    }
    m_titleLabel.setFont(
        juce::Font(juce::Font::getDefaultSansSerifFontName(), 18.0f, juce::Font::plain));
    m_titleLabel.setJustificationType(juce::Justification::left);
    m_titleLabel.setColour(juce::Label::textColourId, aic::ui::BLACK_100);
    addAndMakeVisible(m_titleLabel);

    // Instruction label
    if (m_isLicenseActive)
    {
        m_instructionLabel.setText("Your current license is active. You can update to a "
                                   "different license key if needed. To acquire a new key, "
                                   "contact: info@ai-coustics.com",
                                   juce::dontSendNotification);
    }
    else
    {
        m_instructionLabel.setText("Please enter your license key to continue using this "
                                   "plugin. To acquire a key, contact: info@ai-coustics.com",
                                   juce::dontSendNotification);
    }
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
    m_licenseKeyEditor.setJustification(juce::Justification::centredLeft);
    m_licenseKeyEditor.setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(
                                          0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopq"
                                             "rstuvwxyz!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"),
                                      true);
    m_licenseKeyEditor.setFont(
        juce::Font(juce::Font::getDefaultSansSerifFontName(), 16.0f, juce::Font::plain));
    m_licenseKeyEditor.setColour(juce::TextEditor::backgroundColourId, aic::ui::BLACK_0);
    m_licenseKeyEditor.setColour(juce::TextEditor::textColourId, aic::ui::BLACK_100);
    m_licenseKeyEditor.setColour(juce::TextEditor::highlightedTextColourId, aic::ui::BLACK_100);
    m_licenseKeyEditor.setColour(juce::TextEditor::highlightColourId, aic::ui::BLUE_10);
    m_licenseKeyEditor.setColour(juce::TextEditor::outlineColourId, aic::ui::BLACK_20);
    m_licenseKeyEditor.setColour(juce::TextEditor::focusedOutlineColourId, aic::ui::BLACK_40);
    m_licenseKeyEditor.setLookAndFeel(&m_textEditorLnf);
    addAndMakeVisible(m_licenseKeyEditor);

    // OK button
    if (m_isLicenseActive)
    {
        m_okButton.setButtonText("Update License");
    }
    else
    {
        m_okButton.setButtonText("Activate License");
    }
    m_okButton.setColour(juce::TextButton::buttonColourId, aic::ui::BLACK_100);
    m_okButton.setColour(juce::TextButton::textColourOffId, aic::ui::BLACK_0);
    m_okButton.setColour(juce::TextButton::textColourOnId, aic::ui::BLACK_0);
    m_okButton.addListener(this);
    addAndMakeVisible(m_okButton);

    m_cancelButton.setButtonText("Cancel");
    m_cancelButton.setColour(juce::TextButton::buttonColourId, aic::ui::BLACK_0);
    m_cancelButton.setColour(juce::TextButton::textColourOffId, aic::ui::BLACK_70);
    m_cancelButton.setColour(juce::TextButton::textColourOnId, aic::ui::BLACK_70);
    m_cancelButton.addListener(this);
    addAndMakeVisible(m_cancelButton);

    // Set dialog size
    setSize(415, 298);
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

    auto firstLine = area.removeFromTop(24);
    // Alert Triangle
    m_alertTriangle->setTransformToFit(firstLine.removeFromLeft(24).reduced(0.f, 4.f).toFloat(),
                                       juce::RectanglePlacement::centred);
    firstLine.removeFromLeft(8.f);
    // Title
    m_titleLabel.setBounds(firstLine);
    area.removeFromTop(8); // spacing

    // Instruction text
    m_instructionLabel.setBounds(area.removeFromTop(50));
    area.removeFromTop(24); // spacing

    // License key input
    m_licenseLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);
    m_licenseKeyEditor.setBounds(area.removeFromTop(40));
    area.removeFromTop(32); // spacing

    // Buttons
    auto buttonArea = area.removeFromTop(40);
    m_okButton.setBounds(buttonArea.removeFromRight(147));
    buttonArea.removeFromRight(8);
    m_cancelButton.setBounds(buttonArea.removeFromRight(81));
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

    return false;
}

void LicenseDialog::handleOkAction()
{
    auto licenseKey = m_licenseKeyEditor.getText();

    if (licenseKey.trim().isEmpty())
    {
        m_isLicenseActive = false;
        showErrorDialog("Invalid License Key", "Please enter a license key.");
        return;
    }

    if (m_licenseCallback && m_licenseCallback(licenseKey))
    {
        m_isLicenseActive = true;
        closeDialog();
    }
    else
    {
        showErrorDialog("Invalid License Key", "The license key you entered is not valid.");
        m_isLicenseActive = false;
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

    // Center the dialog within the parent component's bounds
    // This avoids coordinate system conversion issues with scaled plugins
    auto parentBounds = parentComponent->getLocalBounds();
    auto dialogBounds =
        juce::Rectangle<int>(getWidth(), getHeight()).withCentre(parentBounds.getCentre());
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
    // Clear the license key input field
    m_licenseKeyEditor.clear();

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

void LicenseDialog::showErrorDialog(const juce::String& title, const juce::String& message)
{
    // Close any existing error dialog
    if (m_errorDialog)
    {
        m_errorDialog->closeDialog();
        m_errorDialog.reset();
    }

    // Create and show new error dialog
    m_errorDialog = std::make_unique<ErrorDialog>(title, message);
    m_errorDialog->setCloseCallback([this]() { m_errorDialog.reset(); });
    m_errorDialog->showDialog(this);
}

} // namespace ui
} // namespace aic
