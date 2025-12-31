#include "PluginEditor.h"

DX10AudioProcessorEditor::DX10AudioProcessorEditor(DX10AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);

    // Initialize preset manager
    presetManager = std::make_unique<PresetManager>(audioProcessor.apvts);

    // Connect spectrum analyzer to processor
    audioProcessor.setSpectrumAnalyzer(&spectrumAnalyzer);
    addAndMakeVisible(spectrumAnalyzer);

    // Setup knobs
    setupKnob(attackKnob, "ATTACK"); setupKnob(decayKnob, "DECAY"); setupKnob(releaseKnob, "RELEASE");
    setupKnob(coarseKnob, "COARSE"); setupKnob(fineKnob, "FINE");
    setupKnob(modInitKnob, "INIT"); setupKnob(modDecKnob, "DECAY"); setupKnob(modSusKnob, "SUSTAIN");
    setupKnob(modRelKnob, "RELEASE"); setupKnob(modVelKnob, "VEL SENS");
    setupKnob(octaveKnob, "OCTAVE"); setupKnob(fineTuneKnob, "FINE TUNE");
    setupKnob(vibratoKnob, "VIBRATO"); setupKnob(waveformKnob, "WAVEFORM");
    setupKnob(modThruKnob, "MOD THRU"); setupKnob(lfoRateKnob, "LFO RATE");

    // Create attachments
    attackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Attack", attackKnob.getSlider());
    decayAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Decay", decayKnob.getSlider());
    releaseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Release", releaseKnob.getSlider());
    coarseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Coarse", coarseKnob.getSlider());
    fineAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Fine", fineKnob.getSlider());
    modInitAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Mod Init", modInitKnob.getSlider());
    modDecAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Mod Dec", modDecKnob.getSlider());
    modSusAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Mod Sus", modSusKnob.getSlider());
    modRelAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Mod Rel", modRelKnob.getSlider());
    modVelAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Mod Vel", modVelKnob.getSlider());
    octaveAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Octave", octaveKnob.getSlider());
    fineTuneAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FineTune", fineTuneKnob.getSlider());
    vibratoAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Vibrato", vibratoKnob.getSlider());
    waveformAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Waveform", waveformKnob.getSlider());
    modThruAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Mod Thru", modThruKnob.getSlider());
    lfoRateAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "LFO Rate", lfoRateKnob.getSlider());

    // Build preset list
    rebuildPresetList();
    updatePresetSelectorFromParameter();
    
    presetSelector.onChange = [this]() {
        if (isUpdatingPresetSelector) return;
        int selectedId = presetSelector.getSelectedId();
        
        if (selectedId > 0 && selectedId <= numFactoryPresets) {
            // Factory preset
            audioProcessor.setCurrentProgram(selectedId - 1);
        }
        else if (selectedId > 1000) {
            // User preset - look up file from map
            auto it = presetIdToFile.find(selectedId);
            if (it != presetIdToFile.end() && it->second.existsAsFile()) {
                presetManager->loadPresetFromFile(it->second);
            }
        }
    };
    addAndMakeVisible(presetSelector);

    // Previous preset button
    prevPresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A35));
    prevPresetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00D4AA));
    prevPresetButton.onClick = [this]() { goToPreviousPreset(); };
    addAndMakeVisible(prevPresetButton);

    // Next preset button
    nextPresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A35));
    nextPresetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00D4AA));
    nextPresetButton.onClick = [this]() { goToNextPreset(); };
    addAndMakeVisible(nextPresetButton);

    // Settings button (gear icon)
    settingsButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A35));
    settingsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF888899));
    settingsButton.onClick = [this]() { showSettingsMenu(); };
    addAndMakeVisible(settingsButton);

    // Save button
    savePresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A35));
    savePresetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00D4AA));
    savePresetButton.onClick = [this]() { savePresetToFile(); };
    addAndMakeVisible(savePresetButton);

    // Load button
    loadPresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A35));
    loadPresetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00D4AA));
    loadPresetButton.onClick = [this]() { loadPresetFromFile(); };
    addAndMakeVisible(loadPresetButton);

    // Undo/Redo buttons
    undoButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A35));
    undoButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFCCCCCC));
    undoButton.onClick = [this]() { audioProcessor.undoManager.undo(); };
    addAndMakeVisible(undoButton);

    redoButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A35));
    redoButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFCCCCCC));
    redoButton.onClick = [this]() { audioProcessor.undoManager.redo(); };
    addAndMakeVisible(redoButton);

    audioProcessor.apvts.addParameterListener("PresetIndex", this);

    constrainer.setMinimumSize(700, 580);
    constrainer.setMaximumSize(1400, 1160);
    constrainer.setFixedAspectRatio(700.0 / 580.0);
    setConstrainer(&constrainer);
    setResizable(true, true);
    setSize(840, 696);
    
    // Try to load last preset on startup
    presetManager->loadLastPreset();
}

