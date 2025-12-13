#pragma once

#include <JuceHeader.h>

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

    juce::Slider& getSlider() { return slider; }

private:
    juce::Slider slider;
    juce::Label label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnobWithLabel)
};
