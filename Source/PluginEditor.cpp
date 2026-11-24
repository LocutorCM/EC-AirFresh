#include "PluginEditor.h"

EC_AIRAudioProcessorEditor::EC_AIRAudioProcessorEditor(EC_AIRAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(800, 600);
    setLookAndFeel(&look);

    auto setupKnob = [this](juce::Slider& s, const char* id, juce::Label& label, const char* text)
    {
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setLookAndFeel(&look);
        addAndMakeVisible(s);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colour(0xff2a7ab5));
        label.setFont(juce::FontOptions(22.0f, juce::Font::bold));
        addAndMakeVisible(label);
    };

    // CAMBIA ESTOS IDs si en tu PluginProcessor.cpp se llaman diferente
    setupKnob(midAir, "midAir", midLabel, "Mid Air");
    setupKnob(highAir, "highAir", highLabel, "High Air");
    setupKnob(mix, "mix", mixLabel, "MIX");
    setupKnob(output, "output", outLabel, "OUTPUT");

    midAttach = std::make_unique<Attach>(p.apvts, "midAir", midAir);
    highAttach = std::make_unique<Attach>(p.apvts, "highAir", highAir);
    mixAttach = std::make_unique<Attach>(p.apvts, "mix", mix);
    outAttach = std::make_unique<Attach>(p.apvts, "output", output);

    power.setLookAndFeel(&look);
    power.setButtonText("");
    addAndMakeVisible(power);
    powerAttach = std::make_unique<ButtonAttach>(p.apvts, "power", power);

    bypass.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffeef6fb));
    bypass.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff78909c));
    addAndMakeVisible(bypass);

    auto setupValue = [this](juce::Label& l, const char* t)
    {
        l.setText(t, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, juce::Colour(0xff2a7ab5));
        l.setFont(juce::FontOptions(16.0f));
        addAndMakeVisible(l);
    };
    setupValue(midValue, "+2.4 dB");
    setupValue(highValue, "+4.1 dB");
    setupValue(mixValue, "65%");
    setupValue(outValue, "-0.5 dB");

    addAndMakeVisible(waveform);
}

EC_AIRAudioProcessorEditor::~EC_AIRAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void EC_AIRAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fondo degradado blanco -> azul clarito
    juce::ColourGradient bg(juce::Colour(0xffffffff), 0, 0,
                            juce::Colour(0xffdbeefb), 0, (float)getHeight(), false);
    g.setGradientFill(bg);
    g.fillAll();

    // Borde redondeado
    g.setColour(juce::Colour(0xffb0d4ea));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 18.0f, 1.0f);

    // Header
    g.setColour(juce::Colour(0xff2a7ab5));
    g.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    g.drawText("EC AUDIO", 40, 20, 200, 30, juce::Justification::left);

    g.setColour(juce::Colour(0xff78909c));
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("Preset:  Warm & Airy", 540, 22, 200, 26, juce::Justification::right);

    // Titulo
    g.setColour(juce::Colour(0xff2a7ab5));
    g.setFont(juce::FontOptions(42.0f, juce::Font::bold));
    g.drawText("EC AIR EXCITER", 0, 70, getWidth(), 50, juce::Justification::centred);

    g.setColour(juce::Colour(0xff78909c));
    g.setFont(juce::FontOptions(18.0f));
    g.drawText("HARMONIC EXCITER — ADD AIR & CLARITY", 0, 118, getWidth(), 25, juce::Justification::centred);

    // Barra inferior
    g.setColour(juce::Colour(0xfff2f8fc));
    g.fillRoundedRectangle(40, 400, getWidth() - 80, 90, 12.0f);
    g.setColour(juce::Colour(0xffcfe8f5));
    g.drawRoundedRectangle(juce::Rectangle<float>(40, 400, getWidth() - 80, 90), 12.0f, 1.0f);

    g.setColour(juce::Colour(0xff78909c));
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("POWER", 140, 428, 70, 20, juce::Justification::right);
    g.drawText("INPUT", 40, 540, 100, 20, juce::Justification::left);
    g.drawText("OUTPUT", getWidth() - 140, 540, 100, 20, juce::Justification::right);
    g.drawText("VERSION 1.0  •  48kHz  •  STEREO", 40, 565, 300, 20, juce::Justification::left);
    g.drawText("SAFE  •  OVERSAMPLE 2x", getWidth() - 340, 565, 300, 20, juce::Justification::right);
}

void EC_AIRAudioProcessorEditor::resized()
{
    midAir.setBounds(120, 165, 200, 200);
    highAir.setBounds(480, 165, 200, 200);
    midLabel.setBounds(120, 365, 200, 30);
    highLabel.setBounds(480, 365, 200, 30);
    midValue.setBounds(120, 395, 200, 20);
    highValue.setBounds(480, 395, 200, 20);

    power.setBounds(215, 428, 55, 28);
    bypass.setBounds(285, 428, 80, 28);
    mix.setBounds(400, 415, 65, 65);
    output.setBounds(560, 415, 65, 65);
    mixLabel.setBounds(400, 398, 65, 20);
    outLabel.setBounds(560, 398, 65, 20);
    mixValue.setBounds(400, 478, 65, 18);
    outValue.setBounds(560, 478, 65, 18);

    waveform.setBounds(40, 500, getWidth() - 80, 38);
}
