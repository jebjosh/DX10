#pragma once

#include <JuceHeader.h>

class DX10LookAndFeel : public juce::LookAndFeel_V4
{
public:
    DX10LookAndFeel()
    {
        // Define our color scheme - sleek dark theme with cyan accents
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF00D4AA));      // Cyan accent
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF2A2A35));   // Dark track
        setColour(juce::Slider::thumbColourId, juce::Colour(0xFFFFFFFF));                  // White thumb
        setColour(juce::Label::textColourId, juce::Colour(0xFFCCCCCC));                    // Light gray text
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1E1E28));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xFFFFFFFF));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF3A3A45));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF1E1E28));
        setColour(juce::PopupMenu::textColourId, juce::Colour(0xFFFFFFFF));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF00D4AA));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xFF000000));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Background circle with subtle gradient
        {
            juce::ColourGradient bgGrad(juce::Colour(0xFF2A2A35), centreX, ry,
                                         juce::Colour(0xFF1A1A22), centreX, ry + rw, false);
            g.setGradientFill(bgGrad);
            g.fillEllipse(rx, ry, rw, rw);
        }

        // Outer ring (dark)
        g.setColour(juce::Colour(0xFF0D0D12));
        g.drawEllipse(rx, ry, rw, rw, 2.0f);

        // Arc track (background)
        {
            juce::Path arcBg;
            arcBg.addCentredArc(centreX, centreY, radius * 0.75f, radius * 0.75f,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
            g.setColour(juce::Colour(0xFF1A1A22));
            g.strokePath(arcBg, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }

        // Arc track (filled portion with gradient)
        if (sliderPosProportional > 0.0f)
        {
            juce::Path arcFill;
            arcFill.addCentredArc(centreX, centreY, radius * 0.75f, radius * 0.75f,
                                  0.0f, rotaryStartAngle, angle, true);
            
            juce::ColourGradient arcGrad(juce::Colour(0xFF00D4AA), rx, centreY,
                                          juce::Colour(0xFF00A080), rx + rw, centreY, false);
            g.setGradientFill(arcGrad);
            g.strokePath(arcFill, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
        }

        // Inner knob with metallic gradient
        auto knobRadius = radius * 0.55f;
        {
            juce::ColourGradient knobGrad(juce::Colour(0xFF4A4A55), centreX, centreY - knobRadius,
                                           juce::Colour(0xFF252530), centreX, centreY + knobRadius, false);
            g.setGradientFill(knobGrad);
            g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
        }

        // Knob highlight ring
        g.setColour(juce::Colour(0xFF5A5A65));
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

        // Pointer/indicator line
        juce::Path pointer;
        auto pointerLength = knobRadius * 0.7f;
        auto pointerThickness = 3.0f;
        pointer.addRoundedRectangle(-pointerThickness * 0.5f, -knobRadius + 4.0f,
                                     pointerThickness, pointerLength, 1.5f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        
        g.setColour(juce::Colour(0xFF00D4AA));
        g.fillPath(pointer);

        // Glow effect on pointer
        g.setColour(juce::Colour(0xFF00D4AA).withAlpha(0.3f));
        g.strokePath(pointer, juce::PathStrokeType(2.0f));
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        g.fillAll(label.findColour(juce::Label::backgroundColourId));

        if (!label.isBeingEdited())
        {
            auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
            
            g.setFont(getCustomFont().withHeight(label.getFont().getHeight()));
            g.setColour(label.findColour(juce::Label::textColourId));
            
            g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                             juce::jmax(1, (int)(textArea.getHeight() / label.getFont().getHeight())),
                             label.getMinimumHorizontalScale());
        }
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        auto cornerSize = 6.0f;
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(0.5f);

        g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(box.findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

        // Arrow
        juce::Path arrow;
        auto arrowX = static_cast<float>(buttonX) + static_cast<float>(buttonW) * 0.5f;
        auto arrowY = static_cast<float>(height) * 0.5f;
        arrow.addTriangle(arrowX - 4.0f, arrowY - 2.0f,
                          arrowX + 4.0f, arrowY - 2.0f,
                          arrowX, arrowY + 3.0f);
        g.setColour(juce::Colour(0xFF00D4AA));
        g.fillPath(arrow);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return getCustomFont().withHeight(14.0f);
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return getCustomFont().withHeight(12.0f);
    }

    static juce::Font getCustomFont()
    {
        // Use a clean, modern sans-serif font
        return juce::Font(juce::FontOptions("Arial", 12.0f, juce::Font::plain));
    }

    static juce::Font getTitleFont()
    {
        return juce::Font(juce::FontOptions("Arial", 24.0f, juce::Font::bold));
    }

    static juce::Font getSectionFont()
    {
        return juce::Font(juce::FontOptions("Arial", 11.0f, juce::Font::bold));
    }
};
