#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SpectrumAnalyzer.h"

DX10Program::DX10Program(const char *name,
                         float p0,  float p1,  float p2,  float p3,
                         float p4,  float p5,  float p6,  float p7,
                         float p8,  float p9,  float p10, float p11,
                         float p12, float p13, float p14, float p15)
{
    strcpy(this->name, name);
    param[0]  = p0;  param[1]  = p1;  param[2]  = p2;  param[3]  = p3;
    param[4]  = p4;  param[5]  = p5;  param[6]  = p6;  param[7]  = p7;
    param[8]  = p8;  param[9]  = p9;  param[10] = p10; param[11] = p11;
    param[12] = p12; param[13] = p13; param[14] = p14; param[15] = p15;
}

DX10AudioProcessor::DX10AudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    _sampleRate = 44100.0f;
    _inverseSampleRate = 1.0f / _sampleRate;
    createPrograms();
    _currentProgram = 0;
}

DX10AudioProcessor::~DX10AudioProcessor() {}

const juce::String DX10AudioProcessor::getName() const { return JucePlugin_Name; }
int DX10AudioProcessor::getNumPrograms() { return int(_programs.size()); }

int DX10AudioProcessor::getCurrentProgram()
{
    if (auto* param = apvts.getRawParameterValue("PresetIndex"))
        return static_cast<int>(param->load() * (NPRESETS - 1) + 0.5f);
    return _currentProgram;
}

void DX10AudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= static_cast<int>(_programs.size())) return;
    _currentProgram = index;
    if (auto* param = apvts.getParameter("PresetIndex"))
        param->setValueNotifyingHost(static_cast<float>(index) / static_cast<float>(NPRESETS - 1));

    const char *paramNames[] = {"Attack","Decay","Release","Coarse","Fine","Mod Init","Mod Dec","Mod Sus","Mod Rel","Mod Vel","Vibrato","Octave","FineTune","Waveform","Mod Thru","LFO Rate"};
    for (int i = 0; i < NPARAMS; ++i)
        apvts.getParameter(paramNames[i])->setValueNotifyingHost(_programs[index].param[i]);
}

const juce::String DX10AudioProcessor::getProgramName(int index)
{
    if (index >= 0 && index < static_cast<int>(_programs.size())) return { _programs[index].name };
    return {};
}

juce::String DX10AudioProcessor::getPresetName(int index) const
{
    if (index >= 0 && index < static_cast<int>(_programs.size())) return { _programs[index].name };
    return {};
}

void DX10AudioProcessor::changeProgramName(int, const juce::String&) {}
void DX10AudioProcessor::prepareToPlay(double sampleRate, int) { _sampleRate = sampleRate; _inverseSampleRate = 1.0f / _sampleRate; resetState(); }
void DX10AudioProcessor::releaseResources() {}
void DX10AudioProcessor::reset() { resetState(); }
bool DX10AudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const { return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }

