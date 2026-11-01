#include "PluginEditor.h"
#include <cmath>

namespace
{
const juce::Colour bg          { 0xff09090d };
const juce::Colour panel       { 0xff111116 };
const juce::Colour panelRaised { 0xff19191f };
const juce::Colour border      { 0xff30202d };
const juce::Colour pink        { 0xffec4899 };
const juce::Colour pinkSoft    { 0xfff472b6 };
const juce::Colour white       { 0xfff4f1f5 };
const juce::Colour muted       { 0xff89838f };
const juce::Colour cyan        { 0xff22d3ee };
const juce::Colour amber       { 0xfffbbf24 };

void styleCombo (juce::ComboBox& c)
{
    c.setColour (juce::ComboBox::backgroundColourId, panelRaised);
    c.setColour (juce::ComboBox::textColourId, pinkSoft);
    c.setColour (juce::ComboBox::outlineColourId, border);
    c.setColour (juce::ComboBox::arrowColourId, pinkSoft);
    c.setJustificationType (juce::Justification::centred);
}
float readParam (LynxTunerAudioProcessor& processor, int index)
{
    if (auto* value = processor.parameters().getRawParameterValue ("p" + juce::String (index)))
        return value->load (std::memory_order_relaxed);
    return 0.0f;
}
juce::String noteName (int midi)
{
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    if (midi < 0) return "--";
    return juce::String (names[(midi % 12 + 12) % 12]) + juce::String (midi / 12 - 1);
}
}

LynxTunerAudioProcessorEditor::LynxTunerAudioProcessorEditor (LynxTunerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (800, 600);
    setResizable (true, true);
    setResizeLimits (680, 520, 1200, 900);

    header.setText ("LYNX", juce::dontSendNotification);
    header.setFont (juce::FontOptions (22.0f).withStyle ("Bold"));
    header.setColour (juce::Label::textColourId, white);
    addAndMakeVisible (header);

    subtitle.setText ("LIVE AUTOTUNER  •  ZERO-LATENCY PITCH CORRECTION", juce::dontSendNotification);
    subtitle.setFont (juce::FontOptions (9.5f).withStyle ("Bold"));
    subtitle.setColour (juce::Label::textColourId, muted);
    addAndMakeVisible (subtitle);

    auto& state = processor.parameters();
    auto addCombo = [&] (juce::ComboBox& c, int id, const juce::StringArray& items)
    {
        for (int i = 0; i < items.size(); ++i) c.addItem (items[i], i + 1);
        styleCombo (c);
        addAndMakeVisible (c);
        comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "p" + juce::String (id), c));
    };
    addCombo (key, lynx::lynxtune::key, { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" });
    addCombo (scale, lynx::lynxtune::scale, { "Chromatic", "Major", "Minor", "Harmonic Minor", "Melodic Minor",
                                               "Major Pentatonic", "Minor Pentatonic", "Blues", "Dorian",
                                               "Mixolydian", "Phrygian", "Lydian", "Locrian", "Custom" });

    formantPreserve.setButtonText ("FORMANT LOCK");
    formantPreserve.setColour (juce::ToggleButton::textColourId, pinkSoft);
    formantPreserve.setColour (juce::ToggleButton::tickColourId, pink);
    addAndMakeVisible (formantPreserve);
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state,
        "p" + juce::String (lynx::lynxtune::formantPreserve), formantPreserve));

    addKnob (speed, "RETUNE SPEED", lynx::lynxtune::speed);
    addKnob (smoothness, "SMOOTHNESS", lynx::lynxtune::flexTune);
    addKnob (correction, "CORRECTION", lynx::lynxtune::correction);
    addKnob (formantShift, "FORMANT SHIFT", lynx::lynxtune::formantShift);
    addKnob (reference, "REF A4", lynx::lynxtune::reference);
    addKnob (mix, "MIX", lynx::lynxtune::mix);
    addKnob (outputGain, "OUTPUT", lynx::lynxtune::outputTrim);

    status.setColour (juce::Label::textColourId, muted);
    status.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
    status.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (status);
    targetLabel.setColour (juce::Label::textColourId, pinkSoft);
    targetLabel.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
    targetLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (targetLabel);
    centsLabel.setColour (juce::Label::textColourId, amber);
    centsLabel.setFont (juce::FontOptions (19.0f).withStyle ("Bold"));
    centsLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (centsLabel);
    inputLabel.setColour (juce::Label::textColourId, white);
    inputLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (inputLabel);
    correctedLabel.setColour (juce::Label::textColourId, pinkSoft);
    correctedLabel.setFont (juce::FontOptions (10.0f));
    addAndMakeVisible (correctedLabel);
    depthLabel.setColour (juce::Label::textColourId, muted);
    depthLabel.setFont (juce::FontOptions (9.0f));
    addAndMakeVisible (depthLabel);
    startTimerHz (20);
}

