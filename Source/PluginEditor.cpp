#include "PluginEditor.h"

DX10AudioProcessorEditor::DX10AudioProcessorEditor(DX10AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Apply custom look and feel
    setLookAndFeel(&customLookAndFeel);

    // === Setup all knobs first ===
    setupKnob(attackKnob, "ATTACK");
    setupKnob(decayKnob, "DECAY");
    setupKnob(releaseKnob, "RELEASE");
    setupKnob(coarseKnob, "COARSE");
    setupKnob(fineKnob, "FINE");
    setupKnob(modInitKnob, "INIT");
    setupKnob(modDecKnob, "DECAY");
    setupKnob(modSusKnob, "SUSTAIN");
    setupKnob(modRelKnob, "RELEASE");
    setupKnob(modVelKnob, "VEL SENS");
    setupKnob(octaveKnob, "OCTAVE");
    setupKnob(fineTuneKnob, "FINE TUNE");
    setupKnob(vibratoKnob, "VIBRATO");
    setupKnob(waveformKnob, "WAVEFORM");
    setupKnob(modThruKnob, "MOD THRU");
    setupKnob(lfoRateKnob, "LFO RATE");

    // === Create Parameter Attachments ===
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

    // === Setup preset selector ===
    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF888899));
    presetLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(presetLabel);

    // Populate preset selector
    for (int i = 0; i < audioProcessor.getNumPresets(); ++i)
    {
        presetSelector.addItem(audioProcessor.getPresetName(i), i + 1);
    }
    
    // Set initial selection from the parameter
    updatePresetSelectorFromParameter();
    
    // Handle user selection changes
    presetSelector.onChange = [this]()
    {
        if (isUpdatingPresetSelector)
            return;
            
        int selectedIndex = presetSelector.getSelectedId() - 1;
        if (selectedIndex >= 0)
        {
            audioProcessor.setCurrentProgram(selectedIndex);
        }
    };
    addAndMakeVisible(presetSelector);

    // Listen for preset parameter changes (for when state is restored)
    audioProcessor.apvts.addParameterListener("PresetIndex", this);

    // Set size constraints for resizing
    constrainer.setMinimumSize(600, 450);
    constrainer.setMaximumSize(1200, 900);
    constrainer.setFixedAspectRatio(600.0 / 450.0);
    setConstrainer(&constrainer);

    // Make the plugin resizable
    setResizable(true, true);
    setSize(720, 540);
}

DX10AudioProcessorEditor::~DX10AudioProcessorEditor()
{
    audioProcessor.apvts.removeParameterListener("PresetIndex", this);
    setLookAndFeel(nullptr);
}

void DX10AudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "PresetIndex")
    {
        // Update the combo box on the message thread
        juce::MessageManager::callAsync([this]()
        {
            updatePresetSelectorFromParameter();
        });
    }
}

void DX10AudioProcessorEditor::updatePresetSelectorFromParameter()
{
    isUpdatingPresetSelector = true;
    
    if (auto* param = audioProcessor.apvts.getRawParameterValue("PresetIndex"))
    {
        int index = static_cast<int>(param->load() * (NPRESETS - 1) + 0.5f);
        presetSelector.setSelectedId(index + 1, juce::dontSendNotification);
    }
    
    isUpdatingPresetSelector = false;
}

void DX10AudioProcessorEditor::setupKnob(RotaryKnobWithLabel& knob, const juce::String& labelText)
{
    knob.setLabelText(labelText);
    addAndMakeVisible(knob);
}

void DX10AudioProcessorEditor::drawSection(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title)
{
    // Section background with subtle gradient
    juce::ColourGradient sectionGrad(juce::Colour(0xFF1A1A22), 
                                      static_cast<float>(bounds.getX()), 
                                      static_cast<float>(bounds.getY()),
                                      juce::Colour(0xFF15151D), 
                                      static_cast<float>(bounds.getX()), 
                                      static_cast<float>(bounds.getBottom()), 
                                      false);
    g.setGradientFill(sectionGrad);
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);

    // Border
    g.setColour(juce::Colour(0xFF2A2A35));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 8.0f, 1.0f);

    // Section title
    g.setFont(DX10LookAndFeel::getSectionFont());
    g.setColour(juce::Colour(0xFF00D4AA));
    g.drawText(title, bounds.getX() + 12, bounds.getY() + 8, bounds.getWidth() - 24, 16,
               juce::Justification::centredLeft);

    // Accent line under title
    g.setColour(juce::Colour(0xFF00D4AA).withAlpha(0.3f));
    g.fillRect(bounds.getX() + 12, bounds.getY() + 26, bounds.getWidth() - 24, 1);
}