void DX10AudioProcessor::createPrograms()
{
    _programs.emplace_back("Bright E.Piano", 0.000f, 0.650f, 0.441f, 0.842f, 0.329f, 0.230f, 0.800f, 0.050f, 0.800f, 0.900f, 0.000f, 0.500f, 0.500f, 0.447f, 0.000f, 0.414f);
    _programs.emplace_back("Jazz E.Piano",   0.000f, 0.500f, 0.100f, 0.671f, 0.000f, 0.441f, 0.336f, 0.243f, 0.800f, 0.500f, 0.000f, 0.500f, 0.500f, 0.178f, 0.000f, 0.500f);
    _programs.emplace_back("E.Piano Pad",    0.000f, 0.700f, 0.400f, 0.230f, 0.184f, 0.270f, 0.474f, 0.224f, 0.800f, 0.974f, 0.250f, 0.500f, 0.500f, 0.428f, 0.836f, 0.500f);
    _programs.emplace_back("Fuzzy E.Piano",  0.000f, 0.700f, 0.400f, 0.320f, 0.217f, 0.599f, 0.670f, 0.309f, 0.800f, 0.500f, 0.263f, 0.507f, 0.500f, 0.276f, 0.638f, 0.526f);
    _programs.emplace_back("Soft Chimes",    0.400f, 0.600f, 0.650f, 0.760f, 0.000f, 0.390f, 0.250f, 0.160f, 0.900f, 0.500f, 0.362f, 0.500f, 0.500f, 0.401f, 0.296f, 0.493f);
    _programs.emplace_back("Harpsichord",    0.000f, 0.342f, 0.000f, 0.280f, 0.000f, 0.880f, 0.100f, 0.408f, 0.740f, 0.000f, 0.000f, 0.600f, 0.500f, 0.842f, 0.651f, 0.500f);
    _programs.emplace_back("Funk Clav",      0.000f, 0.400f, 0.100f, 0.360f, 0.000f, 0.875f, 0.160f, 0.592f, 0.800f, 0.500f, 0.000f, 0.500f, 0.500f, 0.303f, 0.868f, 0.500f);
    _programs.emplace_back("Sitar",          0.000f, 0.500f, 0.704f, 0.230f, 0.000f, 0.151f, 0.750f, 0.493f, 0.770f, 0.500f, 0.000f, 0.400f, 0.500f, 0.421f, 0.632f, 0.500f);
    _programs.emplace_back("Chiff Organ",    0.600f, 0.990f, 0.400f, 0.320f, 0.283f, 0.570f, 0.300f, 0.050f, 0.240f, 0.500f, 0.138f, 0.500f, 0.500f, 0.283f, 0.822f, 0.500f);
    _programs.emplace_back("Tinkle",         0.000f, 0.500f, 0.650f, 0.368f, 0.651f, 0.395f, 0.550f, 0.257f, 0.900f, 0.500f, 0.300f, 0.800f, 0.500f, 0.000f, 0.414f, 0.500f);
    _programs.emplace_back("Space Pad",      0.000f, 0.700f, 0.520f, 0.230f, 0.197f, 0.520f, 0.720f, 0.280f, 0.730f, 0.500f, 0.250f, 0.500f, 0.500f, 0.336f, 0.428f, 0.500f);
    _programs.emplace_back("Koto",           0.000f, 0.240f, 0.000f, 0.390f, 0.000f, 0.880f, 0.100f, 0.600f, 0.740f, 0.500f, 0.000f, 0.500f, 0.500f, 0.526f, 0.480f, 0.500f);
    _programs.emplace_back("Harp",           0.000f, 0.500f, 0.700f, 0.160f, 0.000f, 0.158f, 0.349f, 0.000f, 0.280f, 0.900f, 0.000f, 0.618f, 0.500f, 0.401f, 0.000f, 0.500f);
    _programs.emplace_back("Jazz Guitar",    0.000f, 0.500f, 0.100f, 0.390f, 0.000f, 0.490f, 0.250f, 0.250f, 0.800f, 0.500f, 0.000f, 0.500f, 0.500f, 0.263f, 0.145f, 0.500f);
    _programs.emplace_back("Steel Drum",     0.000f, 0.300f, 0.507f, 0.480f, 0.730f, 0.000f, 0.100f, 0.303f, 0.730f, 1.000f, 0.000f, 0.600f, 0.500f, 0.579f, 0.000f, 0.500f);
    _programs.emplace_back("Log Drum",       0.000f, 0.300f, 0.500f, 0.320f, 0.000f, 0.467f, 0.079f, 0.158f, 0.500f, 0.500f, 0.000f, 0.400f, 0.500f, 0.151f, 0.020f, 0.500f);
    _programs.emplace_back("Trumpet",        0.000f, 0.990f, 0.100f, 0.230f, 0.000f, 0.000f, 0.200f, 0.450f, 0.800f, 0.000f, 0.112f, 0.600f, 0.500f, 0.711f, 0.000f, 0.401f);
    _programs.emplace_back("Horn",           0.280f, 0.990f, 0.280f, 0.230f, 0.000f, 0.180f, 0.400f, 0.300f, 0.800f, 0.500f, 0.000f, 0.400f, 0.500f, 0.217f, 0.480f, 0.500f);
    _programs.emplace_back("Reed 1",         0.220f, 0.990f, 0.250f, 0.170f, 0.000f, 0.240f, 0.310f, 0.257f, 0.900f, 0.757f, 0.000f, 0.500f, 0.500f, 0.697f, 0.803f, 0.500f);
    _programs.emplace_back("Reed 2",         0.220f, 0.990f, 0.250f, 0.450f, 0.070f, 0.240f, 0.310f, 0.360f, 0.900f, 0.500f, 0.211f, 0.500f, 0.500f, 0.184f, 0.000f, 0.414f);
    _programs.emplace_back("Violin",         0.697f, 0.990f, 0.421f, 0.230f, 0.138f, 0.750f, 0.390f, 0.513f, 0.800f, 0.316f, 0.467f, 0.678f, 0.500f, 0.743f, 0.757f, 0.487f);
    _programs.emplace_back("Chunky Bass",    0.000f, 0.400f, 0.000f, 0.280f, 0.125f, 0.474f, 0.250f, 0.100f, 0.500f, 0.500f, 0.000f, 0.400f, 0.500f, 0.579f, 0.592f, 0.500f);
    _programs.emplace_back("E.Bass",         0.230f, 0.500f, 0.100f, 0.395f, 0.000f, 0.388f, 0.092f, 0.250f, 0.150f, 0.500f, 0.200f, 0.200f, 0.500f, 0.178f, 0.822f, 0.500f);
    _programs.emplace_back("Clunk Bass",     0.000f, 0.600f, 0.400f, 0.230f, 0.000f, 0.450f, 0.320f, 0.050f, 0.900f, 0.500f, 0.000f, 0.200f, 0.500f, 0.520f, 0.105f, 0.500f);
    _programs.emplace_back("Thick Bass",     0.000f, 0.600f, 0.400f, 0.170f, 0.145f, 0.290f, 0.350f, 0.100f, 0.900f, 0.500f, 0.000f, 0.400f, 0.500f, 0.441f, 0.309f, 0.500f);
    _programs.emplace_back("Sine Bass",      0.000f, 0.600f, 0.490f, 0.170f, 0.151f, 0.099f, 0.400f, 0.000f, 0.900f, 0.500f, 0.000f, 0.400f, 0.500f, 0.118f, 0.013f, 0.500f);
    _programs.emplace_back("Square Bass",    0.000f, 0.600f, 0.100f, 0.320f, 0.000f, 0.350f, 0.670f, 0.100f, 0.150f, 0.500f, 0.000f, 0.200f, 0.500f, 0.303f, 0.730f, 0.500f);
    _programs.emplace_back("Upright Bass 1", 0.300f, 0.500f, 0.400f, 0.280f, 0.000f, 0.180f, 0.540f, 0.000f, 0.700f, 0.500f, 0.000f, 0.400f, 0.500f, 0.296f, 0.033f, 0.500f);
    _programs.emplace_back("Upright Bass 2", 0.300f, 0.500f, 0.400f, 0.360f, 0.000f, 0.461f, 0.070f, 0.070f, 0.700f, 0.500f, 0.000f, 0.400f, 0.500f, 0.546f, 0.467f, 0.500f);
    _programs.emplace_back("Harmonics",      0.000f, 0.500f, 0.500f, 0.280f, 0.000f, 0.330f, 0.200f, 0.000f, 0.700f, 0.500f, 0.000f, 0.500f, 0.500f, 0.151f, 0.079f, 0.500f);
    _programs.emplace_back("Scratch",        0.000f, 0.500f, 0.000f, 0.000f, 0.240f, 0.580f, 0.630f, 0.000f, 0.000f, 0.500f, 0.000f, 0.600f, 0.500f, 0.816f, 0.243f, 0.500f);
    _programs.emplace_back("Syn Tom",        0.000f, 0.355f, 0.350f, 0.000f, 0.105f, 0.000f, 0.000f, 0.200f, 0.500f, 0.500f, 0.000f, 0.645f, 0.500f, 1.000f, 0.296f, 0.500f);
}

