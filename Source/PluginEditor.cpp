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
    setupKnob(glideKnob, "GLIDE"); setupKnob(gainKnob, "GAIN"); setupKnob(saturationKnob, "SATURATE");

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
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Gain", gainKnob.getSlider());
    saturationAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Saturation", saturationKnob.getSlider());
    glideAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "Glide", glideKnob.getSlider());

    // Set associated parameters for host context menu (DAW automation)
    attackKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Attack"));
    decayKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Decay"));
    releaseKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Release"));
    coarseKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Coarse"));
    fineKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Fine"));
    modInitKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Mod Init"));
    modDecKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Mod Dec"));
    modSusKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Mod Sus"));
    modRelKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Mod Rel"));
    modVelKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Mod Vel"));
    octaveKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Octave"));
    fineTuneKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("FineTune"));
    vibratoKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Vibrato"));
    waveformKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Waveform"));
    modThruKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Mod Thru"));
    lfoRateKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("LFO Rate"));
    gainKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Gain"));
    saturationKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Saturation"));
    glideKnob.getSlider().setAssociatedParameter(audioProcessor.apvts.getParameter("Glide"));

    // Build preset list
    rebuildPresetList();
    updatePresetSelectorFromParameter();
    
    presetSelector.onChange = [this]() {
        if (isUpdatingPresetSelector) return;
        int selectedId = presetSelector.getSelectedId();
        
        // Store the selected preset ID in the parameter (for undo tracking)
        if (auto* param = audioProcessor.apvts.getParameter("SelectedPresetId"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(selectedId)));
        
        if (selectedId > 0 && selectedId <= numFactoryPresets) {
            // Factory preset
            lastLoadedUserPreset = juce::File();  // Clear user preset tracking
            audioProcessor.setCurrentProgram(selectedId - 1);
        }
        else if (selectedId > 1000) {
            // User preset - look up file from map
            auto it = presetIdToFile.find(selectedId);
            if (it != presetIdToFile.end() && it->second.existsAsFile()) {
                lastLoadedUserPreset = it->second;  // Track which preset was loaded
                // Set the preset name for host display
                audioProcessor.setCurrentPresetName(it->second.getFileNameWithoutExtension());
                presetManager->loadPresetFromFile(it->second);
                // Notify host of program change
                audioProcessor.updateHostDisplay(juce::AudioProcessor::ChangeDetails().withProgramChanged(true));
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

    // Spectrum analyzer always visible
    addAndMakeVisible(spectrumAnalyzer);

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
    undoButton.onClick = [this]() { 
        audioProcessor.undoManager.undo();
        // Restore preset selector from the SelectedPresetId parameter
        if (auto* param = audioProcessor.apvts.getRawParameterValue("SelectedPresetId")) {
            int presetId = static_cast<int>(param->load());
            isUpdatingPresetSelector = true;
            presetSelector.setSelectedId(presetId, juce::dontSendNotification);
            // Update lastLoadedUserPreset if it's a user preset
            if (presetId > 1000) {
                auto it = presetIdToFile.find(presetId);
                if (it != presetIdToFile.end())
                    lastLoadedUserPreset = it->second;
            } else {
                lastLoadedUserPreset = juce::File();
            }
            isUpdatingPresetSelector = false;
        }
    };
    addAndMakeVisible(undoButton);

    redoButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A35));
    redoButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFCCCCCC));
    redoButton.onClick = [this]() { 
        audioProcessor.undoManager.redo();
        // Restore preset selector from the SelectedPresetId parameter
        if (auto* param = audioProcessor.apvts.getRawParameterValue("SelectedPresetId")) {
            int presetId = static_cast<int>(param->load());
            isUpdatingPresetSelector = true;
            presetSelector.setSelectedId(presetId, juce::dontSendNotification);
            // Update lastLoadedUserPreset if it's a user preset
            if (presetId > 1000) {
                auto it = presetIdToFile.find(presetId);
                if (it != presetIdToFile.end())
                    lastLoadedUserPreset = it->second;
            } else {
                lastLoadedUserPreset = juce::File();
            }
            isUpdatingPresetSelector = false;
        }
    };
    addAndMakeVisible(redoButton);

    audioProcessor.apvts.addParameterListener("SelectedPresetId", this);

    setConstrainer(&constrainer);
    constrainer.setFixedAspectRatio(750.0 / 600.0);
    constrainer.setMinimumSize(750, 600);
    constrainer.setMaximumSize(1500, 1200);
    setResizable(true, true);
    setSize(750, 600);
}

