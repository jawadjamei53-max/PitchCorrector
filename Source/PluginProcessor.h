#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <array>
#include "PitchDetector.h"
#include "PitchShifter.h"

class PitchCorrectorProcessor : public juce::AudioProcessor
{
public:
    PitchCorrectorProcessor();
    ~PitchCorrectorProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "PitchCorrector"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "STATE", createLayout() };

    std::atomic<float> detectedMidi { -1.0f }, targetMidi { -1.0f };

private:
    void analyse();

    static constexpr int ringSize = 4096;
    static constexpr int hop = 512;
    static constexpr int W = 1024;

    double sr = 44100.0;
    std::array<float, ringSize> ring {};
    int ringW = 0, hopCount = 0;
    std::vector<float> lin;
    int tauMin = 2, tauMax = 600;
    PitchDetector detector;

    PitchShifter shifters[2];
    DelayLine dry[2];

    float corrGoal = 0.0f, corrSmooth = 0.0f, shiftAmt = 0.0f;
    int unvoiced = 0;

    std::atomic<float>* pKey, *pScale, *pRetune, *pHumanize, *pTranspose, *pMix, *pRef;
    std::atomic<float>* pNote[12];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchCorrectorProcessor)
};
