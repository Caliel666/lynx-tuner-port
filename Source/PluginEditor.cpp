#include "PluginEditor.h"

namespace {
const juce::Colour bg{0xff171622}, panel{0xff1e1b2a}, panelDark{0xff11121a};
const juce::Colour accent{0xffb34d76}, accentLight{0xffd16c91}, text{0xffe9e8ef}, muted{0xff858491};
void styleSlider(juce::Slider& s) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 74, 18);
    s.setColour(juce::Slider::rotarySliderFillColourId, accentLight);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff393744));
    s.setColour(juce::Slider::thumbColourId, text);
    s.setColour(juce::Slider::textBoxTextColourId, text);
    s.setColour(juce::Slider::textBoxBackgroundColourId, panel);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}
void styleCombo(juce::ComboBox& c) {
    c.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff292b3b));
    c.setColour(juce::ComboBox::textColourId, text);
    c.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    c.setColour(juce::ComboBox::arrowColourId, accentLight);
}
void styleToggle(juce::ToggleButton& b) {
    b.setColour(juce::ToggleButton::textColourId, text);
    b.setColour(juce::ToggleButton::tickColourId, accentLight);
    b.setColour(juce::ToggleButton::tickDisabledColourId, muted);
}
}

LynxTunerAudioProcessorEditor::LynxTunerAudioProcessorEditor(LynxTunerAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p) {
    setResizable(true, true);
    setResizeLimits(800, 622, 1600, 1244);
    setSize(1240, 965);
    auto& state = processor.parameters();

    auto addSlider = [&](juce::Slider& s, int param) {
        styleSlider(s); s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); addAndMakeVisible(s);
        sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "p" + juce::String(param), s));
    };
    auto addToggle = [&](juce::ToggleButton& b, const juce::String& name, int param) {
        b.setButtonText(name); styleToggle(b); addAndMakeVisible(b);
        buttonAttachments.push_back(std::make_unique<ButtonAttachment>(state, "p" + juce::String(param), b));
    };
    auto addCombo = [&](juce::ComboBox& c, int param, std::initializer_list<const char*> items, int count) {
        int id=1; for (auto* item: items) c.addItem(item,id++);
        styleCombo(c); addAndMakeVisible(c);
        comboAttachments.push_back(std::make_unique<ComboAttachment>(state, "p" + juce::String(param), c));
    };

    addSlider(tracking,12); addSlider(retune,2); addSlider(flexTune,25);
    addSlider(naturalVibrato,26); addSlider(humanize,3); addSlider(formantShift,5);
    addSlider(throat,89); addSlider(transpose,6); addSlider(reference,7); addSlider(mix,11);
    addCombo(inputRange,8,{"Soprano","Alto / Tenor","Low male","Instrument"},4);
    voicePreset.addItem("Bass",1); voicePreset.addItem("Baritone",2); voicePreset.addItem("Tenor",3);
    voicePreset.addItem("Alto",4); voicePreset.addItem("Mezzo",5); voicePreset.addItem("Soprano",6); voicePreset.addItem("Generic",7);
    styleCombo(voicePreset); addAndMakeVisible(voicePreset);
    voicePreset.setSelectedId(7,juce::dontSendNotification);
    voicePreset.onChange=[this] {
        const int idx=juce::jlimit(0,6,voicePreset.getSelectedId()-1);
        const auto range=lynx::lynxtune::rangePresetNotes(idx);
        auto& state=processor.parameters();
        if(auto* lo=state.getParameter("p"+juce::String(lynx::lynxtune::rangeLow))) lo->setValueNotifyingHost(lynx::lynxtune::rangeNoteValue(range.first));
        if(auto* hi=state.getParameter("p"+juce::String(lynx::lynxtune::rangeHigh))) hi->setValueNotifyingHost(lynx::lynxtune::rangeNoteValue(range.second));
    };
    addCombo(key,0,{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},12);
    addCombo(scale,1,{"Chromatic","Major","Minor","Harmonic minor","Melodic minor","Major pentatonic","Minor pentatonic","Blues","Dorian","Mixolydian","Phrygian","Lydian","Locrian","Custom"},14);
    addCombo(midiChannel,10,{"Off","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},17);
    addToggle(lowLatency,"LOW LATENCY",lynx::lynxtune::lowLatency); addToggle(formant,"FORMANT",lynx::lynxtune::formantPreserve);
    learnScale.onClick=[this] {
        if (learnScale.getToggleState() && midiChannel.getSelectedId() == 1) {
            midiChannel.setSelectedId(2,juce::sendNotification);
        }
    };
    addToggle(classic,"CLASSIC",lynx::lynxtune::classicMode); addToggle(learnScale,"LEARN SCALE",lynx::lynxtune::learnMidi);
    addToggle(inScale,"IN SCALE",93);

    const char* tabNames[]={"PITCH GRAPH","ADVANCED","REAL-TIME"};
    for (int i=0;i<3;++i) {
        pages[(size_t)i].setButtonText(tabNames[i]);
        pages[(size_t)i].setClickingTogglesState(false);
        pages[(size_t)i].onClick=[this,i]{ activePage=i; resized(); repaint(); };
        addAndMakeVisible(pages[(size_t)i]);
    }
    static constexpr const char* noteNames[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    for (int i=0;i<12;++i) {
        noteKeys[(size_t)i].setButtonText(noteNames[i]);
        buttonAttachments.push_back(std::make_unique<ButtonAttachment>(state, "p" + juce::String(13+i), noteKeys[(size_t)i]));
        noteKeys[(size_t)i].setClickingTogglesState(true);
        noteKeys[(size_t)i].setColour(juce::TextButton::buttonColourId, panel);
        noteKeys[(size_t)i].setColour(juce::TextButton::buttonOnColourId, accent);
        addAndMakeVisible(noteKeys[(size_t)i]);
    }
    title.setText("LYNX TUNE", juce::dontSendNotification);
    title.setColour(juce::Label::textColourId,text);
    title.setFont(juce::Font(18.0f,juce::Font::bold));
    addAndMakeVisible(title);
    startTimerHz(30);
}

juce::String LynxTunerAudioProcessorEditor::midiNoteName(float normalized) const {
    const int note = juce::jlimit(0, 127, (int)std::lround(normalized * 127.0f));
    static const char* names[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    return juce::String(names[note % 12]) + juce::String(note / 12 - 1);
}

void LynxTunerAudioProcessorEditor::timerCallback() {
    if (learnScale.getToggleState()) {
        const int mask=juce::jlimit(0,4095,(int)std::lround(processor.engine().controlOutput(5)*4095.0f));
        if (mask!=0 && mask!=lastLearnedMask) {
            auto& state=processor.parameters();
            for(int i=0;i<12;++i) {
                const bool enabled=(mask&(1<<i))!=0;
                if(auto* param=state.getParameter("p"+juce::String(lynx::lynxtune::noteC+i)))
                    param->setValueNotifyingHost(enabled?1.0f:0.0f);
                noteKeys[(size_t)i].setToggleState(enabled,juce::dontSendNotification);
            }
            lastLearnedMask=mask;
        }
    }
    repaint();
}

void LynxTunerAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(bg);
    const float sx=(float)getWidth()/1240.0f, sy=(float)getHeight()/965.0f;
    auto R=[&](float x,float y,float w,float h){return juce::Rectangle<float>(x*sx,y*sy,w*sx,h*sy);};
    auto section=[&](juce::Rectangle<float> r,const juce::String& name){
        g.setColour(panel); g.fillRoundedRectangle(r,15.0f*sx);
        g.setColour(accent); g.fillRoundedRectangle(r.withHeight(5.0f*sy),3.0f*sx);
        g.setColour(text); g.setFont(juce::Font(12.0f*sx,juce::Font::bold));
        g.drawFittedText(name,r.removeFromTop(25.0f*sy).toNearestInt(),juce::Justification::centred,1);
    };
    g.setColour(panelDark); g.fillRect(R(0,0,1240,59));
    g.setColour(text); g.setFont(juce::Font(18.0f*sx,juce::Font::bold)); g.drawText("●  LYNX TUNE",R(28,12,230,34).toNearestInt(),juce::Justification::centredLeft);
    section(R(0,62,523,146),"INPUT");
    section(R(528,62,220,146),"MODE");
    section(R(754,62,486,146),"OUTPUT");
    section(R(0,214,1240,373),"CORRECTION");
    section(R(0,592,1240,220),"");
    section(R(0,818,1240,147),"TARGET NOTES");

    // Original LYNX Tune correction dial: segmented ±100-cent ring and central pitch readout.
    const auto c=R(469,246,314,314).getCentre(); const float rad=R(469,246,314,314).getWidth()*0.5f;
    g.setColour(juce::Colour(0xff08090e)); g.fillEllipse(R(469,246,314,314));
    g.setColour(juce::Colour(0xff343541)); g.drawEllipse(R(477,254,298,298),5.0f*sx);
    for(int i=0;i<50;++i){
        const float a=juce::MathConstants<float>::twoPi*(float)i/50.0f-juce::MathConstants<float>::halfPi;
        const float inner=rad*0.88f, outer=rad*0.98f;
        g.setColour(juce::Colour(0xff444650));
        g.drawLine(c.x+std::cos(a)*inner,c.y+std::sin(a)*inner,c.x+std::cos(a)*outer,c.y+std::sin(a)*outer,4.0f*sx);
    }
    const float sung = processor.engine().controlOutput(0);
    const float target = processor.engine().controlOutput(1);
    const float correction = (processor.engine().controlOutput(2) - 0.5f) * 8.0f;
    const bool voiced = sung > 0.0f;
    if (voiced) {
        const float a = juce::MathConstants<float>::twoPi * juce::jlimit(-1.0f,1.0f,correction/100.0f) * 0.25f - juce::MathConstants<float>::halfPi;
        g.setColour(accentLight);
        g.drawLine(c.x+std::cos(a)*rad*0.82f,c.y+std::sin(a)*rad*0.82f,c.x+std::cos(a)*rad*0.98f,c.y+std::sin(a)*rad*0.98f,5.0f*sx);
    }
    g.setColour(text); g.setFont(juce::Font(25.0f*sx,juce::Font::bold));
    g.drawText(voiced ? midiNoteName(target) : "--",R(580,374,92,38).toNearestInt(),juce::Justification::centred);
    g.setFont(juce::Font(12.0f*sx));
    g.drawText(voiced ? juce::String(correction,1) + " cents" : "no pitch",R(570,414,112,24).toNearestInt(),juce::Justification::centred);
    g.drawText("HOLD",R(580,450,92,22).toNearestInt(),juce::Justification::centred);

    // Pitch graph panel and octave keyboard bed.
    g.setColour(panelDark); g.fillRoundedRectangle(R(10,637,1220,165),8.0f*sx);
    for(int i=0;i<14;++i){float y=(650.0f+i*10.5f)*sy;g.setColour(juce::Colour(0xff41404d));g.drawHorizontalLine((int)y,20*sx,1220*sx);}
    g.setColour(muted); g.setFont(juce::Font(10.0f*sx));
    for (int n=0;n<14;++n) {
        static const char* names[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        const int midi=48+n;
        g.drawText(juce::String(names[midi%12])+juce::String(midi/12-1),R(18,642+n*10.5f,38,12).toNearestInt(),juce::Justification::centredLeft);
    }
    if (voiced) {
        const float y = 790.0f - juce::jlimit(0.0f,1.0f,sung)*128.0f;
        g.setColour(accentLight); g.fillEllipse(R(1180,y,7,7));
        g.setColour(text); g.drawText(midiNoteName(sung),R(1080,y-18,90,16).toNearestInt(),juce::Justification::centredRight);
    } else {
        g.setColour(muted); g.drawText("Play a voice or an instrument through the insert to see its pitch",R(330,705,580,26).toNearestInt(),juce::Justification::centred);
    }
    g.setColour(accent);g.fillRoundedRectangle(R(16,893,1218,12),3.0f*sx);
    auto bed=R(16,909,1218,114); g.setColour(juce::Colour(0xffd9dbe0));g.fillRoundedRectangle(bed,4.0f*sx);
    for(int i=0;i<28;++i){float x=bed.getX()+bed.getWidth()*(float)i/28.0f;g.setColour(juce::Colour(0xff363640));g.drawVerticalLine((int)x,bed.getY(),bed.getBottom());}
    for(int i=0;i<28;++i) if(i%7!=2 && i%7!=6){
        float x=bed.getX()+bed.getWidth()*((float)i+0.68f)/28.0f;
        g.setColour(juce::Colour(0xff17181f));g.fillRoundedRectangle(x,bed.getY(),bed.getWidth()/28.0f*0.62f,bed.getHeight()*0.60f,2.0f*sx);
    }
}

void LynxTunerAudioProcessorEditor::resized() {
    const float sx=(float)getWidth()/1240.0f, sy=(float)getHeight()/965.0f;
    auto set=[&](juce::Component& c,float x,float y,float w,float h){c.setBounds(juce::roundToInt(x*sx),juce::roundToInt(y*sy),juce::roundToInt(w*sx),juce::roundToInt(h*sy));};
    set(title,84,10,190,34);
    set(tracking,22,105,70,88); set(inputRange,104,126,166,35); set(key,279,126,81,35); set(scale,370,126,143,35);
    set(lowLatency,542,102,190,32); set(midiChannel,542,166,190,30);
    set(formant,765,102,140,30); set(formantShift,772,132,70,70); set(throat,842,132,70,70);
    set(transpose,915,133,110,60); set(inScale,913,177,120,25); set(reference,1060,104,90,94); set(mix,1152,104,80,94);
    set(classic,18,244,130,30); set(learnScale,1072,244,150,30);
    set(retune,48,295,210,260); set(flexTune,280,363,150,170); set(naturalVibrato,810,363,150,170); set(humanize,982,295,210,260);
    set(voicePreset,18,820,190,28);\n    for(int i=0;i<3;++i) set(pages[(size_t)i],16+i*170,596,168,32);
    for(int i=0;i<12;++i) set(noteKeys[(size_t)i],18+i*100,853,94,30);
    const bool adv=activePage==1, rt=activePage==2;
    for(auto* c:{(juce::Component*)&retune,(juce::Component*)&flexTune,(juce::Component*)&naturalVibrato,(juce::Component*)&humanize})
        c->setVisible(!adv&&!rt);
}

void LynxTunerAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    const float sx=(float)getWidth()/1240.0f, sy=(float)getHeight()/965.0f;
    const float x=e.position.x/sx, y=e.position.y/sy;
    if (y<909.0f || y>1023.0f || x<16.0f || x>1234.0f) return;
    const float keyWidth=1218.0f/28.0f;
    int white=(int)((x-16.0f)/keyWidth);
    white=juce::jlimit(0,27,white);
    static constexpr int semis[7]={0,2,4,5,7,9,11};
    const int note=48+(white/7)*12+semis[white%7];
    const int pc=note%12;
    noteKeys[(size_t)pc].setToggleState(!noteKeys[(size_t)pc].getToggleState(),juce::sendNotification);
}
