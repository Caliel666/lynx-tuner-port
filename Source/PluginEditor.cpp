#include "PluginEditor.h"

namespace
{
const juce::Colour bg        { 0xff171622 };
const juce::Colour panel     { 0xff1e1b2a };
const juce::Colour panelDark { 0xff11121a };
const juce::Colour accent    { 0xffb34d76 };
const juce::Colour accentLight { 0xffd16c91 };
const juce::Colour text      { 0xffe9e8ef };
const juce::Colour muted     { 0xff858491 };

void styleSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setColour (juce::Slider::rotarySliderFillColourId, accentLight);
    s.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff393744));
    s.setColour (juce::Slider::thumbColourId, text);
}

void styleCombo (juce::ComboBox& c)
{
    c.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff292b3b));
    c.setColour (juce::ComboBox::textColourId, text);
    c.setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    c.setColour (juce::ComboBox::arrowColourId, accentLight);
}

void styleToggle (juce::ToggleButton& b)
{
    b.setColour (juce::ToggleButton::textColourId, text);
    b.setColour (juce::ToggleButton::tickColourId, accentLight);
    b.setColour (juce::ToggleButton::tickDisabledColourId, muted);
}

void styleTextButton (juce::TextButton& b, bool toggleable = false)
{
    b.setColour (juce::TextButton::buttonColourId, panel);
    b.setColour (juce::TextButton::buttonOnColourId, accent);
    b.setColour (juce::TextButton::textColourOffId, text);
    b.setColour (juce::TextButton::textColourOnId, text);
    if (toggleable)
        b.setClickingTogglesState (true);
}
}

