#pragma once

#include <JuceHeader.h>
#include <vector>

// Forward declare structs outside the class to avoid template issues
struct PresetItem
{
    juce::String name;
    juce::File file;
    bool isFolder = false;
    int depth = 0;
    
    PresetItem() = default;
    PresetItem(const juce::String& n, const juce::File& f, bool folder, int d)
        : name(n), file(f), isFolder(folder), depth(d) {}
};

struct FlatPresetItem
{
    juce::String displayName;
    juce::File file;
    bool isFolder = false;
    int depth = 0;
    
    FlatPresetItem() = default;
    FlatPresetItem(const juce::String& name, const juce::File& f, bool folder, int d)
        : displayName(name), file(f), isFolder(folder), depth(d) {}
};

class PresetManager
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts) : valueTreeState(apvts)
    {
        loadSettings();
        
        // Use custom directory if set, otherwise default
        if (customPresetDirectory.isDirectory())
        {
            presetDirectory = customPresetDirectory;
        }
        else
        {
            presetDirectory = getDefaultPresetDirectory();
        }
        
        if (!presetDirectory.exists())
            presetDirectory.createDirectory();
    }

    static juce::String getPresetExtension() { return ".dx10"; }
    
    static juce::File getDefaultPresetDirectory()
    {
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("DX10 Presets");
    }
    
    static juce::File getSettingsFile()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("DX10")
            .getChildFile("settings.xml");
    }
    
    juce::File getPresetDirectory() const { return presetDirectory; }
    
    void setPresetDirectory(const juce::File& newDirectory)
    {
        if (newDirectory.isDirectory())
        {
            presetDirectory = newDirectory;
            customPresetDirectory = newDirectory;
            saveSettings();
        }
    }
    
    void resetToDefaultDirectory()
    {
        presetDirectory = getDefaultPresetDirectory();
        customPresetDirectory = juce::File();
        if (!presetDirectory.exists())
            presetDirectory.createDirectory();
        saveSettings();
    }
    
    juce::String getLastLoadedPreset() const { return lastLoadedPreset; }
    
    void setLastLoadedPreset(const juce::String& presetPath)
    {
        lastLoadedPreset = presetPath;
        saveSettings();
    }

    bool savePresetToFile(const juce::File& file)
    {
        auto state = valueTreeState.copyState();
        
        // Remove PresetIndex and SelectedPresetId from saved state - these are internal tracking params
        for (int i = state.getNumChildren() - 1; i >= 0; --i)
        {
            auto child = state.getChild(i);
            if (child.hasType("PARAM"))
            {
                auto paramId = child.getProperty("id").toString();
                if (paramId == "PresetIndex" || paramId == "SelectedPresetId")
                    state.removeChild(i, nullptr);
            }
        }
        
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        if (xml == nullptr)
            return false;
        
        xml->setAttribute("presetName", file.getFileNameWithoutExtension());
        xml->setAttribute("pluginVersion", "1.0");
        
        // Ensure parent directory exists
        file.getParentDirectory().createDirectory();
        
        return xml->writeTo(file);
    }

    bool loadPresetFromFile(const juce::File& file)
    {
        if (!file.existsAsFile())
            return false;
        
        std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(file);
        if (xml == nullptr)
            return false;
        
        if (!xml->hasTagName(valueTreeState.state.getType()))
            return false;
        
        // Begin undo transaction for preset load
        if (auto* undoManager = valueTreeState.undoManager)
            undoManager->beginNewTransaction("Load Preset: " + file.getFileNameWithoutExtension());
        
        auto newState = juce::ValueTree::fromXml(*xml);
        
        // Set each parameter individually so undo works
        // Skip PresetIndex and SelectedPresetId - these will be set by the caller
        for (int i = 0; i < newState.getNumChildren(); ++i)
        {
            auto child = newState.getChild(i);
            if (child.hasType("PARAM"))
            {
                auto paramId = child.getProperty("id").toString();
                // Skip internal tracking parameters
                if (paramId == "PresetIndex" || paramId == "SelectedPresetId")
                    continue;
                auto value = static_cast<float>(child.getProperty("value"));
                if (auto* param = valueTreeState.getParameter(paramId))
                    param->setValueNotifyingHost(value);
            }
        }
        
        setLastLoadedPreset(file.getFullPathName());
        
        return true;
    }
    
    // Load last used preset on startup
    bool loadLastPreset()
    {
        if (lastLoadedPreset.isNotEmpty())
        {
            juce::File file(lastLoadedPreset);
            if (file.existsAsFile())
                return loadPresetFromFile(file);
        }
        return false;
    }
    
    // Get flat list with indentation info for combo box
    std::vector<FlatPresetItem> getFlatPresetList(int maxDepth = 3) const
    {
        std::vector<FlatPresetItem> items;
        scanDirectoryFlat(presetDirectory, items, 0, maxDepth);
        return items;
    }

    juce::File getPresetFile(const juce::String& presetName) const
    {
        return presetDirectory.getChildFile(presetName + getPresetExtension());
    }

    static bool isValidPresetFile(const juce::File& file)
    {
        if (!file.existsAsFile())
            return false;
        
        if (file.getFileExtension().toLowerCase() != getPresetExtension())
            return false;
        
        std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(file);
        return xml != nullptr && xml->hasTagName("Parameters");
    }

private:
    void scanDirectoryFlat(const juce::File& dir, std::vector<FlatPresetItem>& items, int depth, int maxDepth) const
    {
        if (depth > maxDepth) return;
        
        auto children = dir.findChildFiles(juce::File::findFilesAndDirectories, false);
        children.sort();
        
        // First add folders and their contents
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& child = children[i];
            if (child.isDirectory() && !child.getFileName().startsWithChar('.'))
            {
                // Check if folder has any presets
                auto folderPresets = child.findChildFiles(juce::File::findFiles, true, "*" + getPresetExtension());
                if (folderPresets.size() > 0)
                {
                    items.push_back(FlatPresetItem(child.getFileName(), child, true, depth));
                    scanDirectoryFlat(child, items, depth + 1, maxDepth);
                }
            }
        }
        
        // Then add preset files at this level
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& child = children[i];
            if (child.hasFileExtension(getPresetExtension()))
            {
                items.push_back(FlatPresetItem(child.getFileNameWithoutExtension(), child, false, depth));
            }
        }
    }
    
    void loadSettings()
    {
        auto settingsFile = getSettingsFile();
        if (settingsFile.existsAsFile())
        {
            std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(settingsFile);
            if (xml != nullptr && xml->hasTagName("DX10Settings"))
            {
                auto customDir = xml->getStringAttribute("customPresetDirectory");
                if (customDir.isNotEmpty())
                    customPresetDirectory = juce::File(customDir);
                
                lastLoadedPreset = xml->getStringAttribute("lastLoadedPreset");
            }
        }
    }
    
    void saveSettings()
    {
        auto settingsFile = getSettingsFile();
        settingsFile.getParentDirectory().createDirectory();
        
        juce::XmlElement xml("DX10Settings");
        if (customPresetDirectory.isDirectory())
            xml.setAttribute("customPresetDirectory", customPresetDirectory.getFullPathName());
        if (lastLoadedPreset.isNotEmpty())
            xml.setAttribute("lastLoadedPreset", lastLoadedPreset);
        
        xml.writeTo(settingsFile);
    }

    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::File presetDirectory;
    juce::File customPresetDirectory;
    juce::String lastLoadedPreset;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