DX10AudioProcessorEditor::~DX10AudioProcessorEditor()
{
    audioProcessor.setSpectrumAnalyzer(nullptr);
    audioProcessor.apvts.removeParameterListener("PresetIndex", this);
    setLookAndFeel(nullptr);
}

void DX10AudioProcessorEditor::showSettingsMenu()
{
    juce::PopupMenu menu;
    
    menu.addItem(1, "Select Preset Folder...");
    menu.addItem(2, "Reset to Default Folder");
    menu.addSeparator();
    menu.addItem(3, "Open Preset Folder");
    menu.addSeparator();
    menu.addItem(4, "Refresh Preset List");
    
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&settingsButton),
        [this](int result)
        {
            switch (result)
            {
                case 1: selectPresetFolder(); break;
                case 2: 
                    presetManager->resetToDefaultDirectory();
                    rebuildPresetList();
                    break;
                case 3:
                    presetManager->getPresetDirectory().startAsProcess();
                    break;
                case 4:
                    rebuildPresetList();
                    break;
            }
        });
}

void DX10AudioProcessorEditor::selectPresetFolder()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select Preset Folder",
        presetManager->getPresetDirectory(),
        ""
    );
    
    chooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto folder = fc.getResult();
            if (folder.isDirectory())
            {
                presetManager->setPresetDirectory(folder);
                rebuildPresetList();
            }
        });
}

void DX10AudioProcessorEditor::rebuildPresetList()
{
    presetSelector.clear(juce::dontSendNotification);
    presetIdToFile.clear();
    
    // Add factory presets
    numFactoryPresets = audioProcessor.getNumPresets();
    for (int i = 0; i < numFactoryPresets; ++i)
        presetSelector.addItem(audioProcessor.getPresetName(i), i + 1);
    
    // Get user presets with folder structure
    userPresets = presetManager->getFlatPresetList();
    
    if (userPresets.size() > 0)
    {
        presetSelector.addSeparator();
        
        int nextId = 1001;
        for (const auto& item : userPresets)
        {
            // Create indented name for folders
            juce::String displayName;
            for (int d = 0; d < item.depth; ++d)
                displayName += "    "; // Indent
            
            if (item.isFolder)
            {
                displayName = displayName + juce::String("[") + item.displayName + juce::String("]");
                presetSelector.addSectionHeading(displayName);
            }
            else
            {
                displayName += item.displayName;
                presetSelector.addItem(displayName, nextId);
                presetIdToFile[nextId] = item.file;
                nextId++;
            }
        }
    }
}

void DX10AudioProcessorEditor::goToPreviousPreset()
{
    int currentId = presetSelector.getSelectedId();
    int totalItems = presetSelector.getNumItems();
    
    int currentIndex = -1;
    for (int i = 0; i < totalItems; ++i) {
        if (presetSelector.getItemId(i) == currentId) {
            currentIndex = i;
            break;
        }
    }
    
    if (currentIndex > 0) {
        for (int i = currentIndex - 1; i >= 0; --i) {
            int itemId = presetSelector.getItemId(i);
            if (itemId > 0 && presetSelector.isItemEnabled(i)) {
                presetSelector.setSelectedId(itemId);
                break;
            }
        }
    }
}

void DX10AudioProcessorEditor::goToNextPreset()
{
    int currentId = presetSelector.getSelectedId();
    int totalItems = presetSelector.getNumItems();
    
    int currentIndex = -1;
    for (int i = 0; i < totalItems; ++i) {
        if (presetSelector.getItemId(i) == currentId) {
            currentIndex = i;
            break;
        }
    }
    
    if (currentIndex >= 0 && currentIndex < totalItems - 1) {
        for (int i = currentIndex + 1; i < totalItems; ++i) {
            int itemId = presetSelector.getItemId(i);
            if (itemId > 0 && presetSelector.isItemEnabled(i)) {
                presetSelector.setSelectedId(itemId);
                break;
            }
        }
    }
}