//==============================================================================
LynxTunerAudioProcessorEditor::LynxTunerAudioProcessorEditor (LynxTunerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setResizable (true, true);
    setResizeLimits (900, 700, 1800, 1400);
    setSize (1240, 965);

    auto& state = processor.parameters();

    auto addSlider = [&] (juce::Slider& s, int param)
    {
        styleSlider (s);
        addAndMakeVisible (s);
        sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "p" + juce::String (param), s));
    };

    auto addToggle = [&] (juce::ToggleButton& b, const juce::String& name, int param)
    {
        b.setButtonText (name);
        styleToggle (b);
        addAndMakeVisible (b);
        buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "p" + juce::String (param), b));
    };

    auto addCombo = [&] (juce::ComboBox& c, int param, std::initializer_list<const char*> items)
    {
        int id = 1;
        for (auto* item : items)
            c.addItem (item, id++);
        styleCombo (c);
        addAndMakeVisible (c);
        comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "p" + juce::String (param), c));
    };

    // ---- INPUT ----
    addSlider (tracking, lynx::lynxtune::tracking);
    addCombo (inputRange, lynx::lynxtune::range, { "Soprano", "Alto / Tenor", "Low male", "Instrument" });
    addCombo (key, lynx::lynxtune::key, { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" });
    addCombo (scale, lynx::lynxtune::scale,
              { "Chromatic", "Major", "Minor", "Harmonic minor", "Melodic minor",
                "Major pentatonic", "Minor pentatonic", "Blues", "Dorian", "Mixolydian",
                "Phrygian", "Lydian", "Locrian", "Custom" });

    // ---- MODE ----
    addToggle (lowLatency, "LOW LATENCY", lynx::lynxtune::lowLatency);
    addCombo (midiChannel, lynx::lynxtune::midiChannel,
              { "Off", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" });

    // ---- OUTPUT ----
    addToggle (formant, "FORMANT", lynx::lynxtune::formantPreserve);
    addSlider (formantShift, lynx::lynxtune::formantShift);
    addSlider (throat, lynx::lynxtune::throatLength);
    addSlider (transpose, lynx::lynxtune::transpose);
    addToggle (inScale, "IN SCALE", lynx::lynxtune::transposeInScale);
    addSlider (reference, lynx::lynxtune::reference);
    addSlider (mix, lynx::lynxtune::mix);

    // ---- CORRECTION ----
    addToggle (classic, "CLASSIC", lynx::lynxtune::classicMode);
    addToggle (learnScale, "LEARN SCALE", lynx::lynxtune::learnMidi);
    addSlider (retune, lynx::lynxtune::retune);
    addSlider (flexTune, lynx::lynxtune::flexTune);
    addSlider (naturalVibrato, lynx::lynxtune::naturalVibrato);
    addSlider (humanize, lynx::lynxtune::humanize);

    // ---- Tabs ----
    const char* tabNames[] = { "PITCH GRAPH", "ADVANCED", "REAL-TIME" };
    for (int i = 0; i < 3; ++i)
    {
        pages[(size_t) i].setButtonText (tabNames[i]);
        pages[(size_t) i].setClickingTogglesState (false);
        pages[(size_t) i].onClick = [this, i]
        {
            activePage = i;
            updatePageVisibility();
            resized();
            repaint();
        };
        styleTextButton (pages[(size_t) i]);
        addAndMakeVisible (pages[(size_t) i]);
    }

    // ---- REAL-TIME controls ----
    addToggle (speedLink, "LINK", lynx::lynxtune::speedLink);
    addSlider (speed, lynx::lynxtune::speed);
    addSlider (tolCents, lynx::lynxtune::toleranceCents);
    addSlider (tolTime, lynx::lynxtune::toleranceTime);
    addSlider (vibDepth, lynx::lynxtune::vibratoDepth); // reuse for real-time depth
    addSlider (correctionAmt, lynx::lynxtune::humanize); // placeholder – map if you have a dedicated param
    addToggle (refTone, "REF. TONE", lynx::lynxtune::referenceTone);
    addSlider (refToneLevel, lynx::lynxtune::referenceToneLevel);
    addToggle (vibratoEnable, "VIBRATO", lynx::lynxtune::ignoreVibrato); // inverted meaning is fine for UI

    // ---- ADVANCED controls ----
    vibratoShape.addItem ("No vibrato", 1);
    vibratoShape.addItem ("Sine", 2);
    vibratoShape.addItem ("Square", 3);
    vibratoShape.addItem ("Sawtooth", 4);
    styleCombo (vibratoShape);
    addAndMakeVisible (vibratoShape);
    // (attach later if you expose vibratoShape as a parameter)

    addToggle (ignoreVibrato, "IGNORE VIBRATO", lynx::lynxtune::ignoreVibrato);
    addSlider (vibRate, lynx::lynxtune::vibratoRate);
    addSlider (vibDepthAdv, lynx::lynxtune::vibratoDepth);
    addSlider (vibAmp, lynx::lynxtune::vibratoAmplitude);
    addSlider (vibFormant, lynx::lynxtune::vibratoFormant);
    addSlider (vibVar, lynx::lynxtune::vibratoVariation);
    addSlider (vibDelay, lynx::lynxtune::vibratoDelay);
    addSlider (vibOnset, lynx::lynxtune::vibratoOnset);

    addToggle (driveStage, "DRIVE STAGE", lynx::lynxtune::driveStage);
    addToggle (wideFormant, "FORMANT ±12 ST", lynx::lynxtune::wideFormant);
    addSlider (drive, lynx::lynxtune::drive);
    addSlider (outputTrim, lynx::lynxtune::outputTrim);

    // ---- TARGET NOTES toolbar ----
    bypassBtn.setButtonText ("Bypass");
    removeBtn.setButtonText ("Remove");
    resetBtn.setButtonText ("Reset");
    sendBtn.setButtonText ("Send");
    receiveBtn.setButtonText ("Receive");
    groupBtn.setButtonText ("Group");
    latchBtn.setButtonText ("Latch");

    styleTextButton (bypassBtn, true);
    styleTextButton (removeBtn, true);
    styleTextButton (resetBtn);
    styleTextButton (sendBtn);
    styleTextButton (receiveBtn, true);
    styleTextButton (groupBtn, true);
    styleTextButton (latchBtn, true);

    removeBtn.setToggleState (true, juce::dontSendNotification);
    removeMode = true;

    bypassBtn.onClick = [this]
    {
        removeMode = false;
        bypassBtn.setToggleState (true, juce::dontSendNotification);
        removeBtn.setToggleState (false, juce::dontSendNotification);
    };
    removeBtn.onClick = [this]
    {
        removeMode = true;
        removeBtn.setToggleState (true, juce::dontSendNotification);
        bypassBtn.setToggleState (false, juce::dontSendNotification);
    };
    resetBtn.onClick = [this]
    {
        auto& st = processor.parameters();
        for (int i = 0; i < 12; ++i)
            if (auto* p = st.getParameter ("p" + juce::String (lynx::lynxtune::noteC + i)))
                p->setValueNotifyingHost (1.0f);
    };

    addAndMakeVisible (bypassBtn);
    addAndMakeVisible (removeBtn);
    addAndMakeVisible (resetBtn);
    addAndMakeVisible (sendBtn);
    addAndMakeVisible (receiveBtn);
    addAndMakeVisible (groupBtn);
    addAndMakeVisible (latchBtn);

    // Range presets
    const char* rangeNames[] = { "Bass", "Baritone", "Tenor", "Alto", "Mezzo", "Soprano", "Generic" };
    for (int i = 0; i < 7; ++i)
    {
        rangeBtns[(size_t) i].setButtonText (rangeNames[i]);
        styleTextButton (rangeBtns[(size_t) i], true);
        rangeBtns[(size_t) i].onClick = [this, i]
        {
            const auto range = lynx::lynxtune::rangePresetNotes (i);
            auto& st = processor.parameters();
            if (auto* lo = st.getParameter ("p" + juce::String (lynx::lynxtune::rangeLow)))
                lo->setValueNotifyingHost (lynx::lynxtune::rangeNoteValue (range.first));
            if (auto* hi = st.getParameter ("p" + juce::String (lynx::lynxtune::rangeHigh)))
                hi->setValueNotifyingHost (lynx::lynxtune::rangeNoteValue (range.second));

            for (int j = 0; j < 7; ++j)
                rangeBtns[(size_t) j].setToggleState (j == i, juce::dontSendNotification);
        };
        addAndMakeVisible (rangeBtns[(size_t) i]);
    }
    rangeBtns[6].setToggleState (true, juce::dontSendNotification); // Generic default

    // 12 note switches
    static constexpr const char* noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i)
    {
        noteKeys[(size_t) i].setButtonText (noteNames[i]);
        noteKeys[(size_t) i].setClickingTogglesState (true);
        noteKeys[(size_t) i].setColour (juce::TextButton::buttonColourId, panel);
        noteKeys[(size_t) i].setColour (juce::TextButton::buttonOnColourId, accent);
        noteKeys[(size_t) i].setColour (juce::TextButton::textColourOffId, muted);
        noteKeys[(size_t) i].setColour (juce::TextButton::textColourOnId, text);
        buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "p" + juce::String (lynx::lynxtune::noteC + i), noteKeys[(size_t) i]));
        addAndMakeVisible (noteKeys[(size_t) i]);
    }

    title.setText ("LYNX TUNE", juce::dontSendNotification);
    title.setColour (juce::Label::textColourId, text);
    title.setFont (juce::FontOptions (18.0f).withStyle ("Bold"));
    addAndMakeVisible (title);

    updatePageVisibility();
    startTimerHz (30);
}

