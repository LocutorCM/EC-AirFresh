#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AirLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(x, y, w, h).reduced(6);
        auto centre = bounds.getCentre();
        float radius = bounds.getWidth() * 0.5f;

        g.setColour(juce::Colour(0xff4fc3f7).withAlpha(0.35f));
        g.drawEllipse(bounds.expanded(6), 3.0f);
        g.setColour(juce::Colour(0xff4fc3f7).withAlpha(0.15f));
        g.fillEllipse(bounds.expanded(6));

        juce::Path bgArc;
        bgArc.addCentredArc(centre.x, centre.y, radius, radius, 0, startAngle, endAngle, true);
        g.setColour(juce::Colour(0xffcfe8f5));
        g.strokePath(bgArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, radius, radius, 0, startAngle, sliderPos, true);
        g.setColour(juce::Colour(0xff29b6f6));
        g.strokePath(valueArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        auto knobBounds = bounds.reduced(18);
        juce::ColourGradient grad(juce::Colour(0xffffffff), knobBounds.getTopLeft(),
                                  juce::Colour(0xffa8d8f0), knobBounds.getBottomRight(), false);
        grad.addColour(0.5, juce::Colour(0xffd6ecf8));
        g.setGradientFill(grad);
        g.fillEllipse(knobBounds);
        g.setColour(juce::Colour(0xff7fb8d9));
        g.drawEllipse(knobBounds, 1.5f);

        auto pointer = centre.getPointOnCircumference(radius * 0.62f, sliderPos);
        g.setColour(juce::Colour(0xff00bfff));
        g.drawLine(centre.x, centre.y, pointer.x, pointer.y, 4.0f);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool, bool) override
    {
        auto bounds = b.getLocalBounds().toFloat().reduced(2);
        g.setColour(b.getToggleState() ? juce::Colour(0xff29b6f6) : juce::Colour(0xffb0bec5));
        g.fillRoundedRectangle(bounds, 10.0f);
        g.setColour(juce::Colours::white);
        float x = b.getToggleState() ? bounds.getRight() - 22 : bounds.getX() + 2;
        g.fillEllipse(x, bounds.getY() + 2, 20, bounds.getHeight() - 4);
    }
};

class WaveformComponent : public juce::Component
{
public:
    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour(0xff4fc3f7).withAlpha(0.6f));
        juce::Path p;
        auto w = (float)getWidth();
        auto h = (float)getHeight();
        p.startNewSubPath(0, h * 0.5f);
        for (int x = 0; x < (int)w; ++x)
        {
            float t = x / w;
            float y = h * 0.5f + std::sin(t * 18.0f) * 8.0f * std::sin(t * 3.14f)
                      + std::sin(t * 42.0f) * 4.0f * std::sin(t * 3.14f);
            p.lineTo(x, y);
        }
        g.strokePath(p, juce::PathStrokeType(1.5f));
    }
};

class EC_AIRAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   public juce::Timer
{
public:
    EC_AIRAudioProcessorEditor(EC_AIRAudioProcessor&);
    ~EC_AIRAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    EC_AIRAudioProcessor& audioProcessor;
    AirLookAndFeel look;
    WaveformComponent waveform;

    juce::Slider midAir, highAir, mix, output;
    juce::ToggleButton power;
    juce::TextButton bypass { "BYPASS" };
    juce::ComboBox presetBox;
    juce::Label midLabel, highLabel, midValue, highValue, mixLabel, mixValue, outLabel, outValue;

    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<Attach> midAttach, highAttach, mixAttach, outAttach;
    std::unique_ptr<ButtonAttach> powerAttach;

    void applyPreset(int index);
    void updateLabels();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EC_AIRAudioProcessorEditor)
};
