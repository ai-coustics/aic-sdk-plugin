#pragma once

#include "AicColours.h"

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace aic::ui
{

enum ModelState
{
    Initialized,
    WrongAudioSettings,
    LicenseInactive,
    ProcessingNotAllowed,
    NoModelLoaded,
};

struct ModelInfo
{
    std::string modelId;
    std::string optimalSampleRate;
    std::string optimalNumFrames;
    std::string outputDelay;
    ModelState  modelState;

    ModelInfo() = default;

    ModelInfo(const ModelState state) : modelState(state) {}

    ModelInfo(const std::string& id, const int sr, const int nf, const int od)
        : modelId(id), optimalSampleRate(std::to_string(sr) + " Hz"),
          optimalNumFrames(std::to_string(nf)), outputDelay(std::to_string(od) + " ms")
    {
        modelState = ModelState::Initialized;
    }
};

class AicModelInfoBox : public juce::Component

{
  public:
    AicModelInfoBox() {}

    void setModelInfo(const ModelInfo& info)
    {
        modelInfo = info;
        repaint();
    }

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
        case Initialized:
        {
            std::vector<std::pair<std::string, std::string>> infoLines = {
                {"Model", modelInfo.modelId},
                {"Optimal Sample Rate", modelInfo.optimalSampleRate},
                {"Optimal Num Frames", modelInfo.optimalNumFrames},
                {"Total Output Delay", modelInfo.outputDelay}};

            for (size_t i = 0; i < infoLines.size(); ++i)
            {
                auto line = bounds.removeFromTop(24);
                g.setFont(14.f);
                g.drawText(infoLines[i].first, line, juce::Justification::centredLeft);
                g.setFont(16.f);
                g.drawText(infoLines[i].second, line, juce::Justification::centredRight);

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
        case ProcessingNotAllowed:
        {
            g.setFont(16.f);
            g.drawText("Processing not allowed.", bounds, juce::Justification::centred);
            bounds.removeFromTop(30);
            g.drawText("Check your license and internet connection.", bounds,
                       juce::Justification::centred);
        }
        break;
        case NoModelLoaded:
        {
            g.setFont(16.f);
            g.drawText("No model loaded.", bounds, juce::Justification::centred);
            bounds.removeFromTop(30);
            g.setFont(14.f);
            g.drawText("Download models from artifacts.ai-coustics.io", bounds,
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