//==============================================================================
juce::String LynxTunerAudioProcessorEditor::midiNoteName (float normalised) const
{
    const int note = juce::jlimit (0, 127, (int) std::lround (normalised * 127.0f));
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return juce::String (names[note % 12]) + juce::String (note / 12 - 1);
}

void LynxTunerAudioProcessorEditor::updatePageVisibility()
{
    const bool mainPage = (activePage == 0);
    const bool adv      = (activePage == 1);
    const bool rt       = (activePage == 2);

    // Main correction knobs visible only on Pitch Graph page
    retune.setVisible (mainPage);
    flexTune.setVisible (mainPage);
    naturalVibrato.setVisible (mainPage);
    humanize.setVisible (mainPage);
    classic.setVisible (mainPage);
    learnScale.setVisible (mainPage);

    // Real-Time page
    speedLink.setVisible (rt);
    speed.setVisible (rt);
    tolCents.setVisible (rt);
    tolTime.setVisible (rt);
    vibDepth.setVisible (rt);
    correctionAmt.setVisible (rt);
    refTone.setVisible (rt);
    refToneLevel.setVisible (rt);
    vibratoEnable.setVisible (rt);

    // Advanced page
    vibratoShape.setVisible (adv);
    ignoreVibrato.setVisible (adv);
    vibRate.setVisible (adv);
    vibDepthAdv.setVisible (adv);
    vibAmp.setVisible (adv);
    vibFormant.setVisible (adv);
    vibVar.setVisible (adv);
    vibDelay.setVisible (adv);
    vibOnset.setVisible (adv);
    driveStage.setVisible (adv);
    wideFormant.setVisible (adv);
    drive.setVisible (adv);
    outputTrim.setVisible (adv);

    // Highlight active tab
    for (int i = 0; i < 3; ++i)
        pages[(size_t) i].setColour (juce::TextButton::buttonColourId,
                                     i == activePage ? accent : panel);
}

