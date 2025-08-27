#pragma once

#include "AicColours.h"
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

namespace aic
{
namespace ui
{

struct TextEditorLnF : juce::LookAndFeel_V4
{
    void drawTextEditorOutline(juce::Graphics &g, int width, int height, juce::TextEditor &textEditor) override {
        if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
        {
            g.setColour (textEditor.findColour (juce::TextEditor::focusedOutlineColourId));
            g.drawRoundedRectangle(0.f, 0.f, (float)width, (float)height, 8.f, 2.f);
        }
        else
        {
            g.setColour (textEditor.findColour (juce::TextEditor::outlineColourId));
            g.drawRoundedRectangle(0.f, 0.f, (float)width, (float)height, 8.f, 2.f);
        }
    }

    juce::CaretComponent* createCaretComponent (juce::Component* keyFocusOwner) override {
        auto caret = new juce::CaretComponent (keyFocusOwner);
        caret->setColour(juce::CaretComponent::ColourIds::caretColourId, aic::ui::BLACK_70);
        return caret;
    }
};

/**
 * @brief A simple error dialog component for displaying error messages.
 *
 * This component presents a modal error dialog window that displays an error message
 * with an OK button to dismiss it. The dialog is styled consistently with the
 * LicenseDialog and provides a custom alternative to JUCE's AlertWindow.
 */
class ErrorDialog : public juce::Component, public juce::Button::Listener
{
  public:
    /**
     * @brief Callback function type for handling dialog close events.
     */
    using CloseCallback = std::function<void()>;

    /**
     * @brief Constructs a new Error Dialog.
     *
     * @param title The title text to display in the dialog
     * @param message The error message to display
     */
    ErrorDialog(const juce::String& title, const juce::String& message);

    /**
     * @brief Destructor.
     */
    ~ErrorDialog() override = default;

    //==============================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Button::Listener overrides
    void buttonClicked(juce::Button* button) override;

    //==============================================================================
    // Component overrides
    bool keyPressed(const juce::KeyPress& key) override;

    /**
     * @brief Shows the error dialog as a modal component.
     *
     * @param parentComponent The parent component to attach this dialog to
     */
    void showDialog(juce::Component* parentComponent);

    /**
     * @brief Sets a callback to be invoked when the dialog is closed.
     *
     * @param callback Function to call when the dialog closes
     */
    void setCloseCallback(CloseCallback callback);

    /**
     * @brief Closes the dialog.
     */
    void closeDialog();

  private:
    //==============================================================================
    // UI Components
    std::unique_ptr<juce::Drawable> m_alertTriangle;
    juce::Label                     m_titleLabel;
    juce::Label                     m_messageLabel;
    juce::TextButton                m_okButton;

    // Callback for dialog close events
    CloseCallback m_closeCallback;

    /**
     * @brief Sets up the UI components with appropriate styling and properties.
     */
    void setupComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ErrorDialog)
};

/**
 * @brief A dialog component for entering and validating license keys.
 *
 * This component presents a modal dialog window that prompts the user to enter
 * a license key. The dialog includes:
 * - A text editor for license key input
 * - OK and Cancel buttons
 * - Callback functionality for handling the entered license key
 *
 * The dialog is designed to be shown when license validation fails and the
 * application needs user input to proceed with licensing.
 */
class LicenseDialog : public juce::Component, public juce::Button::Listener
{
  public:
    /**
     * @brief Callback function type for handling license key submission.
     *
     * @param licenseKey The license key entered by the user
     * @return true if the license key was accepted, false otherwise
     */
    using LicenseCallback = std::function<bool(const juce::String& licenseKey)>;

    /**
     * @brief Callback function type for handling dialog close events.
     *
     * This callback is invoked whenever the dialog is closed, regardless of
     * whether it was closed via successful validation, cancellation, or other means.
     */
    using CloseCallback = std::function<void()>;

    /**
     * @brief Constructs a new License Dialog.
     *
     * @param callback Function to call when the user submits a license key
     */
    explicit LicenseDialog(LicenseCallback callback);

    /**
     * @brief Destructor.
     */
    ~LicenseDialog() override = default;

    //==============================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Button::Listener overrides
    void buttonClicked(juce::Button* button) override;

    //==============================================================================
    // Component overrides
    bool keyPressed(const juce::KeyPress& key) override;

    /**
     * @brief Shows the dialog as a modal component.
     *
     * @param parentComponent The parent component to attach this dialog to
     */
    void showDialog(juce::Component* parentComponent);

    /**
     * @brief Sets a callback to be invoked when the dialog is closed.
     *
     * This callback will be called whenever the dialog closes, regardless of
     * whether it was closed via successful validation, cancellation, or other means.
     *
     * @param callback Function to call when the dialog closes
     */
    void setCloseCallback(CloseCallback callback);

    /**
     * @brief Closes the dialog.
     */
    void closeDialog();

    /**
     * @brief Sets focus to the license key text editor.
     */
    void focusLicenseInput();

  private:
    //==============================================================================
    // UI Components
    TextEditorLnF m_textEditorLnf;
    std::unique_ptr<juce::Drawable> m_alertTriangle;

    juce::Label      m_titleLabel;
    juce::Label      m_instructionLabel;
    juce::Label      m_licenseLabel;
    juce::TextEditor m_licenseKeyEditor;
    juce::TextButton m_okButton;
    juce::TextButton m_cancelButton;

    // Callback for license validation
    LicenseCallback m_licenseCallback;

    // Callback for dialog close events
    CloseCallback m_closeCallback;

    // Modal component for showing the dialog
    std::unique_ptr<juce::Component> m_modalComponent;

    // Error dialog for custom error messages
    std::unique_ptr<ErrorDialog> m_errorDialog;

    /**
     * @brief Handles the OK button click or Enter key press.
     */
    void handleOkAction();

    /**
     * @brief Handles the Cancel button click or Escape key press.
     */
    void handleCancelAction();

    /**
     * @brief Sets up the UI components with appropriate styling and properties.
     */
    void setupComponents();

    /**
     * @brief Shows a custom error dialog with the specified title and message.
     *
     * @param title The title text to display in the error dialog
     * @param message The error message to display
     */
    void showErrorDialog(const juce::String& title, const juce::String& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LicenseDialog)
};

} // namespace ui
} // namespace aic