DX10AudioProcessorEditor::~DX10AudioProcessorEditor()
{
    audioProcessor.setSpectrumAnalyzer(nullptr);
    audioProcessor.apvts.removeParameterListener("SelectedPresetId", this);
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
    fileToPresetId.clear();
    
    // Add factory presets (IDs 1-32)
    numFactoryPresets = audioProcessor.getNumPresets();
    for (int i = 0; i < numFactoryPresets; ++i)
        presetSelector.addItem(audioProcessor.getPresetName(i), i + 1);
    
    // Get user presets with folder structure
    userPresets = presetManager->getFlatPresetList();
    
    if (userPresets.size() > 0)
    {
        presetSelector.addSeparator();
        
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
                // Generate unique ID from file path hash (1001 - 999999 range)
                int presetId = generatePresetIdFromFile(item.file);
                presetSelector.addItem(displayName, presetId);
                presetIdToFile[presetId] = item.file;
                fileToPresetId[item.file.getFullPathName()] = presetId;
            }
        }
    }
}

int DX10AudioProcessorEditor::generatePresetIdFromFile(const juce::File& file)
{
    // Generate a hash from the full file path
    juce::String path = file.getFullPathName();
    size_t hash = std::hash<std::string>{}(path.toStdString());
    
    // Map to range 1001 - 999999 (leaving room for factory presets 1-1000)
    int baseId = 1001 + static_cast<int>(hash % 998999);
    
    // Handle collisions by incrementing
    while (presetIdToFile.find(baseId) != presetIdToFile.end())
        baseId++;
    
    return baseId;
}

void DX10AudioProcessorEditor::goToPreviousPreset()
{
    int currentId = presetSelector.getSelectedId();
    
    // Build list of all valid preset IDs (factory: 1-32, user: 1001+)
    std::vector<int> validIds;
    
    // Add factory preset IDs
    for (int i = 0; i < numFactoryPresets; ++i)
        validIds.push_back(i + 1);
    
    // Add user preset IDs from the map
    for (const auto& pair : presetIdToFile)
        validIds.push_back(pair.first);
    
    // Sort to ensure order
    std::sort(validIds.begin(), validIds.end());
    
    if (validIds.empty()) return;
    
    // Find current position
    auto it = std::find(validIds.begin(), validIds.end(), currentId);
    
    int newId;
    if (it == validIds.end() || it == validIds.begin()) {
        // Not found or at beginning - wrap to end
        newId = validIds.back();
    } else {
        // Go to previous
        newId = *(--it);
    }
    
    presetSelector.setSelectedId(newId);
}

