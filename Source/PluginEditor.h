#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>
#include <vector>

class LynxTunerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit LynxTunerAudioProcessorEditor (LynxTunerAudioProcessor&);
    ~LynxTunerAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void timerCallback() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    LynxTunerAudioProcessor& processor;

    // Top
    juce::Label title;

    // INPUT
    juce::Slider tracking;
    juce::ComboBox inputRange, key, scale;

    // MODE
    juce::ToggleButton lowLatency;
    juce::ComboBox midiChannel;

    // OUTPUT
    juce::ToggleButton formant, inScale;
    juce::Slider formantShift, throat, transpose, reference, mix;

    // CORRECTION (main page)
    juce::ToggleButton classic, learnScale;
    juce::Slider retune, flexTune, naturalVibrato, humanize;

    // Tabs
    std::array<juce::TextButton, 3> pages;
    int activePage = 0; // 0 = Pitch Graph, 1 = Advanced, 2 = Real-Time

    // REAL-TIME page
    juce::ToggleButton speedLink, refTone, vibratoEnable;
    juce::Slider speed, tolCents, tolTime, vibDepth, correctionAmt, refToneLevel;

    // ADVANCED page
    juce::ComboBox vibratoShape;
    juce::ToggleButton ignoreVibrato, driveStage, wideFormant;
    juce::Slider vibRate, vibDepthAdv, vibAmp, vibFormant, vibVar, vibDelay, vibOnset;
    juce::Slider drive, outputTrim;

    // TARGET NOTES toolbar
    juce::TextButton bypassBtn, removeBtn, resetBtn, sendBtn, receiveBtn;
    juce::TextButton groupBtn, latchBtn;
    std::array<juce::TextButton, 7> rangeBtns; // Bass … Generic

    // 12 note switches
    std::array<juce::ToggleButton, 12> noteKeys;

    // Attachments
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<ComboAttachment>>  comboAttachments;

    // Live pitch history
    std::array<float, 240> pitchHistory{};
    int pitchHistoryWrite = 0;
    int lastLearnedMask = -1;
    bool removeMode = true; // true = Remove, false = Bypass when clicking piano

    juce::String midiNoteName (float normalised) const;
    void updatePageVisibility();
    void paintCorrectionDial (juce::Graphics& g, float sx, float sy);
    void paintPitchGraph (juce::Graphics& g, float sx, float sy);
    void paintKeyboard (juce::Graphics& g, float sx, float sy);
    void handleKeyboardClick (float x, float y);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LynxTunerAudioProcessorEditor)
};
