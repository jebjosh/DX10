#pragma once

#include <JuceHeader.h>

class SpectrumAnalyzer : public juce::Component,
                          private juce::Timer
{
public:
    SpectrumAnalyzer()
        : forwardFFT(fftOrder),
          window(fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        setOpaque(true);
        startTimerHz(30);
        
        // Initialize FFT data
        std::fill(fftData.begin(), fftData.end(), 0.0f);
        std::fill(scopeData.begin(), scopeData.end(), 0.0f);
    }

    ~SpectrumAnalyzer() override
    {
        stopTimer();
    }

    void pushBuffer(const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() > 0)
        {
            const auto* channelData = buffer.getReadPointer(0);
            
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                pushNextSampleIntoFifo(channelData[i]);
            }
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(juce::Colour(0xFF0D0D12));
        g.fillRoundedRectangle(bounds, 6.0f);
        
        // Border
        g.setColour(juce::Colour(0xFF2A2A35));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
        
        // Grid lines
        g.setColour(juce::Colour(0xFF1A1A22));
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();
        
        // Horizontal grid lines (dB levels)
        for (int i = 1; i < 5; ++i)
        {
            float y = bounds.getY() + (height * i / 5.0f);
            g.drawHorizontalLine(static_cast<int>(y), bounds.getX() + 2, bounds.getRight() - 2);
        }
        
        // Vertical grid lines (frequency decades)
        for (int i = 1; i < 4; ++i)
        {
            float x = bounds.getX() + (width * i / 4.0f);
            g.drawVerticalLine(static_cast<int>(x), bounds.getY() + 2, bounds.getBottom() - 2);
        }
        
        // Draw spectrum
        drawSpectrum(g, bounds.reduced(4.0f));
    }

    void resized() override
    {
        // Nothing needed
    }

private:
    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextFrameOfSpectrum();
            nextFFTBlockReady = false;
            repaint();
        }
    }

    void pushNextSampleIntoFifo(float sample)
    {
        if (fifoIndex == fftSize)
        {
            if (!nextFFTBlockReady)
            {
                std::copy(fifo.begin(), fifo.end(), fftData.begin());
                nextFFTBlockReady = true;
            }
            fifoIndex = 0;
        }
        
        fifo[static_cast<size_t>(fifoIndex++)] = sample;
    }

    void drawNextFrameOfSpectrum()
    {
        // Apply window function
        window.multiplyWithWindowingTable(fftData.data(), fftSize);
        
        // Perform FFT
        forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
        
        // Convert to dB and smooth
        auto mindB = -100.0f;
        auto maxdB = 0.0f;
        
        for (size_t i = 0; i < scopeSize; ++i)
        {
            // Logarithmic frequency mapping
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - static_cast<float>(i) / static_cast<float>(scopeSize)) * 0.2f);
            auto fftDataIndex = static_cast<size_t>(skewedProportionX * static_cast<float>(fftSize / 2));
            
            auto level = juce::jmap(juce::jlimit(mindB, maxdB,
                juce::Decibels::gainToDecibels(fftData[fftDataIndex])
                - juce::Decibels::gainToDecibels(static_cast<float>(fftSize))),
                mindB, maxdB, 0.0f, 1.0f);
            
            // Smoothing
            scopeData[i] = scopeData[i] * 0.7f + level * 0.3f;
        }
    }

    void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();
        
        // Create gradient for spectrum fill
        juce::ColourGradient gradient(
            juce::Colour(0xFF00D4AA).withAlpha(0.8f),
            bounds.getX(), bounds.getBottom(),
            juce::Colour(0xFF00A080).withAlpha(0.3f),
            bounds.getX(), bounds.getY(),
            false
        );
        
        // Draw filled spectrum
        juce::Path spectrumPath;
        spectrumPath.startNewSubPath(bounds.getX(), bounds.getBottom());
        
        for (size_t i = 0; i < scopeSize; ++i)
        {
            float x = bounds.getX() + static_cast<float>(i) / static_cast<float>(scopeSize) * width;
            float y = bounds.getBottom() - scopeData[i] * height;
            
            if (i == 0)
                spectrumPath.lineTo(x, y);
            else
                spectrumPath.lineTo(x, y);
        }
        
        spectrumPath.lineTo(bounds.getRight(), bounds.getBottom());
        spectrumPath.closeSubPath();
        
        g.setGradientFill(gradient);
        g.fillPath(spectrumPath);
        
        // Draw outline
        juce::Path outlinePath;
        bool started = false;
        
        for (size_t i = 0; i < scopeSize; ++i)
        {
            float x = bounds.getX() + static_cast<float>(i) / static_cast<float>(scopeSize) * width;
            float y = bounds.getBottom() - scopeData[i] * height;
            
            if (!started)
            {
                outlinePath.startNewSubPath(x, y);
                started = true;
            }
            else
            {
                outlinePath.lineTo(x, y);
            }
        }
        
        g.setColour(juce::Colour(0xFF00D4AA));
        g.strokePath(outlinePath, juce::PathStrokeType(1.5f));
    }

    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;  // 2048
    static constexpr size_t scopeSize = 256;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    std::array<float, scopeSize> scopeData;
    
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};