void DX10AudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    float width = static_cast<float>(bounds.getWidth());
    float height = static_cast<float>(bounds.getHeight());

    // Main background gradient
    juce::ColourGradient bgGradient(juce::Colour(0xFF12121A), 0.0f, 0.0f,
                                     juce::Colour(0xFF0A0A10), width, height, true);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // Subtle grid pattern overlay
    g.setColour(juce::Colour(0xFF1A1A22).withAlpha(0.5f));
    float gridSize = 20.0f * (width / 720.0f);
    for (float x = 0.0f; x < width; x += gridSize)
    {
        g.drawLine(x, 0.0f, x, height, 0.5f);
    }
    for (float y = 0.0f; y < height; y += gridSize)
    {
        g.drawLine(0.0f, y, width, y, 0.5f);
    }

    // Calculate scaled dimensions
    float scale = width / 720.0f;
    int margin = static_cast<int>(16.0f * scale);
    int headerHeight = static_cast<int>(70.0f * scale);
    int sectionGap = static_cast<int>(12.0f * scale);

    // Header area
    auto headerBounds = bounds.removeFromTop(headerHeight);
    
    // Logo/Title
    g.setFont(DX10LookAndFeel::getTitleFont().withHeight(28.0f * scale));
    g.setColour(juce::Colour(0xFFFFFFFF));
    g.drawText("DX10", headerBounds.reduced(margin, 0), juce::Justification::centredLeft);

    // Subtitle
    g.setFont(juce::Font(juce::FontOptions(11.0f * scale)));
    g.setColour(juce::Colour(0xFF666677));
    g.drawText("FM SYNTHESIZER", 
               headerBounds.getX() + margin, 
               headerBounds.getY() + static_cast<int>(32.0f * scale),
               static_cast<int>(150.0f * scale), 
               static_cast<int>(20.0f * scale),
               juce::Justification::centredLeft);

    // Accent line
    g.setColour(juce::Colour(0xFF00D4AA));
    g.fillRect(margin, headerHeight - 2, static_cast<int>(width) - margin * 2, 2);

    // Content area for sections
    auto contentBounds = bounds.reduced(margin, margin / 2);
    
    int sectionWidth = (contentBounds.getWidth() - sectionGap * 2) / 3;
    int topRowHeight = static_cast<int>(160.0f * scale);
    int bottomRowHeight = contentBounds.getHeight() - topRowHeight - sectionGap;

    // === Draw Sections ===
    
    // Carrier Envelope Section
    auto carrierBounds = juce::Rectangle<int>(
        contentBounds.getX(),
        contentBounds.getY(),
        sectionWidth,
        topRowHeight
    );
    drawSection(g, carrierBounds, "CARRIER ENVELOPE");

    // Modulator Ratio Section
    auto ratioBounds = juce::Rectangle<int>(
        contentBounds.getX() + sectionWidth + sectionGap,
        contentBounds.getY(),
        sectionWidth,
        topRowHeight
    );
    drawSection(g, ratioBounds, "MODULATOR RATIO");

    // Tuning Section
    auto tuningBounds = juce::Rectangle<int>(
        contentBounds.getX() + (sectionWidth + sectionGap) * 2,
        contentBounds.getY(),
        sectionWidth,
        topRowHeight
    );
    drawSection(g, tuningBounds, "TUNING");

    // Modulator Envelope Section (bottom left, wider)
    auto modEnvBounds = juce::Rectangle<int>(
        contentBounds.getX(),
        contentBounds.getY() + topRowHeight + sectionGap,
        sectionWidth * 2 + sectionGap,
        bottomRowHeight
    );
    drawSection(g, modEnvBounds, "MODULATOR ENVELOPE");

    // Output Section
    auto outputBounds = juce::Rectangle<int>(
        contentBounds.getX() + (sectionWidth + sectionGap) * 2,
        contentBounds.getY() + topRowHeight + sectionGap,
        sectionWidth,
        bottomRowHeight
    );
    drawSection(g, outputBounds, "OUTPUT / LFO");
}