void DX10AudioProcessor::resetState()
{
    for (int v = 0; v < NVOICES; ++v) { _voices[v].env = 0.0f; _voices[v].car = 0.0f; _voices[v].dcar = 0.0f; _voices[v].mod0 = 0.0f; _voices[v].mod1 = 0.0f; _voices[v].dmod = 0.0f; _voices[v].cdec = 0.99f; }
    _numActiveVoices = 0; _notes[0] = EVENTS_DONE; _modWheel = 0.0f; _pitchBend = 1.0f; _volume = 0.0035f; _sustain = 0; _lfoStep = 0; _lfo0 = 0.0f; _lfo1 = 1.0f; _modulationAmount = 0.0f;
}

void DX10AudioProcessor::update()
{
    float param11 = apvts.getRawParameterValue("Octave")->load();
    _tune = 8.175798915644f * _inverseSampleRate * std::pow(2.0f, std::floor(param11 * 6.9f) - 2.0f);
    float param12 = apvts.getRawParameterValue("FineTune")->load();
    _fineTune = param12 + param12 - 1.0f;
    float coarse = apvts.getRawParameterValue("Coarse")->load();
    coarse = std::floor(40.1f * coarse * coarse);
    float fine = apvts.getRawParameterValue("Fine")->load();
    if (fine < 0.5f) { fine = 0.2f * fine * fine; }
    else { switch (int(8.9f * fine)) { case 4: fine = 0.25f; break; case 5: fine = 0.33333333f; break; case 6: fine = 0.50f; break; case 7: fine = 0.66666667f; break; default: fine = 0.75f; } }
    _ratio = 1.570796326795f * (coarse + fine);
    _velocitySensitivity = apvts.getRawParameterValue("Mod Vel")->load();
    float param10 = apvts.getRawParameterValue("Vibrato")->load();
    _vibrato = 0.001f * param10 * param10;
    float param0 = apvts.getRawParameterValue("Attack")->load();
    _attack = 1.0f - std::exp(-_inverseSampleRate * std::exp(8.0f - 8.0f * param0));
    float param1 = apvts.getRawParameterValue("Decay")->load();
    if (param1 > 0.98f) { _decay = 1.0f; } else { _decay = std::exp(-_inverseSampleRate * std::exp(5.0f - 8.0f * param1)); }
    float param2 = apvts.getRawParameterValue("Release")->load();
    _release = std::exp(-_inverseSampleRate * std::exp(5.0f - 5.0f * param2));
    float param5 = apvts.getRawParameterValue("Mod Init")->load();
    _modInitialLevel = 0.0002f * param5 * param5;
    float param6 = apvts.getRawParameterValue("Mod Dec")->load();
    _modDecay = 1.0f - std::exp(-_inverseSampleRate * std::exp(6.0f - 7.0f * param6));
    float param7 = apvts.getRawParameterValue("Mod Sus")->load();
    _modSustain = 0.0002f * param7 * param7;
    float param8 = apvts.getRawParameterValue("Mod Rel")->load();
    _modRelease = 1.0f - std::exp(-_inverseSampleRate * std::exp(5.0f - 8.0f * param8));
    float param13 = apvts.getRawParameterValue("Waveform")->load();
    _richness = 0.50f - 3.0f * param13 * param13;
    float param14 = apvts.getRawParameterValue("Mod Thru")->load();
    _modMix = 0.25f * param14 * param14;
    float param15 = apvts.getRawParameterValue("LFO Rate")->load();
    _lfoInc = 628.3f * _inverseSampleRate * 25.0f * param15 * param15;
}