void DX10AudioProcessorEditor::savePresetToFile()
{
    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Save Preset",
        presetManager->getPresetDirectory(),
        "*" + PresetManager::getPresetExtension()
    );

    fileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, fileChooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File{}) {
                if (!file.hasFileExtension(PresetManager::getPresetExtension()))
                    file = file.withFileExtension(PresetManager::getPresetExtension());
                
                if (presetManager->savePresetToFile(file)) {
                    rebuildPresetList();
                    // Select the saved preset
                    for (const auto& pair : presetIdToFile) {
                        if (pair.second == file) {
                            isUpdatingPresetSelector = true;
                            presetSelector.setSelectedId(pair.first, juce::dontSendNotification);
                            isUpdatingPresetSelector = false;
                            break;
                        }
                    }
                }
            }
        }
    );
}

void DX10AudioProcessorEditor::loadPresetFromFile()
{
    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Load Preset",
        presetManager->getPresetDirectory(),
        "*" + PresetManager::getPresetExtension()
    );

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, fileChooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile()) {
                if (presetManager->loadPresetFromFile(file)) {
                    rebuildPresetList();
                    // Select the loaded preset if it's in the list
                    for (const auto& pair : presetIdToFile) {
                        if (pair.second == file) {
                            isUpdatingPresetSelector = true;
                            presetSelector.setSelectedId(pair.first, juce::dontSendNotification);
                            isUpdatingPresetSelector = false;
                            break;
                        }
                    }
                }
            }
        }
    );
}

void DX10AudioProcessorEditor::parameterChanged(const juce::String& parameterID, float)
{
    if (parameterID == "PresetIndex")
        juce::MessageManager::callAsync([this]() { updatePresetSelectorFromParameter(); });
}

void DX10AudioProcessorEditor::updatePresetSelectorFromParameter()
{
    isUpdatingPresetSelector = true;
    if (auto* param = audioProcessor.apvts.getRawParameterValue("PresetIndex")) {
        int index = static_cast<int>(param->load() * (NPRESETS - 1) + 0.5f);
        if (index >= 0 && index < numFactoryPresets)
            presetSelector.setSelectedId(index + 1, juce::dontSendNotification);
    }
    isUpdatingPresetSelector = false;
}

void DX10AudioProcessorEditor::setupKnob(RotaryKnobWithLabel& knob, const juce::String& labelText)
{
    knob.setLabelText(labelText);
    addAndMakeVisible(knob);
}

// Drag and drop support
bool DX10AudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& file : files)
        if (file.endsWith(PresetManager::getPresetExtension()))
            return true;
    return false;
}

void DX10AudioProcessorEditor::fileDragEnter(const juce::StringArray&, int, int)
{
    isDragOver = true;
    repaint();
}

void DX10AudioProcessorEditor::fileDragExit(const juce::StringArray&)
{
    isDragOver = false;
    repaint();
}

void DX10AudioProcessorEditor::filesDropped(const juce::StringArray& files, int, int)
{
    isDragOver = false;
    repaint();
    
    for (const auto& filePath : files) {
        juce::File file(filePath);
        if (file.hasFileExtension(PresetManager::getPresetExtension())) {
            if (presetManager->loadPresetFromFile(file)) {
                rebuildPresetList();
                break;
            }
        }
    }
}

void DX10AudioProcessorEditor::drawSection(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title)
{
    juce::ColourGradient sectionGrad(juce::Colour(0xFF1A1A22), float(bounds.getX()), float(bounds.getY()),
                                      juce::Colour(0xFF15151D), float(bounds.getX()), float(bounds.getBottom()), false);
    g.setGradientFill(sectionGrad);
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xFF2A2A35));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 8.0f, 1.0f);
    g.setFont(DX10LookAndFeel::getSectionFont());
    g.setColour(juce::Colour(0xFF00D4AA));
    g.drawText(title, bounds.getX() + 12, bounds.getY() + 8, bounds.getWidth() - 24, 16, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xFF00D4AA).withAlpha(0.3f));
    g.fillRect(bounds.getX() + 12, bounds.getY() + 26, bounds.getWidth() - 24, 1);
}