void DX10AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    float scale = static_cast<float>(bounds.getWidth()) / 720.0f;
    
    int margin = static_cast<int>(16.0f * scale);
    int headerHeight = static_cast<int>(70.0f * scale);
    int sectionGap = static_cast<int>(12.0f * scale);
    int knobSize = static_cast<int>(85.0f * scale);
    int sectionPadding = static_cast<int>(35.0f * scale); // Space for section title

    // Preset selector in header
    auto headerBounds = bounds.removeFromTop(headerHeight);
    presetLabel.setBounds(
        headerBounds.getRight() - margin - static_cast<int>(250.0f * scale),
        headerBounds.getY() + margin,
        static_cast<int>(50.0f * scale),
        static_cast<int>(24.0f * scale)
    );
    presetSelector.setBounds(
        headerBounds.getRight() - margin - static_cast<int>(190.0f * scale),
        headerBounds.getY() + margin,
        static_cast<int>(180.0f * scale),
        static_cast<int>(28.0f * scale)
    );

    // Content area
    auto contentBounds = bounds.reduced(margin, margin / 2);
    
    int sectionWidth = (contentBounds.getWidth() - sectionGap * 2) / 3;
    int topRowHeight = static_cast<int>(160.0f * scale);
    int bottomRowHeight = contentBounds.getHeight() - topRowHeight - sectionGap;

    // === Position Knobs ===

    // Carrier Envelope Section
    auto carrierBounds = juce::Rectangle<int>(
        contentBounds.getX(),
        contentBounds.getY(),
        sectionWidth,
        topRowHeight
    );
    {
        auto knobArea = carrierBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
        int knobSpacing = knobArea.getWidth() / 3;
        
        attackKnob.setBounds(knobArea.getX() + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
        decayKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
        releaseKnob.setBounds(knobArea.getX() + knobSpacing * 2 + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
    }

    // Modulator Ratio Section
    auto ratioBounds = juce::Rectangle<int>(
        contentBounds.getX() + sectionWidth + sectionGap,
        contentBounds.getY(),
        sectionWidth,
        topRowHeight
    );
    {
        auto knobArea = ratioBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
        int knobSpacing = knobArea.getWidth() / 2;
        
        coarseKnob.setBounds(knobArea.getX() + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
        fineKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
    }

    // Tuning Section
    auto tuningBounds = juce::Rectangle<int>(
        contentBounds.getX() + (sectionWidth + sectionGap) * 2,
        contentBounds.getY(),
        sectionWidth,
        topRowHeight
    );
    {
        auto knobArea = tuningBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
        int knobSpacing = knobArea.getWidth() / 2;
        
        octaveKnob.setBounds(knobArea.getX() + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
        fineTuneKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
    }

    // Modulator Envelope Section
    auto modEnvBounds = juce::Rectangle<int>(
        contentBounds.getX(),
        contentBounds.getY() + topRowHeight + sectionGap,
        sectionWidth * 2 + sectionGap,
        bottomRowHeight
    );
    {
        auto knobArea = modEnvBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
        int knobSpacing = knobArea.getWidth() / 5;
        
        modInitKnob.setBounds(knobArea.getX() + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
        modDecKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
        modSusKnob.setBounds(knobArea.getX() + knobSpacing * 2 + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
        modRelKnob.setBounds(knobArea.getX() + knobSpacing * 3 + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
        modVelKnob.setBounds(knobArea.getX() + knobSpacing * 4 + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobArea.getHeight());
    }

    // Output Section
    auto outputBounds = juce::Rectangle<int>(
        contentBounds.getX() + (sectionWidth + sectionGap) * 2,
        contentBounds.getY() + topRowHeight + sectionGap,
        sectionWidth,
        bottomRowHeight
    );
    {
        auto knobArea = outputBounds.reduced(8, 0).withTrimmedTop(sectionPadding);
        int knobSpacing = knobArea.getWidth() / 2;
        int knobRowHeight = knobArea.getHeight() / 2;
        
        // Top row
        vibratoKnob.setBounds(knobArea.getX() + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobRowHeight);
        lfoRateKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing / 2 - knobSize / 2, knobArea.getY(), knobSize, knobRowHeight);
        
        // Bottom row
        waveformKnob.setBounds(knobArea.getX() + knobSpacing / 2 - knobSize / 2, knobArea.getY() + knobRowHeight, knobSize, knobRowHeight);
        modThruKnob.setBounds(knobArea.getX() + knobSpacing + knobSpacing / 2 - knobSize / 2, knobArea.getY() + knobRowHeight, knobSize, knobRowHeight);
    }
}
