#include "PluginEditor.h"
#include "Scales.h"

void PitchCorrectorEditor::addKnob(juce::Slider& s, juce::Label& l, const juce::String& text)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    addAndMakeVisible(s);
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(l);
}

PitchCorrectorEditor::PitchCorrectorEditor(PitchCorrectorProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setSize(640, 330);

    keyBox.addItemList(Scales::keyNames, 1);
    scaleBox.addItemList(Scales::scaleNames, 1);
    addAndMakeVisible(keyBox);
    addAndMakeVisible(scaleBox);
    lKey.setText("Key", juce::dontSendNotification);
    lScale.setText("Scale", juce::dontSendNotification);
    addAndMakeVisible(lKey);
    addAndMakeVisible(lScale);

    addKnob(retune, lRetune, "Retune (ms)");
    addKnob(humanize, lHumanize, "Humanize");
    addKnob(transpose, lTranspose, "Transpose (st)");
    addKnob(ref, lRef, "A (Hz)");
    addKnob(mix, lMix, "Mix");

    auto& v = proc.apvts;
    aKey = std::make_unique<CA>(v, "key", keyBox);
    aScale = std::make_unique<CA>(v, "scale", scaleBox);
    aRetune = std::make_unique<SA>(v, "retune", retune);
    aHumanize = std::make_unique<SA>(v, "humanize", humanize);
    aTranspose = std::make_unique<SA>(v, "transpose", transpose);
    aRef = std::make_unique<SA>(v, "ref", ref);
    aMix = std::make_unique<SA>(v, "mix", mix);

    for (int i = 0; i < 12; ++i)
    {
        noteBtn[i].setButtonText(Scales::keyNames[i]);
        addAndMakeVisible(noteBtn[i]);
        aNote[i] = std::make_unique<BA>(v, "note" + juce::String(i), noteBtn[i]);
    }

    info.setJustificationType(juce::Justification::centred);
    info.setFont(juce::FontOptions(18.0f));
    addAndMakeVisible(info);
    startTimerHz(30);
}

void PitchCorrectorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1b1d24));
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(20.0f));
    g.drawText("PitchCorrector", 10, 6, 300, 26, juce::Justification::left);
}

void PitchCorrectorEditor::resized()
{
    auto r = getLocalBounds().reduced(10);
    r.removeFromTop(30);

    auto top = r.removeFromTop(28);
    lKey.setBounds(top.removeFromLeft(40));
    keyBox.setBounds(top.removeFromLeft(90));
    top.removeFromLeft(20);
    lScale.setBounds(top.removeFromLeft(50));
    scaleBox.setBounds(top.removeFromLeft(160));

    r.removeFromTop(8);
    auto notes = r.removeFromTop(34);
    const int w = notes.getWidth() / 12;
    for (int i = 0; i < 12; ++i) noteBtn[i].setBounds(notes.removeFromLeft(w));

    r.removeFromTop(8);
    info.setBounds(r.removeFromTop(30));

    auto knobs = r;
    const int kw = knobs.getWidth() / 5;
    juce::Slider* sl[5] = { &retune, &humanize, &transpose, &ref, &mix };
    juce::Label* lb[5] = { &lRetune, &lHumanize, &lTranspose, &lRef, &lMix };
    for (int i = 0; i < 5; ++i)
    {
        auto c = knobs.removeFromLeft(kw);
        lb[i]->setBounds(c.removeFromTop(20));
        sl[i]->setBounds(c);
    }
}

void PitchCorrectorEditor::timerCallback()
{
    const float d = proc.detectedMidi.load(), t = proc.targetMidi.load();
    if (d < 0.0f || t < 0.0f) { info.setText("-", juce::dontSendNotification); return; }
    const int ni = (int) std::lround(d);
    const float cents = (d - (float) ni) * 100.0f;
    const int ti = (int) t;
    auto name = [](int m) { return Scales::keyNames[((m % 12) + 12) % 12] + juce::String(m / 12 - 1); };
    info.setText(name(ni) + " (" + juce::String(cents, 0) + " cents)  ->  " + name(ti), juce::dontSendNotification);
}