void LynxTunerAudioProcessorEditor::addKnob (juce::Slider& s, const juce::String& name, int id)
{
    s.setName (name);
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 76, 17);
    s.setColour (juce::Slider::rotarySliderFillColourId, pink);
    s.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff34313b));
    s.setColour (juce::Slider::thumbColourId, white);
    s.setColour (juce::Slider::textBoxTextColourId, pinkSoft);
    s.setColour (juce::Slider::textBoxBackgroundColourId, panel);
    s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (s);
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (
        processor.parameters(), "p" + juce::String (id), s));
}

void LynxTunerAudioProcessorEditor::drawKnobGroup (juce::Graphics& g, juce::Rectangle<int> r,
                                                    const juce::String& title)
{
    g.setColour (panel);
    g.fillRoundedRectangle (r.toFloat(), 12.0f);
    g.setColour (border);
    g.drawRoundedRectangle (r.toFloat(), 12.0f, 1.0f);
    g.setColour (pink);
    g.fillRoundedRectangle ((float) r.getX() + 12.0f, (float) r.getY(), (float) r.getWidth() - 24.0f, 2.0f, 1.0f);
    g.setColour (muted);
    g.setFont (juce::FontOptions (9.0f).withStyle ("Bold"));
    g.drawText (title, r.withHeight (22).translated (0, 5), juce::Justification::centred, false);
}

void LynxTunerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    const float sx = (float) getWidth() / 800.0f;
    const float sy = (float) getHeight() / 600.0f;
    auto R = [&] (float x, float y, float w, float h) { return juce::Rectangle<float> (x*sx, y*sy, w*sx, h*sy); };

    g.setColour (juce::Colour (0xff111015));
    g.fillRect (R (0, 0, 800, 62));
    g.setColour (border);
    g.drawLine (0, 61*sy, (float)getWidth(), 61*sy, 1.0f);
    g.setColour (pink);
    g.fillRoundedRectangle (R (20, 15, 30, 30), 8.0f*sx);
    g.setColour (white);
    g.setFont (juce::FontOptions (17.0f*sx).withStyle ("Bold"));
    g.drawText ("✦", R (20, 15, 30, 30).toNearestInt(), juce::Justification::centred);
    g.setColour (muted);
    g.setFont (juce::FontOptions (8.5f*sx).withStyle ("Bold"));
    g.drawText ("LIVE", R (78, 39, 50, 12).toNearestInt(), juce::Justification::centredLeft);

    drawKnobGroup (g, R (16, 74, 768, 184).toNearestInt(), "PITCH CORRECTION");
    const auto meter = R (30, 104, 740, 100);
    g.setColour (juce::Colour (0xff08080c));
    g.fillRoundedRectangle (meter, 12.0f*sx);
    g.setColour (border);
    g.drawRoundedRectangle (meter, 12.0f*sx, 1.0f);
    const float centre = meter.getCentreX();
    g.setColour (juce::Colour (0xff28232d));
    g.fillRoundedRectangle (juce::Rectangle<float> (meter.getX()+12*sx, meter.getY()+48*sy, meter.getWidth()-24*sx, 12*sy), 5.0f);
    g.setColour (pink);
    g.fillRect (juce::Rectangle<float> (centre-1.0f*sx, meter.getY()+43*sy, 2.0f*sx, 22*sy));
    const float sung = processor.engine().controlOutput (0) * 127.0f;
    const float target = processor.engine().controlOutput (1) * 127.0f;
    const float cents = (processor.engine().controlOutput (2) - 0.5f) * 400.0f;
    if (sung > 0.0f)
    {
        const float x = centre + juce::jlimit (-50.0f, 50.0f, cents) * (meter.getWidth() * 0.009f);
        g.setColour (cents >= 0 ? amber : cyan);
        g.fillRoundedRectangle (juce::Rectangle<float> (std::min (centre, x), meter.getY()+46*sy,
                                                        std::max (2.0f*sx, std::abs (x-centre)), 16*sy), 5.0f);
    }
    g.setColour (muted);
    g.setFont (juce::FontOptions (8.0f*sx));
    g.drawText ("-50¢  FLAT", R (36, 214, 100, 12).toNearestInt(), juce::Justification::centredLeft);
    g.drawText ("TARGET  0¢", R (350, 214, 100, 12).toNearestInt(), juce::Justification::centred);
    g.drawText ("+50¢  SHARP", R (664, 214, 100, 12).toNearestInt(), juce::Justification::centredRight);

    drawKnobGroup (g, R (16, 270, 768, 126).toNearestInt(), "SCALE QUANTIZER PITCH GRID");
    const int keyIdx = juce::jlimit (0, 11, (int) std::lround (readParam (processor, lynx::lynxtune::key) * 11.0f));
    const int scaleIdx = juce::jlimit (0, 13, (int) std::lround (readParam (processor, lynx::lynxtune::scale) * 13.0f));
    static const int masks[14] = { 4095, 2741, 1453, 2477, 2741, 661, 661, 1197, 1709, 2741, 1451, 2741, 1453, 2741 };
    const int mask = masks[scaleIdx];
    const int targetPc = sung > 0.0f ? ((int)std::lround (target) % 12) : -1;
    for (int i=0; i<12; ++i)
    {
        const int pc = (i + keyIdx) % 12;
        const bool inScale = (mask & (1 << i)) != 0;
        const bool active = targetPc == pc;
        auto keyRect = R (24 + i*62, 308, 56, 66);
        g.setColour (active ? pink : (inScale ? (i == 0 ? juce::Colour (0xff39172b) : panelRaised) : bg));
        g.fillRoundedRectangle (keyRect, 6.0f*sx);
        g.setColour (active ? pinkSoft : (inScale ? border : juce::Colour (0xff211e25)));
        g.drawRoundedRectangle (keyRect, 6.0f*sx, 1.0f);
        g.setColour (active ? bg : (inScale ? white : juce::Colour (0xff625c67)));
        g.setFont (juce::FontOptions (10.0f*sx).withStyle ("Bold"));
        static const char* names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        g.drawText (names[pc], keyRect.withY (keyRect.getY()+13*sy).withHeight (18*sy).toNearestInt(), juce::Justification::centred);
        g.setFont (juce::FontOptions (7.0f*sx).withStyle ("Bold"));
        g.drawText (active ? "SNAP" : (i==0 ? "ROOT" : (inScale ? "IN" : "OFF")),
                    keyRect.withY (keyRect.getY()+39*sy).withHeight (12*sy).toNearestInt(), juce::Justification::centred);
    }

    drawKnobGroup (g, R (16, 408, 768, 174).toNearestInt(), "TUNING ENGINE  •  FORMANTS  •  MASTER");
    g.setColour (muted);
    g.setFont (juce::FontOptions (8.0f*sx));
    g.drawText ("RETUNE & DYNAMICS", R (28, 428, 270, 12).toNearestInt(), juce::Justification::centred);
    g.drawText ("PITCH REFERENCE", R (306, 428, 188, 12).toNearestInt(), juce::Justification::centred);
    g.drawText ("MASTER OUTPUT", R (506, 428, 266, 12).toNearestInt(), juce::Justification::centred);
}

