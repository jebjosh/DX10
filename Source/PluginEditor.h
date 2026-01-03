#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "RotaryKnobWithLabel.h"
#include "SpectrumAnalyzer.h"
#include "PresetManager.h"
#include <vector>
#include <map>
#include <algorithm>
#include <functional>

class DX10AudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::AudioProcessorValueTreeState::Listener,
                                  public juce::FileDragAndDropTarget
{
public:
    DX10AudioProcessorEditor(DX10AudioProcessor&);
    ~DX10AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    DX10AudioProcessor& audioProcessor;
    DX10LookAndFeel customLookAndFeel;

    // Preset management
    std::unique_ptr<PresetManager> presetManager;
    juce::ComboBox presetSelector;
    juce::TextButton savePresetButton { "Save" };
    juce::TextButton loadPresetButton { "Load" };
    juce::TextButton prevPresetButton { "<" };
    juce::TextButton nextPresetButton { ">" };
    juce::TextButton settingsButton { "..." };
    bool isUpdatingPresetSelector = false;
    bool isDragOver = false;

    // Undo/Redo buttons
    juce::TextButton undoButton { "Undo" };
    juce::TextButton redoButton { "Redo" };

    // Spectrum Analyzer
    SpectrumAnalyzer spectrumAnalyzer;

    // Knobs
    RotaryKnobWithLabel attackKnob, decayKnob, releaseKnob;
    RotaryKnobWithLabel coarseKnob, fineKnob;
    RotaryKnobWithLabel modInitKnob, modDecKnob, modSusKnob, modRelKnob, modVelKnob;
    RotaryKnobWithLabel octaveKnob, fineTuneKnob;
    RotaryKnobWithLabel vibratoKnob, waveformKnob, modThruKnob, lfoRateKnob;
    RotaryKnobWithLabel gainKnob, saturationKnob, glideKnob;

    // Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment, releaseAttachment;
    std::unique_ptr<SliderAttachment> coarseAttachment, fineAttachment;
    std::unique_ptr<SliderAttachment> modInitAttachment, modDecAttachment, modSusAttachment, modRelAttachment, modVelAttachment;
    std::unique_ptr<SliderAttachment> octaveAttachment, fineTuneAttachment;
    std::unique_ptr<SliderAttachment> vibratoAttachment, waveformAttachment, modThruAttachment, lfoRateAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment, saturationAttachment, glideAttachment;

    void setupKnob(RotaryKnobWithLabel& knob, const juce::String& labelText);
    void drawSection(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title);
    void updatePresetSelectorFromParameter();
    void savePresetToFile();
    void loadPresetFromFile();
    void goToPreviousPreset();
    void goToNextPreset();
    void rebuildPresetList();
    void showSettingsMenu();
    void selectPresetFolder();
    int generatePresetIdFromFile(const juce::File& file);

    juce::ComponentBoundsConstrainer constrainer;

    // Preset list data
    int numFactoryPresets = 0;
    std::vector<FlatPresetItem> userPresets;
    
    // Map combo box IDs to preset files (ID -> File)
    std::map<int, juce::File> presetIdToFile;
    
    // Map file paths to preset IDs (Path -> ID) for reverse lookup
    std::map<juce::String, int> fileToPresetId;
    
    // Track the last loaded user preset for undo/display purposes
    juce::File lastLoadedUserPreset;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DX10AudioProcessorEditor);
};
