#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Scales.h"

using APVTS = juce::AudioProcessorValueTreeState;

APVTS::ParameterLayout PitchCorrectorProcessor::createLayout()
{
    APVTS::ParameterLayout l;
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "key", 1 }, "Key", Scales::keyNames, 0));
    l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "scale", 1 }, "Scale", Scales::scaleNames, 0));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "retune", 1 }, "Retune Speed (ms)",
          juce::NormalisableRange<float>(0.0f, 400.0f, 0.1f, 0.5f), 20.0f));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "humanize", 1 }, "Humanize",
          juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "transpose", 1 }, "Transpose (st)",
          juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "mix", 1 }, "Mix",
          juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "ref", 1 }, "Reference A (Hz)",
          juce::NormalisableRange<float>(430.0f, 450.0f, 0.1f), 440.0f));
    for (int i = 0; i < 12; ++i)
        l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "note" + juce::String(i), 1 },
              "Note " + Scales::keyNames[i], true));
    return l;
}

PitchCorrectorProcessor::PitchCorrectorProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    pKey = apvts.getRawParameterValue("key");
    pScale = apvts.getRawParameterValue("scale");
    pRetune = apvts.getRawParameterValue("retune");
    pHumanize = apvts.getRawParameterValue("humanize");
    pTranspose = apvts.getRawParameterValue("transpose");
    pMix = apvts.getRawParameterValue("mix");
    pRef = apvts.getRawParameterValue("ref");
    for (int i = 0; i < 12; ++i) pNote[i] = apvts.getRawParameterValue("note" + juce::String(i));
}

bool PitchCorrectorProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& in = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    if (in != out) return false;
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void PitchCorrectorProcessor::prepareToPlay(double sampleRate, int)
{
    sr = sampleRate;
    const int win = (int) (0.030 * sr);
    for (int c = 0; c < 2; ++c)
    {
        shifters[c].prepare(win);
        dry[c].prepare(win / 2 + 2);
    }
    setLatencySamples(win / 2 + 2);

    tauMin = juce::jmax(2, (int) (sr / 1200.0));
    tauMax = juce::jmin((int) (sr / 70.0), 1500);
    detector.prepare(tauMax);
    lin.assign((size_t) (W + tauMax), 0.0f);

    ring.fill(0.0f);
    ringW = hopCount = unvoiced = 0;
    corrGoal = corrSmooth = shiftAmt = 0.0f;
    detectedMidi = -1.0f;
    targetMidi = -1.0f;
}

void PitchCorrectorProcessor::analyse()
{
    const int L = W + tauMax;
    for (int i = 0; i < L; ++i)
        lin[(size_t) i] = ring[(size_t) ((ringW - L + i) & (ringSize - 1))];

    float energy = 0.0f;
    for (int i = 0; i < W; ++i) energy += lin[(size_t) i] * lin[(size_t) i];
    const float rms = std::sqrt(energy / (float) W);

    float conf = 0.0f;
    float f = 0.0f;
    if (rms > 0.003f)
        f = detector.detect(lin.data(), W, tauMin, tauMax, sr, 0.15f, conf);

    bool allowed[12];
    Scales::buildMask((int) pKey->load(), (int) pScale->load(), allowed);
    bool any = false;
    for (int i = 0; i < 12; ++i)
    {
        allowed[i] = allowed[i] && pNote[i]->load() > 0.5f;
        any = any || allowed[i];
    }

    if (f > 60.0f && f < 1400.0f && conf > 0.85f && any)
    {
        unvoiced = 0;
        const float midi = 69.0f + 12.0f * std::log2(f / pRef->load());
        const int base = (int) std::lround(midi);
        float best = 1e9f;
        int target = base;
        for (int k = -6; k <= 6; ++k)
        {
            const int n = base + k;
            const int pc = ((n % 12) + 12) % 12;
            if (allowed[pc] && std::abs((float) n - midi) < best)
            {
                best = std::abs((float) n - midi);
                target = n;
            }
        }

        float dev = (float) target - midi;
        const float h = pHumanize->load() / 100.0f;
        if (std::abs(dev) < h * 0.3f) dev = 0.0f;                  // zona muerta: conserva vibrato suave
        const float retuneMs = pRetune->load() * (1.0f + 4.0f * h); // humanize ralentiza la correccion
        const float hopMs = 1000.0f * (float) hop / (float) sr;
        const float alpha = retuneMs < 0.5f ? 1.0f : 1.0f - std::exp(-hopMs / retuneMs);
        corrGoal += (dev - corrGoal) * alpha;

        detectedMidi = midi;
        targetMidi = (float) target;
    }
    else
    {
        if (++unvoiced > 3) corrGoal *= 0.8f;
        detectedMidi = -1.0f;
        targetMidi = -1.0f;
    }
}

void PitchCorrectorProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int nCh = juce::jlimit(1, 2, juce::jmin(getTotalNumInputChannels(), getTotalNumOutputChannels()));
    for (int c = nCh; c < getTotalNumOutputChannels(); ++c) buffer.clear(c, 0, buffer.getNumSamples());

    const float transpose = pTranspose->load();
    const float mix = pMix->load() / 100.0f;
    const int n = buffer.getNumSamples();
    float* ch[2] = { buffer.getWritePointer(0), nCh > 1 ? buffer.getWritePointer(1) : nullptr };

    for (int i = 0; i < n; ++i)
    {
        float mono = ch[0][i];
        if (nCh > 1) mono = 0.5f * (mono + ch[1][i]);

        ring[(size_t) ringW] = mono;
        ringW = (ringW + 1) & (ringSize - 1);
        if (++hopCount >= hop) { hopCount = 0; analyse(); }

        corrSmooth += (corrGoal - corrSmooth) * 0.005f;
        const float semis = corrSmooth + transpose;
        const float ratio = std::exp2(semis / 12.0f);
        shiftAmt += ((std::abs(semis) > 0.03f ? 1.0f : 0.0f) - shiftAmt) * 0.002f;

        for (int c = 0; c < nCh; ++c)
        {
            const float x = ch[c][i];
            const float wet = shifters[c].process(x, ratio);
            const float d = dry[c].process(x);
            const float shifted = d + shiftAmt * (wet - d);
            ch[c][i] = d + mix * (shifted - d);
        }
    }
}

juce::AudioProcessorEditor* PitchCorrectorProcessor::createEditor() { return new PitchCorrectorEditor(*this); }

void PitchCorrectorProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary(*xml, dest);
}

void PitchCorrectorProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new PitchCorrectorProcessor(); }
