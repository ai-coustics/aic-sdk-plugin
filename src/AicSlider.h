#pragma once

#include "AicColours.h"

#include <cassert>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

namespace
{
struct SliderLnF : juce::LookAndFeel_V4
{
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float, float, const juce::Slider::SliderStyle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height);

        auto tooltipBounds = bounds.removeFromTop(18.f);
        auto triangleSize  = 4.f;

        // Tooltip
        if (slider.isMouseOverOrDragging())
        {
            // Tooltip Background
            auto tooltipWidth = 34.f;
            tooltipBounds.removeFromLeft(sliderPos - tooltipWidth / 2.f);
            tooltipBounds.removeFromRight(tooltipBounds.getWidth() - tooltipWidth);
            g.setColour(aic::ui::BLUE_10);
            g.fillRoundedRectangle(tooltipBounds, 4.f);

            // Tooltip Text
            g.setColour(aic::ui::BLACK_70);
            g.setFont(11.f);
            g.drawText(juce::String(juce::roundToInt(slider.getValue() * 100.0) / 100.0),
                       tooltipBounds.toNearestInt(), juce::Justification::centred);

            // Triangle pointing down from tooltip
            auto triangleX = tooltipBounds.getCentreX();
            auto triangleY = tooltipBounds.getBottom();

            juce::Path triangle;
            triangle.addTriangle(triangleX - triangleSize, triangleY, triangleX + triangleSize,
                                 triangleY, triangleX, triangleY + triangleSize);

            g.setColour(aic::ui::BLUE_10);
            g.fillPath(triangle);
        }

        bounds.removeFromTop(triangleSize);

        // Slider
        auto sliderThumbWidth = 20.f;
        auto sliderBounds     = bounds.removeFromTop(sliderThumbWidth);

        // Background Path
        auto backgroundTrackBounds = sliderBounds.reduced(0.0, 7.f);
        g.setColour(aic::ui::BLUE_10);
        g.fillRoundedRectangle(backgroundTrackBounds, backgroundTrackBounds.getHeight() / 2.f);

        // Value Path
        auto valuePathBounds = backgroundTrackBounds.removeFromLeft(sliderPos);
        g.setColour(aic::ui::BLUE_50);
        g.fillRoundedRectangle(valuePathBounds, valuePathBounds.getHeight() / 2.f);

        // Thumb
        auto sliderThumbBounds = sliderBounds;
        auto sliderThumbRad    = sliderThumbWidth / 2.f;
        sliderThumbBounds.removeFromLeft(sliderPos - sliderThumbRad);
        sliderThumbBounds.removeFromRight((float) width - sliderPos - sliderThumbRad);
        g.setColour(aic::ui::BLUE_50);
        g.fillEllipse(sliderThumbBounds);
        g.setColour(aic::ui::BLACK_0);
        g.fillEllipse(sliderThumbBounds.reduced(3.f));

        // Labels`
        g.setColour(aic::ui::BLACK_60);
        g.setFont(14.f);
        g.drawText("0", bounds, juce::Justification::topLeft);
        g.drawText("1", bounds, juce::Justification::topRight);
    }

    auto getSliderLayout(juce::Slider& slider) -> juce::Slider::SliderLayout override
    {
        return {slider.getLocalBounds(), juce::Rectangle<int>(0, 0)};
    }
};
} // namespace

namespace aic::ui
{
class AicSlider : public juce::Slider
{
  public:
    explicit AicSlider()
    {
        setTextBoxStyle(NoTextBox, true, 0, 0);
        setSliderStyle(LinearHorizontal);

        setLookAndFeel(&m_lnf);
    }

    ~AicSlider() override
    {
        setLookAndFeel(nullptr);
    }

  private:
    SliderLnF m_lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicSlider)
};
} // namespace aic::ui