void LynxTunerAudioProcessorEditor::timerCallback()
{
    // Learn-scale feedback
    if (learnScale.getToggleState())
    {
        const int mask = juce::jlimit (0, 4095, (int) std::lround (processor.engine().controlOutput (5) * 4095.0f));
        if (mask != 0 && mask != lastLearnedMask)
        {
            auto& st = processor.parameters();
            for (int i = 0; i < 12; ++i)
            {
                const bool enabled = (mask & (1 << i)) != 0;
                if (auto* param = st.getParameter ("p" + juce::String (lynx::lynxtune::noteC + i)))
                    param->setValueNotifyingHost (enabled ? 1.0f : 0.0f);
                noteKeys[(size_t) i].setToggleState (enabled, juce::dontSendNotification);
            }
            lastLearnedMask = mask;
        }
    }

    // Pitch history for the graph
    const float sung = processor.engine().controlOutput (0);
    pitchHistory[(size_t) pitchHistoryWrite] = sung;
    pitchHistoryWrite = (pitchHistoryWrite + 1) % (int) pitchHistory.size();

    repaint();
}

//==============================================================================
void LynxTunerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);

    const float sx = (float) getWidth()  / 1240.0f;
    const float sy = (float) getHeight() / 965.0f;

    auto R = [&] (float x, float y, float w, float h)
    {
        return juce::Rectangle<float> (x * sx, y * sy, w * sx, h * sy);
    };

    auto section = [&] (juce::Rectangle<float> r, const juce::String& name)
    {
        g.setColour (panel);
        g.fillRoundedRectangle (r, 12.0f * sx);
        g.setColour (accent);
        g.fillRoundedRectangle (r.withHeight (4.0f * sy), 2.0f * sx);
        if (name.isNotEmpty())
        {
            g.setColour (text);
            g.setFont (juce::FontOptions (11.0f * sx).withStyle ("Bold"));
            g.drawFittedText (name, r.removeFromTop (22.0f * sy).toNearestInt(), juce::Justification::centred, 1);
        }
    };

    // Top bar
    g.setColour (panelDark);
    g.fillRect (R (0, 0, 1240, 52));
    g.setColour (text);
    g.setFont (juce::FontOptions (17.0f * sx).withStyle ("Bold"));
    g.drawText ("●  LYNX TUNE", R (20, 10, 260, 32).toNearestInt(), juce::Justification::centredLeft);

    // Sections
    section (R (8, 58, 510, 140), "INPUT");
    section (R (526, 58, 210, 140), "MODE");
    section (R (744, 58, 488, 140), "OUTPUT");
    section (R (8, 206, 1224, 370), "CORRECTION");
    section (R (8, 584, 1224, 220), "");          // tab content area
    section (R (8, 812, 1224, 145), "TARGET NOTES");

    // Value labels under main knobs (Pitch Graph page)
    if (activePage == 0)
    {
        auto valueText = [&] (const juce::Slider& s, int param, juce::Rectangle<float> bounds)
        {
            g.setColour (muted);
            g.setFont (juce::FontOptions (10.5f * sx));
            g.drawFittedText (lynx::lynxtune::paramText (param, (float) s.getValue()),
                              bounds.toNearestInt(), juce::Justification::centred, 1);
        };

        valueText (tracking,       lynx::lynxtune::tracking,       R (20, 185, 80, 16));
        valueText (retune,         lynx::lynxtune::retune,         R (80, 545, 160, 16));
        valueText (flexTune,       lynx::lynxtune::flexTune,       R (280, 530, 140, 16));
        valueText (naturalVibrato, lynx::lynxtune::naturalVibrato, R (810, 530, 140, 16));
        valueText (humanize,       lynx::lynxtune::humanize,       R (1000, 545, 160, 16));
        valueText (formantShift,   lynx::lynxtune::formantShift,   R (760, 185, 80, 14));
        valueText (throat,         lynx::lynxtune::throatLength,   R (840, 185, 80, 14));
        valueText (transpose,      lynx::lynxtune::transpose,      R (920, 185, 90, 14));
        valueText (reference,      lynx::lynxtune::reference,      R (1055, 185, 80, 14));
        valueText (mix,            lynx::lynxtune::mix,            R (1145, 185, 70, 14));
    }

    paintCorrectionDial (g, sx, sy);

    if (activePage == 0)
        paintPitchGraph (g, sx, sy);
    else if (activePage == 1)
    {
        // Advanced page captions
        g.setColour (text);
        g.setFont (juce::FontOptions (12.0f * sx).withStyle ("Bold"));
        g.drawText ("CREATE VIBRATO", R (30, 610, 200, 20).toNearestInt(), juce::Justification::centredLeft);
        g.drawText ("VOICE SHIFTER",  R (700, 610, 200, 20).toNearestInt(), juce::Justification::centredLeft);
    }
    else if (activePage == 2)
    {
        g.setColour (text);
        g.setFont (juce::FontOptions (12.0f * sx).withStyle ("Bold"));
        g.drawText ("SPEED: IN-NOTE DRIFT AND VIBRATO", R (30, 610, 400, 20).toNearestInt(), juce::Justification::centredLeft);
        g.drawText ("TOLERANCE: HOW FAR / HOW LONG A NOTE", R (30, 700, 400, 20).toNearestInt(), juce::Justification::centredLeft);
    }

    paintKeyboard (g, sx, sy);
}


