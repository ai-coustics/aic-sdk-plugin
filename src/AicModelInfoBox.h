#pragma once

#include "AicColours.h"

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace aic::ui
{

enum ModelState
{
    Initilized,
    WrongAudioSettings,
    LicenseInactive,
};

struct ModelInfo
{
    std::string modelSampleRate;
    std::string modelWindowLength;
    std::string modelDelay;
    std::string optimalNumFrames;
    std::string currentOutputDelay;
    ModelState  modelState;

    // Constructor for easy initialization
    ModelInfo() = default;

    ModelInfo(const ModelState state) : modelState(state) {}

    ModelInfo(const int sr, const int w, const int md, const int nf, const int od)
        : modelSampleRate(std::to_string(sr) + " Hz"), modelWindowLength(std::to_string(w) + " ms"),
          modelDelay(std::to_string(md) + " ms"), optimalNumFrames(std::to_string(nf)),
          currentOutputDelay(std::to_string(od) + " ms")
    {
        modelState = ModelState::Initilized;
    }
};

class AicModelInfoBox : public juce::Component

{
  public:
    AicModelInfoBox() {}

    // Method to update model information
    void setModelInfo(const ModelInfo& info)
    {
        modelInfo = info;
        repaint(); // Trigger a repaint to show updated info
    }

    // Getter for current model info
    const ModelInfo& getModelInfo() const
    {
        return modelInfo;
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour(aic::ui::ROSA_TINT);
        g.fillRoundedRectangle(bounds.toFloat(), 8.f);

        bounds.reduce(16, 12);

        g.setColour(aic::ui::BLACK_100);

        switch (modelInfo.modelState)
        {
        case Initilized:
        {
            // Create a vector of label-value pairs for easy iteration
            std::vector<std::pair<std::string, std::string>> infoLines = {
                {"Model Sample Rate", modelInfo.modelSampleRate},
                {"Model Window Length", modelInfo.modelWindowLength},
                {"Model Delay", modelInfo.modelDelay},
                {"Optimal Num Frames", modelInfo.optimalNumFrames},
                {"Current Output Delay", modelInfo.currentOutputDelay}};

            // Draw each line
            for (size_t i = 0; i < infoLines.size(); ++i)
            {
                auto line = bounds.removeFromTop(24);
                g.setFont(14.f);
                g.drawText(infoLines[i].first, line, juce::Justification::centredLeft);
                g.setFont(16.f);
                g.drawText(infoLines[i].second, line, juce::Justification::centredRight);

                // Add spacing between lines (except after the last line)
                if (i < infoLines.size() - 1)
                    bounds.removeFromTop(6);
            }
            break;
        }
        case WrongAudioSettings:
        {
            g.setFont(16.f);
            g.drawText("Unsupported audio settings...", bounds, juce::Justification::centred);
        }
        break;
        case LicenseInactive:
        {
            g.setFont(16.f);
            g.drawText("No license found, open the dialog at the top right...", bounds,
                       juce::Justification::centred);
        }
        break;
        }
    }

    void setLicenseInvalid();

  private:
    ModelInfo modelInfo;
    bool      licenseInvalid;
};
} // namespace aic::ui