void DX10AudioProcessor::processEvents(juce::MidiBuffer &midiMessages)
{
    int npos = 0;
    for (const auto metadata : midiMessages) {
        if (metadata.numBytes != 3) continue;
        const auto data0 = metadata.data[0]; const auto data1 = metadata.data[1]; const auto data2 = metadata.data[2];
        const int deltaFrames = metadata.samplePosition;
        switch (data0 & 0xf0) {
            case 0x80: _notes[npos++] = deltaFrames; _notes[npos++] = data1 & 0x7F; _notes[npos++] = 0; break;
            case 0x90: _notes[npos++] = deltaFrames; _notes[npos++] = data1 & 0x7F; _notes[npos++] = data2 & 0x7F; break;
            case 0xB0:
                switch (data1) {
                    case 0x01: _modWheel = 0.00000005f * float(data2 * data2); break;
                    case 0x07: _volume = 0.00000035f * float(data2 * data2); break;
                    case 0x40: _sustain = data2 & 0x40; if (_sustain == 0) { _notes[npos++] = deltaFrames; _notes[npos++] = SUSTAIN; _notes[npos++] = 0; } break;
                    default: if (data1 > 0x7A) { for (int v = 0; v < NVOICES; ++v) _voices[v].cdec = 0.99f; _sustain = 0; } break;
                }
                break;
            case 0xC0: if (data1 < _programs.size()) setCurrentProgram(data1); break;
            case 0xE0: _pitchBend = float(data1 + 128 * data2 - 8192); _pitchBend = (_pitchBend > 0.0f) ? 1.0f + 0.000014951f * _pitchBend : 1.0f + 0.000013318f * _pitchBend; break;
            default: break;
        }
        if (npos > EVENTBUFFER) npos -= 3;
    }
    _notes[npos] = EVENTS_DONE;
    midiMessages.clear();
}