void LynxTunerAudioProcessorEditor::paintCorrectionDial (juce::Graphics& g, float sx, float sy)
{
    const auto bounds = juce::Rectangle<float> (469.0f*sx, 246.0f*sy, 314.0f*sx, 314.0f*sy);
    const auto centre = bounds.getCentre();
    const float radius = bounds.getWidth() * 0.5f;
    const float sung = processor.engine().controlOutput (0);
    const float target = processor.engine().controlOutput (1);
    const float cents = (processor.engine().controlOutput (2) - 0.5f) * 400.0f;
    const bool voiced = sung > 0.0f;

    g.setColour (juce::Colour (0xff08090e));
    g.fillEllipse (bounds);
    g.setColour (juce::Colour (0xff343541));
    g.drawEllipse (bounds.reduced (7.0f*sx), 4.0f*sx);

    for (int i = 0; i < 50; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * (float)i / 50.0f
                          - juce::MathConstants<float>::halfPi;
        const float inner = radius * 0.87f;
        const float outer = radius * 0.97f;
        g.setColour (juce::Colour (0xff444650));
        g.drawLine (centre.x + std::cos (angle)*inner, centre.y + std::sin (angle)*inner,
                    centre.x + std::cos (angle)*outer, centre.y + std::sin (angle)*outer, 4.0f*sx);
    }

    if (voiced)
    {
        const float angle = juce::MathConstants<float>::twoPi
                          * juce::jlimit (-1.0f, 1.0f, cents / 100.0f) * 0.25f
                          - juce::MathConstants<float>::halfPi;
        g.setColour (accentLight);
        g.drawLine (centre.x + std::cos (angle)*radius*0.78f,
                    centre.y + std::sin (angle)*radius*0.78f,
                    centre.x + std::cos (angle)*radius*0.98f,
                    centre.y + std::sin (angle)*radius*0.98f, 5.0f*sx);
    }

    g.setColour (text);
    g.setFont (juce::FontOptions (25.0f*sx).withStyle ("Bold"));
    g.drawText (voiced ? midiNoteName (target) : "--",
                juce::Rectangle<int> (juce::roundToInt (centre.x - 55*sx),
                                      juce::roundToInt (centre.y - 28*sy),
                                      juce::roundToInt (110*sx), juce::roundToInt (38*sy)),
                juce::Justification::centred);
    g.setFont (juce::FontOptions (12.0f*sx));
    g.drawText (voiced ? juce::String (cents, 1) + " cents" : "no pitch",
                juce::Rectangle<int> (juce::roundToInt (centre.x - 65*sx),
                                      juce::roundToInt (centre.y + 12*sy),
                                      juce::roundToInt (130*sx), juce::roundToInt (22*sy)),
                juce::Justification::centred);
    g.setColour (muted);
    g.drawText ("HOLD",
                juce::Rectangle<int> (juce::roundToInt (centre.x - 45*sx),
                                      juce::roundToInt (centre.y + 48*sy),
                                      juce::roundToInt (90*sx), juce::roundToInt (18*sy)),
                juce::Justification::centred);
}