void DX10AudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    float width = float(bounds.getWidth());
    float height = float(bounds.getHeight());

    juce::ColourGradient bgGradient(juce::Colour(0xFF12121A), 0.0f, 0.0f, juce::Colour(0xFF0A0A10), width, height, true);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // Grid overlay
    g.setColour(juce::Colour(0xFF1A1A22).withAlpha(0.5f));
    float gridSize = 20.0f * (width / 840.0f);
    for (float x = 0.0f; x < width; x += gridSize) g.drawLine(x, 0.0f, x, height, 0.5f);
    for (float y = 0.0f; y < height; y += gridSize) g.drawLine(0.0f, y, width, y, 0.5f);

    // Drag overlay
    if (isDragOver) {
        g.setColour(juce::Colour(0xFF00D4AA).withAlpha(0.15f));
        g.fillAll();
        g.setColour(juce::Colour(0xFF00D4AA));
        g.drawRect(bounds, 3);
        g.setFont(24.0f);
        g.drawText("Drop Preset File Here", bounds, juce::Justification::centred);
    }

    float scale = width / 840.0f;
    int margin = int(16.0f * scale);
    int headerHeight = int(70.0f * scale);
    int sectionGap = int(12.0f * scale);

    auto headerBounds = bounds.removeFromTop(headerHeight);
    
    // Title - DX10
    g.setFont(DX10LookAndFeel::getTitleFont().withHeight(28.0f * scale));
    g.setColour(juce::Colour(0xFFFFFFFF));
    g.drawText("DX10", margin, headerBounds.getY() + int(8.0f * scale), int(80.0f * scale), int(30.0f * scale), juce::Justification::centredLeft);
    
    // Subtitle - FM SYNTHESIZER
    g.setFont(juce::Font(juce::FontOptions(9.0f * scale)));
    g.setColour(juce::Colour(0xFF666677));
    g.drawText("FM SYNTHESIZER", margin, headerBounds.getY() + int(36.0f * scale), int(100.0f * scale), int(14.0f * scale), juce::Justification::centredLeft);
    
    // Accent line
    g.setColour(juce::Colour(0xFF00D4AA));
    g.fillRect(margin, headerHeight - 2, int(width) - margin * 2, 2);

    auto contentBounds = bounds.reduced(margin, margin / 2);
    int sectionWidth = (contentBounds.getWidth() - sectionGap * 2) / 3;
    int topRowHeight = int(140.0f * scale);
    int spectrumHeight = int(80.0f * scale);
    int bottomRowHeight = contentBounds.getHeight() - topRowHeight - spectrumHeight - sectionGap * 2;

    // Draw sections
    drawSection(g, {contentBounds.getX(), contentBounds.getY(), sectionWidth, topRowHeight}, "CARRIER ENVELOPE");
    drawSection(g, {contentBounds.getX() + sectionWidth + sectionGap, contentBounds.getY(), sectionWidth, topRowHeight}, "MODULATOR RATIO");
    drawSection(g, {contentBounds.getX() + (sectionWidth + sectionGap) * 2, contentBounds.getY(), sectionWidth, topRowHeight}, "TUNING");
    drawSection(g, {contentBounds.getX(), contentBounds.getY() + topRowHeight + sectionGap, sectionWidth * 2 + sectionGap, bottomRowHeight}, "MODULATOR ENVELOPE");
    drawSection(g, {contentBounds.getX() + (sectionWidth + sectionGap) * 2, contentBounds.getY() + topRowHeight + sectionGap, sectionWidth, bottomRowHeight}, "OUTPUT / LFO");
}

