#pragma once

#include <JuceHeader.h>

// Custom slider that forwards right-click to host for DAW automation
class HostContextSlider : public juce::Slider
{
public:
    HostContextSlider() = default;
    
    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            // Try to show host's context menu for automation (works in FL Studio)
            if (auto* pluginEditor = findParentComponentOfClass<juce::AudioProcessorEditor>())
            {
                if (auto* hostContext = pluginEditor->getHostContext())
                {
                    if (associatedParam != nullptr)
                    {
                        auto pos = e.getEventRelativeTo(pluginEditor).getPosition();
                        if (auto menuInfo = hostContext->getContextMenuForParameter(associatedParam))
                        {
                            menuInfo->getEquivalentPopupMenu().showMenuAsync(
                                juce::PopupMenu::Options()
                                    .withTargetScreenArea(juce::Rectangle<int>{}.withPosition(pluginEditor->localPointToGlobal(pos))));
                            return;
                        }
                    }
                }
            }
            // If host doesn't provide menu, do nothing (don't show JUCE default menu)
            return;
        }
        juce::Slider::mouseDown(e);
    }
    
    void setAssociatedParameter(juce::RangedAudioParameter* param)
    {
        associatedParam = param;
    }
    
private:
    juce::RangedAudioParameter* associatedParam = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HostContextSlider)
};

class RotaryKnobWithLabel : public juce::Component
{
public:
    RotaryKnobWithLabel()
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFF00D4AA));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider.setPopupMenuEnabled(true);
        
        addAndMakeVisible(slider);

        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colour(0xFF888899));
        label.setFont(juce::Font(juce::FontOptions(11.0f)));
        addAndMakeVisible(label);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto labelHeight = 18;
        
        label.setBounds(bounds.removeFromTop(labelHeight));
        slider.setBounds(bounds);
    }

    void setLabelText(const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
    }

    HostContextSlider& getSlider() { return slider; }

private:
    HostContextSlider slider;
    juce::Label label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnobWithLabel);
};