void LynxTunerAudioProcessorEditor::paintPitchGraph (juce::Graphics& g, float sx, float sy)
{
    const juce::Rectangle<float> graph (10.0f*sx, 637.0f*sy, 1220.0f*sx, 165.0f*sy);
    g.setColour (panelDark);
    g.fillRoundedRectangle (graph, 8.0f*sx);

    constexpr int lowestMidi = 36;
    constexpr int highestMidi = 84;
    const float laneHeight = graph.getHeight() / (float)(highestMidi - lowestMidi + 1);
    g.setFont (juce::FontOptions (9.0f*sx));
    for (int note = lowestMidi; note <= highestMidi; ++note)
    {
        const float y = graph.getY() + (float)(highestMidi - note) * laneHeight;
        g.setColour (juce::Colour (0xff41404d));
        g.drawHorizontalLine (juce::roundToInt (y), graph.getX()+10.0f*sx, graph.getRight()-8.0f*sx);
        if (note % 12 == 0)
        {
            g.setColour (muted);
            g.drawText (midiNoteName ((float)note / 127.0f),
                        juce::Rectangle<int> (juce::roundToInt (graph.getX()+6*sx),
                                              juce::roundToInt (y),
                                              juce::roundToInt (32*sx),
                                              juce::roundToInt (laneHeight+2*sx)),
                        juce::Justification::centredLeft);
        }
    }

    juce::Path trace;
    bool started = false;
    for (int i = 0; i < (int)pitchHistory.size(); ++i)
    {
        const int index = (pitchHistoryWrite + i) % (int)pitchHistory.size();
        const float pitch = pitchHistory[(size_t)index];
        if (pitch <= 0.0f) { started = false; continue; }
        const float midi = pitch * 127.0f;
        const float y = graph.getBottom() - (midi - (float)lowestMidi)
                      / (float)(highestMidi - lowestMidi) * graph.getHeight();
        const float x = graph.getX() + 42.0f*sx
                      + (float)i * (graph.getWidth()-52.0f*sx) / (float)(pitchHistory.size()-1);
        if (!started) { trace.startNewSubPath (x, y); started = true; }
        else trace.lineTo (x, y);
    }
    g.setColour (accentLight);
    g.strokePath (trace, juce::PathStrokeType (2.0f*sx));
}

