#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

class PitchCorrectorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PitchCorrectorEditor(PitchCorrectorProcessor&);
    ~PitchCorrectorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void addKnob(juce::Slider&, juce::Label&, const juce::String& text);

    PitchCorrectorProcessor& proc;
    juce::ComboBox keyBox, scaleBox;
    juce::Slider retune, humanize, transpose, mix, ref;
    juce::Label lRetune, lHumanize, lTranspose, lMix, lRef, lKey, lScale, info;
    juce::ToggleButton noteBtn[12];

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SA> aRetune, aHumanize, aTranspose, aMix, aRef;
    std::unique_ptr<CA> aKey, aScale;
    std::unique_ptr<BA> aNote[12];
};