void LynxTunerAudioProcessorEditor::resized()
{
    const float sx = (float)getWidth()/800.0f, sy = (float)getHeight()/600.0f;
    auto set = [&] (juce::Component& c, float x, float y, float w, float h)
    { c.setBounds (juce::roundToInt(x*sx), juce::roundToInt(y*sy), juce::roundToInt(w*sx), juce::roundToInt(h*sy)); };
    set (header, 60, 9, 90, 29);
    set (subtitle, 60, 34, 300, 16);
    set (key, 400, 18, 70, 27);
    set (scale, 482, 18, 112, 27);
    set (formantPreserve, 610, 18, 170, 27);

    set (status, 34, 82, 380, 18);
    set (targetLabel, 560, 82, 205, 18);
    set (centsLabel, 330, 145, 140, 32);
    set (inputLabel, 36, 232, 300, 18);
    set (correctedLabel, 370, 232, 260, 18);
    set (depthLabel, 630, 232, 135, 18);

    set (speed, 34, 438, 92, 122);
    set (correction, 128, 438, 92, 122);
    set (smoothness, 222, 438, 92, 122);
    set (formantShift, 330, 438, 92, 122);
    set (reference, 422, 438, 92, 122);
    set (mix, 548, 438, 92, 122);
    set (outputGain, 646, 438, 92, 122);
}

void LynxTunerAudioProcessorEditor::timerCallback()
{
    const float sung = processor.engine().controlOutput (0) * 127.0f;
    const float target = processor.engine().controlOutput (1) * 127.0f;
    const float cents = (processor.engine().controlOutput (2) - 0.5f) * 400.0f;
    const bool active = sung > 0.0f;
    status.setText (active ? "●  PITCH TRACKING ACTIVE" : "●  AWAITING AUDIO INPUT",
                    juce::dontSendNotification);
    status.setColour (juce::Label::textColourId, active ? pink : muted);
    targetLabel.setText ("TARGET NOTE: " + (active ? noteName ((int)std::lround(target)) : "--"),
                         juce::dontSendNotification);
    centsLabel.setText (active ? juce::String (cents > 0 ? "+" : "") + juce::String (cents, 1) + "¢" : "0.0¢",
                        juce::dontSendNotification);
    centsLabel.setColour (juce::Label::textColourId, cents >= 0 ? amber : cyan);
    const float a4 = 430.0f + readParam (processor, lynx::lynxtune::reference) * 20.0f;
    const float inputHz = active ? (float)(a4 * std::pow (2.0, ((double)sung - 69.0) / 12.0)) : 0.0f;
    const float targetHz = active ? (float)(a4 * std::pow (2.0, ((double)target - 69.0) / 12.0)) : 0.0f;
    inputLabel.setText ("INPUT FREQ: " + (active ? juce::String (inputHz, 1) + " Hz" : "--"),
                        juce::dontSendNotification);
    correctedLabel.setText ("CORRECTED: " + (active ? juce::String (targetHz, 1) + " Hz" : "--"),
                            juce::dontSendNotification);
    depthLabel.setText ("CORRECTION " + juce::String (readParam (processor, lynx::lynxtune::correction) * 100.0f, 0) + "%",
                        juce::dontSendNotification);
    repaint();
}