void LynxTunerAudioProcessorEditor::paintKeyboard (juce::Graphics& g, float sx, float sy)
{
    const juce::Rectangle<float> bed (16.0f*sx, 909.0f*sy, 1218.0f*sx, 54.0f*sy);
    g.setColour (accent);
    g.fillRoundedRectangle (juce::Rectangle<float> (16.0f*sx, 893.0f*sy, 1218.0f*sx, 12.0f*sy), 3.0f*sx);
    g.setColour (juce::Colour (0xffd9dbe0));
    g.fillRoundedRectangle (bed, 4.0f*sx);

    constexpr int whiteCount = 28;
    const float whiteWidth = bed.getWidth() / (float)whiteCount;
    static constexpr int whiteSemitones[7] = { 0, 2, 4, 5, 7, 9, 11 };
    for (int i = 0; i < whiteCount; ++i)
    {
        const int note = 48 + (i/7)*12 + whiteSemitones[i%7];
        const bool enabled = noteKeys[(size_t)(note%12)].getToggleState();
        const float x = bed.getX() + (float)i*whiteWidth;
        g.setColour (enabled ? juce::Colour (0xfff2c8d8) : juce::Colour (0xffd9dbe0));
        g.fillRect (juce::Rectangle<float> (x+0.5f*sx, bed.getY()+0.5f*sy,
                                            whiteWidth-sx, bed.getHeight()-sy));
        g.setColour (juce::Colour (0xff343640));
        g.drawRect (juce::Rectangle<float> (x, bed.getY(), whiteWidth, bed.getHeight()), 0.8f*sx);
        if (note % 12 == 0)
        {
            g.setColour (juce::Colour (0xff30313a));
            g.setFont (juce::FontOptions (9.0f*sx));
            g.drawText (midiNoteName ((float)note/127.0f),
                        juce::Rectangle<int> (juce::roundToInt (x+3*sx),
                                              juce::roundToInt (bed.getBottom()-14*sy),
                                              juce::roundToInt (whiteWidth-6*sx),
                                              juce::roundToInt (11*sy)),
                        juce::Justification::centredLeft);
        }
    }

    static constexpr bool hasBlack[12] = { false,true,false,true,false,false,true,false,true,false,true,false };
    for (int i = 0; i < whiteCount-1; ++i)
    {
        const int note = 48 + (i/7)*12 + whiteSemitones[i%7];
        const int next = 48 + ((i+1)/7)*12 + whiteSemitones[(i+1)%7];
        if (next-note != 2) continue;
        const int blackNote = note+1;
        const float x = bed.getX() + ((float)i+0.68f)*whiteWidth;
        const float w = whiteWidth*0.62f;
        g.setColour (noteKeys[(size_t)(blackNote%12)].getToggleState()
                         ? juce::Colour (0xffa84a70) : juce::Colour (0xff17181f));
        g.fillRoundedRectangle (juce::Rectangle<float> (x, bed.getY(), w, bed.getHeight()*0.60f), 2.0f*sx);
    }
}

void LynxTunerAudioProcessorEditor::handleKeyboardClick (float x, float y)
{
    const float sx = (float)getWidth() / 1240.0f;
    const float sy = (float)getHeight() / 965.0f;
    const float px = x / sx;
    const float py = y / sy;
    if (py < 909.0f || py > 963.0f || px < 16.0f || px > 1234.0f)
        return;

    const float keyWidth = 1218.0f / 28.0f;
    const int whiteIndex = juce::jlimit (0, 27, (int)((px-16.0f)/keyWidth));
    static constexpr int whiteSemitones[7] = { 0, 2, 4, 5, 7, 9, 11 };
    int note = 48 + (whiteIndex/7)*12 + whiteSemitones[whiteIndex%7];

    // Detect black-key area and map it to the adjacent semitone.
    const float within = std::fmod (px-16.0f, keyWidth);
    if (py < 941.0f && within > keyWidth*0.68f && whiteIndex < 27)
    {
        const int next = 48 + ((whiteIndex+1)/7)*12 + whiteSemitones[(whiteIndex+1)%7];
        if (next-note == 2) ++note;
    }

    auto& button = noteKeys[(size_t)(note%12)];
    button.setToggleState (!button.getToggleState(), juce::sendNotification);
}

