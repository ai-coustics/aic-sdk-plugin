#pragma once

#include "AicColours.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace aic::ui
{
using namespace juce;
class ModelSelectorLnF : public juce::LookAndFeel_V4
{
    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int,
                      juce::ComboBox&) override
    {
        auto                 cornerSize = 8.f;
        juce::Rectangle<int> boxBounds(0, 0, width, height);

        g.setColour(aic::ui::BLACK_0);
        g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

        g.setColour(aic::ui::BLACK_20);
        g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);

        juce::Rectangle<int> arrowZone(width - 28, 18, 8, 4);
        juce::Path           path;
        path.startNewSubPath((float) arrowZone.getX(), (float) arrowZone.getY());
        path.lineTo((float) arrowZone.getCentreX(), (float) arrowZone.getBottom());
        path.lineTo((float) arrowZone.getRight(), (float) arrowZone.getY());

        g.setColour(aic::ui::BLACK_60);
        g.strokePath(path, juce::PathStrokeType(1.33f));
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return withDefaultMetrics(juce::FontOptions{14.f});
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(10, 1, box.getWidth() - 30, box.getHeight() - 2);

        label.setFont(getComboBoxFont(box));
    }

    void drawPopupMenuBackground(Graphics& g, int, int) override
    {
        g.fillAll(aic::ui::BLACK_0);
        // g.setColour(aic::ui::BLACK_20);
        // g.drawRoundedRectangle(0, 0, (float) width, (float) height, 8.f, 1.f);
    }

    void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area, const bool, const bool isActive,
                           const bool isHighlighted, const bool isTicked, const bool,
                           const String& text, const String&, const Drawable*,
                           const Colour* const) override
    {
        auto textColour = aic::ui::BLACK_100;

        auto r = area.reduced(1);

        if (isHighlighted && isActive)
        {
            g.setColour(aic::ui::BLUE_10);
            g.fillRect(r);

            g.setColour(aic::ui::BLACK_100);
        }
        else
        {
            g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
        }

        r.reduce(jmin(5, area.getWidth() / 20), 0);

        auto font = getPopupMenuFont();

        font.setHeight(14.f);

        g.setFont(font);

        auto iconArea = r.removeFromLeft(14).toFloat();

        if (isTicked)
        {
            auto tick = getTickShape(1.0f);
            g.fillPath(tick, tick.getTransformToScaleToFit(
                                 iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
        }

        r.removeFromRight(3);
        g.drawFittedText(text, r, Justification::centredLeft, 1);
    }
};

class AicModelSelector : public juce::ComboBox
{
  public:
    explicit AicModelSelector()
    {
        setColour(ComboBox::textColourId, aic::ui::BLACK_100);
        setLookAndFeel(&m_lnf);
    }

    ~AicModelSelector() override
    {
        setLookAndFeel(nullptr);
    }

  private:
    ModelSelectorLnF m_lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicModelSelector)
};

} // namespace aic::ui