void DX10AudioProcessorEditor::goToNextPreset()
{
    int currentId = presetSelector.getSelectedId();
    
    // Build list of all valid preset IDs (factory: 1-32, user: 1001+)
    std::vector<int> validIds;
    
    // Add factory preset IDs
    for (int i = 0; i < numFactoryPresets; ++i)
        validIds.push_back(i + 1);
    
    // Add user preset IDs from the map
    for (const auto& pair : presetIdToFile)
        validIds.push_back(pair.first);
    
    // Sort to ensure order
    std::sort(validIds.begin(), validIds.end());
    
    if (validIds.empty()) return;
    
    // Find current position
    auto it = std::find(validIds.begin(), validIds.end(), currentId);
    
    int newId;
    if (it == validIds.end()) {
        // Not found - go to first
        newId = validIds.front();
    } else {
        ++it;
        if (it == validIds.end()) {
            // At end - wrap to beginning
            newId = validIds.front();
        } else {
            newId = *it;
        }
    }
    
    presetSelector.setSelectedId(newId);
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
                    lastLoadedUserPreset = file;
                    
                    // Set the preset name for host display
                    audioProcessor.setCurrentPresetName(file.getFileNameWithoutExtension());
                    
                    rebuildPresetList();
                    
                    // Look up the preset ID using the file path
                    auto it = fileToPresetId.find(file.getFullPathName());
                    if (it != fileToPresetId.end()) {
                        int presetId = it->second;
                        isUpdatingPresetSelector = true;
                        presetSelector.setSelectedId(presetId, juce::dontSendNotification);
                        if (auto* param = audioProcessor.apvts.getParameter("SelectedPresetId"))
                            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(presetId)));
                        isUpdatingPresetSelector = false;
                    }
                    
                    // Notify host of program change
                    audioProcessor.updateHostDisplay(juce::AudioProcessor::ChangeDetails().withProgramChanged(true));
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
                    lastLoadedUserPreset = file;
                    
                    // Set the preset name for host display
                    audioProcessor.setCurrentPresetName(file.getFileNameWithoutExtension());
                    
                    rebuildPresetList();
                    
                    // Look up the preset ID using the file path
                    auto it = fileToPresetId.find(file.getFullPathName());
                    if (it != fileToPresetId.end()) {
                        int presetId = it->second;
                        isUpdatingPresetSelector = true;
                        presetSelector.setSelectedId(presetId, juce::dontSendNotification);
                        if (auto* param = audioProcessor.apvts.getParameter("SelectedPresetId"))
                            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(presetId)));
                        isUpdatingPresetSelector = false;
                    }
                    
                    // Notify host of program change
                    audioProcessor.updateHostDisplay(juce::AudioProcessor::ChangeDetails().withProgramChanged(true));
                }
            }
        }
    );
}

void DX10AudioProcessorEditor::parameterChanged(const juce::String& parameterID, float)
{
    // Only update selector when SelectedPresetId changes, not PresetIndex
    if (parameterID == "SelectedPresetId")
        juce::MessageManager::callAsync([this]() { updatePresetSelectorFromParameter(); });
}

void DX10AudioProcessorEditor::updatePresetSelectorFromParameter()
{
    isUpdatingPresetSelector = true;
    if (auto* param = audioProcessor.apvts.getRawParameterValue("SelectedPresetId")) {
        int presetId = static_cast<int>(param->load());
        if (presetId > 0) {
            presetSelector.setSelectedId(presetId, juce::dontSendNotification);
            // Update lastLoadedUserPreset if it's a user preset
            if (presetId > 1000) {
                auto it = presetIdToFile.find(presetId);
                if (it != presetIdToFile.end())
                    lastLoadedUserPreset = it->second;
            } else {
                lastLoadedUserPreset = juce::File();
            }
        }
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
                lastLoadedUserPreset = file;
                
                // Set the preset name for host display
                audioProcessor.setCurrentPresetName(file.getFileNameWithoutExtension());
                
                rebuildPresetList();
                
                // Look up the preset ID and update selector
                auto it = fileToPresetId.find(file.getFullPathName());
                if (it != fileToPresetId.end()) {
                    int presetId = it->second;
                    isUpdatingPresetSelector = true;
                    presetSelector.setSelectedId(presetId, juce::dontSendNotification);
                    if (auto* param = audioProcessor.apvts.getParameter("SelectedPresetId"))
                        param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(presetId)));
                    isUpdatingPresetSelector = false;
                }
                
                // Notify host of program change
                audioProcessor.updateHostDisplay(juce::AudioProcessor::ChangeDetails().withProgramChanged(true));
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
    int topRowHeight = int(130.0f * scale);
    int midRowHeight = int(130.0f * scale);
    int bottomRowHeight = int(130.0f * scale);

    // Draw sections - Row 1
    drawSection(g, {contentBounds.getX(), contentBounds.getY(), sectionWidth, topRowHeight}, "CARRIER ENVELOPE");
    drawSection(g, {contentBounds.getX() + sectionWidth + sectionGap, contentBounds.getY(), sectionWidth, topRowHeight}, "MODULATOR RATIO");
    drawSection(g, {contentBounds.getX() + (sectionWidth + sectionGap) * 2, contentBounds.getY(), sectionWidth, topRowHeight}, "TUNING");
    
    // Row 2 - Modulator Envelope (full width)
    drawSection(g, {contentBounds.getX(), contentBounds.getY() + topRowHeight + sectionGap, contentBounds.getWidth(), midRowHeight}, "MODULATOR ENVELOPE");
    
    // Row 3 - Output / LFO (full width)
    drawSection(g, {contentBounds.getX(), contentBounds.getY() + topRowHeight + midRowHeight + sectionGap * 2, contentBounds.getWidth(), bottomRowHeight}, "OUTPUT / LFO");
}