void DX10AudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) buffer.clear(i, 0, buffer.getNumSamples());

    update();
    processEvents(midiMessages);

    int sampleFrames = buffer.getNumSamples();
    float *out1 = buffer.getWritePointer(0);
    float *out2 = buffer.getWritePointer(1);
    int event = 0, frame = 0;

    if (_numActiveVoices > 0 || _notes[event] < sampleFrames) {
        while (frame < sampleFrames) {
            int frames = _notes[event++];
            if (frames > sampleFrames) frames = sampleFrames;
            frames -= frame;
            frame += frames;

            while (--frames >= 0) {
                Voice *V = _voices;
                float o = 0.0f;
                if (--_lfoStep < 0) { _lfo0 += _lfoInc * _lfo1; _lfo1 -= _lfoInc * _lfo0; _modulationAmount = _lfo1 * (_modWheel + _vibrato); _lfoStep = 100; }
                for (int v = 0; v < NVOICES; ++v) {
                    float e = V->env;
                    if (e > SILENCE) {
                        V->env = e * V->cdec;
                        V->cenv += V->catt * (e - V->cenv);
                        float y = V->dmod * V->mod0 - V->mod1; V->mod1 = V->mod0; V->mod0 = y;
                        V->menv += V->mdec * (V->mlev - V->menv);
                        float x = V->car + V->dcar + y * V->menv + _modulationAmount;
                        while (x > 1.0f) x -= 2.0f; while (x < -1.0f) x += 2.0f;
                        V->car = x;
                        float s = x + x * x * x * (_richness * x * x - 1.0f - _richness);
                        o += V->cenv * (_modMix * V->mod1 + s);
                    }
                    V++;
                }
                *out1++ = o; *out2++ = o;
            }
            if (frame < sampleFrames) { int note = _notes[event++]; int vel = _notes[event++]; noteOn(note, vel); }
        }
        _numActiveVoices = NVOICES;
        for (int v = 0; v < NVOICES; ++v) {
            if (_voices[v].env < SILENCE) { _voices[v].env = 0.0f; _voices[v].cenv = 0.0f; _numActiveVoices--; }
            if (_voices[v].menv < SILENCE) { _voices[v].menv = 0.0f; _voices[v].mlev = 0.0f; }
        }
    } else {
        while (--sampleFrames >= 0) { *out1++ = 0.0f; *out2++ = 0.0f; }
    }
    _notes[0] = EVENTS_DONE;

    // Push audio to spectrum analyzer
    if (spectrumAnalyzer != nullptr)
        spectrumAnalyzer->pushBuffer(buffer);
}

