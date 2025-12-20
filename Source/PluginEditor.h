#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "RotaryKnobWithLabel.h"

class DX10AudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::AudioProcessorValueTreeState::Listener
{
public:
    DX10AudioProcessorEditor(DX10AudioProcessor&);
    ~DX10AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Parameter listener callback
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    // Reference to the processor
    DX10AudioProcessor& audioProcessor;

    // Custom look and feel
    DX10LookAndFeel customLookAndFeel;

    // Preset selector
    juce::ComboBox presetSelector;
    juce::Label presetLabel;
    
    // Flag to prevent feedback loop when updating preset selector
    bool isUpdatingPresetSelector = false;

    // === Carrier Envelope Section ===
    RotaryKnobWithLabel attackKnob;
    RotaryKnobWithLabel decayKnob;
    RotaryKnobWithLabel releaseKnob;

    // === Modulator Ratio Section ===
    RotaryKnobWithLabel coarseKnob;
    RotaryKnobWithLabel fineKnob;

    // === Modulator Envelope Section ===
    RotaryKnobWithLabel modInitKnob;
    RotaryKnobWithLabel modDecKnob;
    RotaryKnobWithLabel modSusKnob;
    RotaryKnobWithLabel modRelKnob;
    RotaryKnobWithLabel modVelKnob;

    // === Tuning Section ===
    RotaryKnobWithLabel octaveKnob;
    RotaryKnobWithLabel fineTuneKnob;

    // === Output Section ===
    RotaryKnobWithLabel vibratoKnob;
    RotaryKnobWithLabel waveformKnob;
    RotaryKnobWithLabel modThruKnob;
    RotaryKnobWithLabel lfoRateKnob;

    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;
    std::unique_ptr<SliderAttachment> coarseAttachment;
    std::unique_ptr<SliderAttachment> fineAttachment;
    std::unique_ptr<SliderAttachment> modInitAttachment;
    std::unique_ptr<SliderAttachment> modDecAttachment;
    std::unique_ptr<SliderAttachment> modSusAttachment;
    std::unique_ptr<SliderAttachment> modRelAttachment;
    std::unique_ptr<SliderAttachment> modVelAttachment;
    std::unique_ptr<SliderAttachment> octaveAttachment;
    std::unique_ptr<SliderAttachment> fineTuneAttachment;
    std::unique_ptr<SliderAttachment> vibratoAttachment;
    std::unique_ptr<SliderAttachment> waveformAttachment;
    std::unique_ptr<SliderAttachment> modThruAttachment;
    std::unique_ptr<SliderAttachment> lfoRateAttachment;

    // Helper methods
    void setupKnob(RotaryKnobWithLabel& knob, const juce::String& labelText);
    void drawSection(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title);
    void updatePresetSelectorFromParameter();
    
    // Constraint for resizing
    juce::ComponentBoundsConstrainer constrainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DX10AudioProcessorEditor)
};