void DX10AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    float scale = float(bounds.getWidth()) / 840.0f;
    
    int margin = int(16.0f * scale);
    int headerHeight = int(70.0f * scale);
    int sectionGap = int(12.0f * scale);
    int knobSize = int(90.0f * scale);  // Bigger knobs
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

    // Spectrum toggle button (bottom left, just above where spectrum would be)
    int spectrumButtonWidth = int(70.0f * scale);
    // Position will be set after contentBounds is calculated

    auto contentBounds = bounds.reduced(margin, margin / 2);
    int sectionWidth = (contentBounds.getWidth() - sectionGap * 2) / 3;
    int topRowHeight = int(130.0f * scale);
    int midRowHeight = int(130.0f * scale);
    int bottomRowHeight = int(130.0f * scale);
    int spectrumHeight = int(100.0f * scale);

    // Spectrum analyzer at bottom
    int spectrumY = contentBounds.getY() + topRowHeight + midRowHeight + bottomRowHeight + sectionGap * 3;
    spectrumAnalyzer.setBounds(contentBounds.getX(), spectrumY, contentBounds.getWidth(), contentBounds.getBottom() - spectrumY);

    // Row 1: Carrier Envelope
    auto carrierBounds = juce::Rectangle<int>(contentBounds.getX(), contentBounds.getY(), sectionWidth, topRowHeight);
    auto knobArea = carrierBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    int knobSpacing = knobArea.getWidth() / 3;
    attackKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    decayKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    releaseKnob.setBounds(knobArea.getX() + knobSpacing*2 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());

    // Row 1: Modulator Ratio
    auto ratioBounds = juce::Rectangle<int>(contentBounds.getX() + sectionWidth + sectionGap, contentBounds.getY(), sectionWidth, topRowHeight);
    knobArea = ratioBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    knobSpacing = knobArea.getWidth() / 2;
    coarseKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    fineKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());

    // Row 1: Tuning
    auto tuningBounds = juce::Rectangle<int>(contentBounds.getX() + (sectionWidth + sectionGap)*2, contentBounds.getY(), sectionWidth, topRowHeight);
    knobArea = tuningBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    knobSpacing = knobArea.getWidth() / 2;
    octaveKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    fineTuneKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());

    // Row 2: Modulator Envelope (full width, 5 knobs)
    auto modEnvBounds = juce::Rectangle<int>(contentBounds.getX(), contentBounds.getY() + topRowHeight + sectionGap, contentBounds.getWidth(), midRowHeight);
    knobArea = modEnvBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    knobSpacing = knobArea.getWidth() / 5;
    modInitKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    modDecKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    modSusKnob.setBounds(knobArea.getX() + knobSpacing*2 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    modRelKnob.setBounds(knobArea.getX() + knobSpacing*3 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    modVelKnob.setBounds(knobArea.getX() + knobSpacing*4 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());

    // Row 3: Output / LFO (full width, 7 knobs)
    auto outputBounds = juce::Rectangle<int>(contentBounds.getX(), contentBounds.getY() + topRowHeight + midRowHeight + sectionGap * 2, contentBounds.getWidth(), bottomRowHeight);
    knobArea = outputBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
    knobSpacing = knobArea.getWidth() / 7;
    vibratoKnob.setBounds(knobArea.getX() + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    lfoRateKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    waveformKnob.setBounds(knobArea.getX() + knobSpacing*2 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    modThruKnob.setBounds(knobArea.getX() + knobSpacing*3 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    glideKnob.setBounds(knobArea.getX() + knobSpacing*4 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    gainKnob.setBounds(knobArea.getX() + knobSpacing*5 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
    saturationKnob.setBounds(knobArea.getX() + knobSpacing*6 + knobSpacing/2 - knobSize/2, knobArea.getY(), knobSize, knobArea.getHeight());
}