void LynxTunerAudioProcessorEditor::resized()
{
    const float sx = (float)getWidth() / 1240.0f;
    const float sy = (float)getHeight() / 965.0f;
    auto set = [&] (juce::Component& component, float x, float y, float w, float h)
    {
        component.setBounds (juce::roundToInt (x*sx), juce::roundToInt (y*sy),
                             juce::roundToInt (w*sx), juce::roundToInt (h*sy));
    };

    set (title, 84, 10, 190, 34);
    set (tracking, 22, 105, 70, 88);
    set (inputRange, 104, 126, 166, 35);
    set (key, 279, 126, 81, 35);
    set (scale, 370, 126, 143, 35);
    set (lowLatency, 542, 102, 190, 32);
    set (midiChannel, 542, 166, 190, 30);
    set (formant, 765, 102, 140, 30);
    set (formantShift, 772, 132, 70, 70);
    set (throat, 842, 132, 70, 70);
    set (transpose, 915, 133, 110, 60);
    set (inScale, 913, 177, 120, 25);
    set (reference, 1060, 104, 90, 94);
    set (mix, 1152, 104, 80, 94);

    set (classic, 18, 244, 130, 30);
    set (learnScale, 1072, 244, 150, 30);
    set (retune, 48, 295, 210, 260);
    set (flexTune, 280, 363, 150, 170);
    set (naturalVibrato, 810, 363, 150, 170);
    set (humanize, 982, 295, 210, 260);

    for (int i = 0; i < 3; ++i)
        set (pages[(size_t)i], 16+i*170, 596, 168, 32);
    for (int i = 0; i < 12; ++i)
        set (noteKeys[(size_t)i], 18+i*100, 853, 94, 30);

    const bool mainPage = activePage == 0;
    const bool adv = activePage == 1;
    const bool rt = activePage == 2;
    for (auto* component : { (juce::Component*)&retune, (juce::Component*)&flexTune,
                             (juce::Component*)&naturalVibrato, (juce::Component*)&humanize,
                             (juce::Component*)&classic, (juce::Component*)&learnScale })
        component->setVisible (mainPage);

    auto setPageControl = [&] (juce::Component& component, float x, float y, float w, float h, bool visible)
    {
        set (component, x, y, w, h);
        component.setVisible (visible);
    };
    setPageControl (speedLink, 30, 625, 100, 28, rt);
    setPageControl (speed, 140, 620, 120, 150, rt);
    setPageControl (tolCents, 290, 620, 120, 150, rt);
    setPageControl (tolTime, 440, 620, 120, 150, rt);
    setPageControl (vibDepth, 590, 620, 120, 150, rt);
    setPageControl (correctionAmt, 740, 620, 120, 150, rt);
    setPageControl (refTone, 880, 625, 110, 28, rt);
    setPageControl (refToneLevel, 1000, 620, 120, 150, rt);
    setPageControl (vibratoEnable, 30, 775, 130, 26, rt);

    setPageControl (vibratoShape, 30, 625, 180, 30, adv);
    setPageControl (ignoreVibrato, 230, 625, 180, 28, adv);
    setPageControl (vibRate, 30, 660, 110, 130, adv);
    setPageControl (vibDepthAdv, 150, 660, 110, 130, adv);
    setPageControl (vibAmp, 270, 660, 110, 130, adv);
    setPageControl (vibFormant, 390, 660, 110, 130, adv);
    setPageControl (vibVar, 510, 660, 110, 130, adv);
    setPageControl (vibDelay, 630, 660, 110, 130, adv);
    setPageControl (vibOnset, 750, 660, 110, 130, adv);
    setPageControl (driveStage, 880, 625, 150, 28, adv);
    setPageControl (wideFormant, 1030, 625, 170, 28, adv);
    setPageControl (drive, 900, 660, 110, 130, adv);
    setPageControl (outputTrim, 1030, 660, 110, 130, adv);
}

void LynxTunerAudioProcessorEditor::mouseDown (const juce::MouseEvent& event)
{
    handleKeyboardClick (event.position.x, event.position.y);
}

void LynxTunerAudioProcessorEditor::mouseDrag (const juce::MouseEvent& event)
{
    handleKeyboardClick (event.position.x, event.position.y);
}