void DX10AudioProcessor::noteOn(int note, int velocity)
{
    if (velocity > 0) {
        float l = 1.0f; int vl = 0;
        for (int v = 0; v < NVOICES; v++) { if (_voices[v].env < l) { l = _voices[v].env; vl = v; } }
        float p = std::exp(0.05776226505f * (float(note) + _fineTune));
        _voices[vl].note = note;
        _voices[vl].car = 0.0f;
        _voices[vl].dcar = _tune * _pitchBend * p;
        if (p > 50.0f) p = 50.0f;
        p *= (64.0f + _velocitySensitivity * (velocity - 64));
        _voices[vl].menv = _modInitialLevel * p;
        _voices[vl].mlev = _modSustain * p;
        _voices[vl].mdec = _modDecay;
        _voices[vl].dmod = _ratio * _voices[vl].dcar;
        _voices[vl].mod0 = 0.0f;
        _voices[vl].mod1 = std::sin(_voices[vl].dmod);
        _voices[vl].dmod = 2.0f * std::cos(_voices[vl].dmod);
        float param13 = apvts.getRawParameterValue("Waveform")->load();
        _voices[vl].env = (1.5f - param13) * _volume * (velocity + 10);
        _voices[vl].cdec = _decay;
        _voices[vl].catt = _attack;
        _voices[vl].cenv = 0.0f;
    } else {
        for (int v = 0; v < NVOICES; v++) {
            if (_voices[v].note == note) {
                if (_sustain == 0) { _voices[v].cdec = _release; _voices[v].env = _voices[v].cenv; _voices[v].catt = 1.0f; _voices[v].mlev = 0.0f; _voices[v].mdec = _modRelease; }
                else { _voices[v].note = SUSTAIN; }
            }
        }
    }
}

juce::AudioProcessorEditor *DX10AudioProcessor::createEditor() { return new DX10AudioProcessorEditor(*this); }

void DX10AudioProcessor::getStateInformation(juce::MemoryBlock &destData) { copyXmlToBinary(*apvts.copyState().createXml(), destData); }

void DX10AudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
        if (auto* param = apvts.getRawParameterValue("PresetIndex"))
            _currentProgram = static_cast<int>(param->load() * (NPRESETS - 1) + 0.5f);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout DX10AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("PresetIndex", 1), "Preset", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Attack", 1), "Attack", juce::NormalisableRange<float>(), 0.0f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Decay", 1), "Decay", juce::NormalisableRange<float>(), 0.65f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Release", 1), "Release", juce::NormalisableRange<float>(), 0.441f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Coarse", 1), "Coarse", juce::NormalisableRange<float>(), 0.842f, juce::AudioParameterFloatAttributes().withLabel("ratio").withStringFromValueFunction([](float v, int) { return juce::String(int(std::floor(40.1f * v * v))); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Fine", 1), "Fine", juce::NormalisableRange<float>(), 0.329f, juce::AudioParameterFloatAttributes().withLabel("ratio").withStringFromValueFunction([](float v, int) { float f = 0.0f; if (v < 0.5f) { f = 0.2f * v * v; } else { switch (int(8.9f * v)) { case 4: f = 0.25f; break; case 5: f = 0.33333333f; break; case 6: f = 0.50f; break; case 7: f = 0.66666667f; break; default: f = 0.75f; } } return juce::String(f, 3); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Mod Init", 1), "Mod Init", juce::NormalisableRange<float>(), 0.23f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Mod Dec", 1), "Mod Dec", juce::NormalisableRange<float>(), 0.8f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Mod Sus", 1), "Mod Sus", juce::NormalisableRange<float>(), 0.05f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Mod Rel", 1), "Mod Rel", juce::NormalisableRange<float>(), 0.8f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Mod Vel", 1), "Mod Vel", juce::NormalisableRange<float>(), 0.9f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Vibrato", 1), "Vibrato", juce::NormalisableRange<float>(), 0.0f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Octave", 1), "Octave", juce::NormalisableRange<float>(), 0.5f, juce::AudioParameterFloatAttributes().withStringFromValueFunction([](float v, int) { return juce::String(int(v * 6.9f) - 3); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("FineTune", 1), "FineTune", juce::NormalisableRange<float>(), 0.5f, juce::AudioParameterFloatAttributes().withLabel("cents").withStringFromValueFunction([](float v, int) { return juce::String(int(200.0f * v - 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Waveform", 1), "Waveform", juce::NormalisableRange<float>(), 0.447f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Mod Thru", 1), "Mod Thru", juce::NormalisableRange<float>(), 0.0f, juce::AudioParameterFloatAttributes().withLabel("%").withStringFromValueFunction([](float v, int) { return juce::String(int(v * 100.0f)); })));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LFO Rate", 1), "LFO Rate", juce::NormalisableRange<float>(), 0.414f, juce::AudioParameterFloatAttributes().withLabel("Hz").withStringFromValueFunction([](float v, int) { return juce::String(25.0f * v * v, 2); })));
    return layout;
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new DX10AudioProcessor(); }