void DX10AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    float scale = float(bounds.getWidth()) / 840.0f;
    
    int margin = int(16.0f * scale);
    int headerHeight = int(70.0f * scale);
    int sectionGap = int(12.0f * scale);
    int knobSize = int(80.0f * scale);
    int sectionPadding = int(35.0f * scale);
    int buttonHeight = int(24.0f * scale);
    int smallButtonWidth = int(26.0f * scale);
    int buttonWidth = int(42.0f * scale);

    auto headerBounds = bounds.removeFromTop(headerHeight);
    int headerY = headerBounds.getY() + int(22.0f * scale);
    int rightEdge = bounds.getWidth() - margin;
    
    // Right side buttons (from right to left): Redo, Undo, |gap|, Load, Save, |gap|, Settings
    redoButton.setBounds(rightEdge - buttonWidth, headerY, buttonWidth, buttonHeight);
    undoButton.setBounds(redoButton.getX() - buttonWidth - 4, headerY, buttonWidth, buttonHeight);
    
    loadPresetButton.setBounds(undoButton.getX() - buttonWidth - int(12.0f * scale), headerY, buttonWidth, buttonHeight);
    savePresetButton.setBounds(loadPresetButton.getX() - buttonWidth - 4, headerY, buttonWidth, buttonHeight);
    
    settingsButton.setBounds(savePresetButton.getX() - smallButtonWidth - int(12.0f * scale), headerY, smallButtonWidth, buttonHeight);
    
    // Left side: Preset navigation
    int presetAreaX = margin + int(110.0f * scale);
    prevPresetButton.setBounds(presetAreaX, headerY, smallButtonWidth, buttonHeight);
    
    int presetSelectorWidth = settingsButton.getX() - prevPresetButton.getRight() - smallButtonWidth - int(16.0f * scale);
    presetSelector.setBounds(prevPresetButton.getRight() + 2, headerY, presetSelectorWidth, buttonHeight);
    nextPresetButton.setBounds(presetSelector.getRight() + 2, headerY, smallButtonWidth, buttonHeight);

    auto contentBounds = bounds.reduced(margin, margin / 2);
    int sectionWidth = (contentBounds.getWidth() - sectionGap * 2) / 3;
    int topRowHeight = int(140.0f * scale);
    int spectrumHeight = int(80.0f * scale);
    int bottomRowHeight = contentBounds.getHeight() - topRowHeight - spectrumHeight - sectionGap * 2;

    // Spectrum analyzer at bottom
    spectrumAnalyzer.setBounds(contentBounds.getX(), contentBounds.getBottom() - spectrumHeight, contentBounds.getWidth(), spectrumHeight);

    // Carrier Envelope
    auto carrierBounds = juce::Rectangle<int>(contentBounds.getX(), contentBounds.getY(), sectionWidth, topRowHeight);
    auto knobArea = carrierBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    int knobSpacing = knobArea.getWidth() / 3;
    attackKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    decayKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    releaseKnob.setBounds(knobArea.getX() + knobSpacing*2 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());

    // Modulator Ratio
    auto ratioBounds = juce::Rectangle<int>(contentBounds.getX() + sectionWidth + sectionGap, contentBounds.getY(), sectionWidth, topRowHeight);
    knobArea = ratioBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    knobSpacing = knobArea.getWidth() / 2;
    coarseKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    fineKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());

    // Tuning
    auto tuningBounds = juce::Rectangle<int>(contentBounds.getX() + (sectionWidth + sectionGap)*2, contentBounds.getY(), sectionWidth, topRowHeight);
    knobArea = tuningBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    knobSpacing = knobArea.getWidth() / 2;
    octaveKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    fineTuneKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());

    // Modulator Envelope
    auto modEnvBounds = juce::Rectangle<int>(contentBounds.getX(), contentBounds.getY() + topRowHeight + sectionGap, sectionWidth*2 + sectionGap, bottomRowHeight);
    knobArea = modEnvBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    knobSpacing = knobArea.getWidth() / 5;
    modInitKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    modDecKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    modSusKnob.setBounds(knobArea.getX() + knobSpacing*2 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    modRelKnob.setBounds(knobArea.getX() + knobSpacing*3 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    modVelKnob.setBounds(knobArea.getX() + knobSpacing*4 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());

    // Output / LFO
    auto outputBounds = juce::Rectangle<int>(contentBounds.getX() + (sectionWidth + sectionGap)*2, contentBounds.getY() + topRowHeight + sectionGap, sectionWidth, bottomRowHeight);
    knobArea = outputBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    knobSpacing = knobArea.getWidth() / 2;
    int knobRowHeight = knobArea.getHeight() / 2;
    vibratoKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobRowHeight);
    lfoRateKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobRowHeight);
    waveformKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY() + knobRowHeight, knobSize, knobRowHeight);
    modThruKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY() + knobRowHeight, knobSize, knobRowHeight);
}
