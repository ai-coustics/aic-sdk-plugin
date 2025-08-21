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
        assert(height >= 80);
        (void) height;

        auto tooltipWidth = 34.f;

        auto realX     = static_cast<float>(x) + tooltipWidth / 2.f;
        auto realWidth = static_cast<float>(width) - tooltipWidth;

        auto labelX      = realX;
        auto labelY      = static_cast<float>(y);
        auto labelWidth  = realWidth;
        auto labelHeight = 22.f;

        auto sliderPosReal = juce::jmap(sliderPos, static_cast<float>(x),
                                        static_cast<float>(x + width), realX, realX + realWidth);
        auto tooltipX      = sliderPosReal - 17.f;
        auto tooltipY      = labelY + labelHeight;
        auto tooltipHeight = 18.f;

        auto sliderPathX      = realX;
        auto sliderPathY      = tooltipY + tooltipHeight + 12.f;
        auto sliderPathWidth  = realWidth;
        auto sliderPathHeight = 6.f;

        auto sliderThumbX      = sliderPosReal - 10.f;
        auto sliderThumbY      = sliderPathY - 7.f;
        auto sliderThumbWidth  = 20.f;
        auto sliderThumbHeight = 20.f;

        auto valueLabelX      = realX;
        auto valueLabelY      = sliderThumbY + sliderThumbWidth;
        auto valueLabelWidth  = realWidth;
        auto valueLabelHeight = 16.f;

        // Slider Label
        g.setColour(aic::ui::BLACK_60);
        g.setFont(16.f);
        g.drawText("Enhancement Level", juce::roundToInt(labelX), juce::roundToInt(labelY),
                   juce::roundToInt(labelWidth), juce::roundToInt(labelHeight),
                   juce::Justification::centredLeft);

        // Tooltip
        if (slider.isMouseOverOrDragging())
        {
            g.setColour(aic::ui::BLUE_10);
            g.fillRoundedRectangle(tooltipX, tooltipY, tooltipWidth, tooltipHeight, 4.f);

            g.setColour(aic::ui::BLACK_70); // Black_70

            g.setFont(11.f);
            g.drawText(juce::String(juce::roundToInt(slider.getValue() * 100.0) / 100.0),
                       juce::roundToInt(tooltipX), juce::roundToInt(tooltipY),
                       juce::roundToInt(tooltipWidth), juce::roundToInt(tooltipHeight),
                       juce::Justification::centred);
        }

        // Background Path
        g.setColour(aic::ui::BLUE_10);
        g.fillRoundedRectangle(sliderPathX, sliderPathY, sliderPathWidth, sliderPathHeight,
                               sliderPathHeight / 2.f);

        // Value Path
        g.setColour(aic::ui::BLUE_50);
        auto valueWidth = sliderPosReal - sliderPathX;
        g.fillRoundedRectangle(sliderPathX, sliderPathY, valueWidth, sliderPathHeight,
                               sliderPathHeight / 2.f);

        // Thumb
        g.setColour(aic::ui::BLUE_50);
        g.fillEllipse(sliderThumbX, sliderThumbY, sliderThumbWidth, sliderThumbHeight);
        g.setColour(aic::ui::BLACK_0);
        g.fillEllipse(sliderThumbX + 3.f, sliderThumbY + 3.f, sliderThumbWidth - 6.f,
                      sliderThumbHeight - 6.f);

        // Labels
        g.setColour(aic::ui::BLACK_60);
        g.setFont(14.f);
        g.drawText("0", juce::roundToInt(valueLabelX), juce::roundToInt(valueLabelY),
                   juce::roundToInt(valueLabelWidth), juce::roundToInt(valueLabelHeight),
                   juce::Justification::centredLeft);
        g.drawText("1", juce::roundToInt(valueLabelX), juce::roundToInt(valueLabelY),
                   juce::roundToInt(valueLabelWidth), juce::roundToInt(valueLabelHeight),
                   juce::Justification::centredRight);
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

    void resized() override
    {
        // Ensure the component maintains its fixed size
        if (getWidth() != 419 || getHeight() != 80)
        {
            setSize(419, 80);
        }
        juce::Slider::resized();
    }

    bool hitTest(int x, int y) override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft(10);
        bounds.removeFromRight(10);
        bounds.removeFromTop(45);
        bounds.removeFromBottom(16);
        return bounds.contains(x, y);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        auto remappedEvent = e.withNewPosition(remapMousePosition(e.getPosition()));
        juce::Slider::mouseDown(remappedEvent);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        auto remappedEvent = e.withNewPosition(remapMousePosition(e.getPosition()));
        juce::Slider::mouseDrag(remappedEvent);
    }

  private:
    juce::Rectangle<int> getReducedBounds() const
    {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft(17);
        bounds.removeFromRight(17);
        bounds.removeFromTop(45);
        bounds.removeFromBottom(16);
        return bounds;
    }

    juce::Point<int> remapMousePosition(juce::Point<int> mousePos) const
    {
        auto fullBounds    = getLocalBounds();
        auto reducedBounds = getReducedBounds();

        // Remap X coordinate from reduced bounds to full bounds
        float ratio = (static_cast<float>(mousePos.x - reducedBounds.getX())) /
                      (float) reducedBounds.getWidth();
        ratio = juce::jlimit(0.0f, 1.0f, ratio);
        int remappedX =
            fullBounds.getX() + (int) (ratio * static_cast<float>(fullBounds.getWidth()));
        return juce::Point<int>(remappedX, mousePos.y);

        return mousePos; // For rotary sliders, no remapping needed
    }

    SliderLnF m_lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AicSlider)
};
} // namespace aic::ui
