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

        auto tooltipWidth  = 34.f;
        auto bubbleHeight  = 18.f;
        auto arrowHeight   = 4.f;
        auto tooltipHeight = bubbleHeight + arrowHeight;

        // Tooltip and thumb will otherwise move outside of the window
        auto sliderStart     = tooltipWidth / 2.f;
        auto sliderEnd       = width - tooltipWidth / 2.f;
        auto mappedSliderPos = juce::jmap(sliderPos, 0.f, (float) width, sliderStart, sliderEnd);

        // Always make space for the tooltip
        auto tooltipBounds = bounds.removeFromTop(tooltipHeight);

        // Draw Tooltip
        if (slider.isMouseOverOrDragging())
        {
            auto bubbleBounds = tooltipBounds.removeFromTop(bubbleHeight)
                                    .removeFromLeft(tooltipWidth)
                                    .withCentre({mappedSliderPos, bubbleHeight / 2.f});

            // Tooltip Background
            g.setColour(aic::ui::BLUE_10);
            g.fillRoundedRectangle(bubbleBounds, 4.f);

            // Tooltip Text
            g.setColour(aic::ui::BLACK_70);
            g.setFont(11.f);
            g.drawText(juce::String(juce::roundToInt(slider.getValue() * 100.0) / 100.0),
                       bubbleBounds.toNearestInt(), juce::Justification::centred);

            // Arrow pointing down from tooltip to thumb
            auto arrowX = bubbleBounds.getCentreX();
            auto arrowY = bubbleBounds.getBottom();

            juce::Path arrow;
            arrow.addTriangle(arrowX - arrowHeight, arrowY, arrowX + arrowHeight, arrowY, arrowX,
                              arrowY + arrowHeight);

            g.setColour(aic::ui::BLUE_10);
            g.fillPath(arrow);
        }

        // Slider
        auto sliderThumbWidth = 20.f;
        auto sliderBounds     = bounds.removeFromTop(sliderThumbWidth);

        // Background Path
        auto backgroundTrackBounds = sliderBounds.reduced(sliderStart, 7.f);
        g.setColour(aic::ui::BLUE_10);
        g.fillRoundedRectangle(backgroundTrackBounds, backgroundTrackBounds.getHeight() / 2.f);

        // Value Path
        auto valuePathBounds = backgroundTrackBounds.withRight(mappedSliderPos);
        g.setColour(aic::ui::BLUE_50);
        g.fillRoundedRectangle(valuePathBounds, valuePathBounds.getHeight() / 2.f);

        // Thumb
        auto sliderThumbBounds = sliderBounds.removeFromLeft(sliderThumbWidth)
                                     .withCentre({mappedSliderPos, sliderBounds.getCentreY()});
        g.setColour(aic::ui::BLUE_50);
        g.fillEllipse(sliderThumbBounds);
        g.setColour(aic::ui::BLACK_0);
        g.fillEllipse(sliderThumbBounds.reduced(3.f));

        // Labels
        g.setColour(aic::ui::BLACK_60);
        g.setFont(14.f);
        auto labelBounds = bounds.removeFromLeft(10.f);
        g.drawText("0", labelBounds.withCentre({sliderStart, bounds.getCentreY()}),
                   juce::Justification::centredTop);
        g.drawText("1", bounds.withCentre({sliderEnd, bounds.getCentreY()}),
                   juce::Justification::centredTop);
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
