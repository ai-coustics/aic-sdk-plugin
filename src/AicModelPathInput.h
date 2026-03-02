#pragma once

#include "AicColours.h"

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

namespace aic::ui
{

class ModelPathInputLnF : public juce::LookAndFeel_V4
{
    void drawButtonBackground(juce::Graphics& g, juce::Button&, const juce::Colour&, bool isOver,
                              bool) override
    {
        auto bounds = g.getClipBounds().toFloat();

        g.setColour(isOver ? aic::ui::BLUE_10 : aic::ui::BLACK_0);
        g.fillRoundedRectangle(bounds, 8.f);

        g.setColour(aic::ui::BLACK_20);
        g.drawRoundedRectangle(bounds.reduced(0.5f, 0.5f), 8.f, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        auto bounds = button.getLocalBounds().reduced(10, 0);

        g.setColour(aic::ui::BLACK_100);
        g.setFont(16.f);
        g.drawText(button.getButtonText(), bounds.removeFromLeft(bounds.getWidth() - 28),
                   juce::Justification::centredLeft, true);

        // Draw arrow/browse indicator
        juce::Rectangle<int> arrowZone(button.getWidth() - 28, 18, 8, 4);
        juce::Path           path;
        path.startNewSubPath((float) arrowZone.getX(), (float) arrowZone.getY());
        path.lineTo((float) arrowZone.getCentreX(), (float) arrowZone.getBottom());
        path.lineTo((float) arrowZone.getRight(), (float) arrowZone.getY());

        g.setColour(aic::ui::BLACK_60);
        g.strokePath(path, juce::PathStrokeType(1.33f));
    }
};

class AicModelPathInput : public juce::Component
{
  public:
    using PathCallback = std::function<void(const juce::String&)>;

    explicit AicModelPathInput(PathCallback callback) : m_callback(std::move(callback))
    {
        m_button.setButtonText("Select Model...");
        m_button.setLookAndFeel(&m_lnf);
        m_button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        m_button.onClick = [this]()
        {
            m_fileChooser = std::make_unique<juce::FileChooser>("Select Model File");
            m_fileChooser->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& fc)
                {
                    auto result = fc.getResult();
                    if (result.existsAsFile())
                    {
                        setPath(result.getFullPathName());
                        if (m_callback)
                            m_callback(result.getFullPathName());
                    }
                });
        };
        addAndMakeVisible(m_button);
    }

    ~AicModelPathInput() override
    {
        m_button.setLookAndFeel(nullptr);
    }

    void setPath(const juce::String& path)
    {
        m_path = path;
        if (path.isEmpty())
        {
            m_button.setButtonText("Select Model...");
        }
        else
        {
            juce::File file(path);
            m_button.setButtonText(file.getFileName());
        }
    }

    juce::String getPath() const
    {
        return m_path;
    }

    void resized() override
    {
        m_button.setBounds(getLocalBounds());
    }

  private:
    juce::TextButton                       m_button;
    ModelPathInputLnF                      m_lnf;
    PathCallback                           m_callback;
    std::unique_ptr<juce::FileChooser>     m_fileChooser;
    juce::String                           m_path;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicModelPathInput)
};

} // namespace aic::ui
