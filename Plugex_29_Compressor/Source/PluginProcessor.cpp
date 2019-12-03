/*******************************************************************************
* Plugex - PLUGin EXamples
*
* Plugex est une série de plugiciels auto-documentés permettant une étude 
* autonome du développement de plugiciels avec JUCE ainsi que des bases du
* traitement de signal audio avec le langage C++.
*
* © Olivier Bélanger 2019
*
*******************************************************************************/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif

static String threshSliderValueToText(float value) {
    return String(value, 2) + String(" dB");
}

static float threshSliderTextToValue(const String& text) {
    return text.getFloatValue();
}

static String ratioSliderValueToText(float value) {
    return String(value, 2) + String(" x");
}

static float ratioSliderTextToValue(const String& text) {
    return text.getFloatValue();
}

static String risetimeSliderValueToText(float value) {
    return String(value, 2) + String(" ms");
}

static float risetimeSliderTextToValue(const String& text) {
    return text.getFloatValue();
}

static String lookaheadSliderValueToText(float value) {
    return String(value, 2) + String(" ms");
}

static float lookaheadSliderTextToValue(const String& text) {
    return text.getFloatValue();
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    using Parameter = AudioProcessorValueTreeState::Parameter;

    std::vector<std::unique_ptr<Parameter>> parameters;

    parameters.push_back(std::make_unique<Parameter>(String("thresh"), String("Tresh"), String(),
                                                     NormalisableRange<float>(-60.0f, 0.0f, 0.01f, 1.0f),
                                                     -20.0f, threshSliderValueToText, threshSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("ratio"), String("Ratio"), String(),
                                                     NormalisableRange<float>(1.0f, 20.0f, 0.01f, 0.3f),
                                                     2.0f, ratioSliderValueToText, ratioSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("risetime"), String("Risetime"), String(),
                                                     NormalisableRange<float>(0.01f, 500.0f, 0.01f, 0.3f),
                                                     10.0f, risetimeSliderValueToText, risetimeSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("falltime"), String("Falltime"), String(),
                                                     NormalisableRange<float>(0.01f, 500.0f, 0.01f, 0.3f),
                                                     100.0f, risetimeSliderValueToText, risetimeSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("lookahead"), String("LookAhead"), String(),
                                                     NormalisableRange<float>(0.01f, 10.0f, 0.01f, 1.0f),
                                                     5.0f, lookaheadSliderValueToText, lookaheadSliderTextToValue));

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
Plugex_29_compressorAudioProcessor::Plugex_29_compressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters (*this, nullptr, Identifier(JucePlugin_Name), createParameterLayout())
{
    threshParameter = parameters.getRawParameterValue("thresh");
    ratioParameter = parameters.getRawParameterValue("ratio");
    risetimeParameter = parameters.getRawParameterValue("risetime");
    falltimeParameter = parameters.getRawParameterValue("falltime");
    lookaheadParameter = parameters.getRawParameterValue("lookahead");
}

Plugex_29_compressorAudioProcessor::~Plugex_29_compressorAudioProcessor()
{
}

//==============================================================================
const String Plugex_29_compressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Plugex_29_compressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Plugex_29_compressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Plugex_29_compressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Plugex_29_compressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Plugex_29_compressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Plugex_29_compressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Plugex_29_compressorAudioProcessor::setCurrentProgram (int index)
{
}

const String Plugex_29_compressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void Plugex_29_compressorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Plugex_29_compressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    follower[0] = follower[1] = 0.0f;

    threshSmoothed.reset(sampleRate, samplesPerBlock/sampleRate);
    threshSmoothed.setCurrentAndTargetValue(*threshParameter);
    ratioSmoothed.reset(sampleRate, samplesPerBlock/sampleRate);
    ratioSmoothed.setCurrentAndTargetValue(*ratioParameter);
    risetimeSmoothed.reset(sampleRate, samplesPerBlock/sampleRate);
    risetimeSmoothed.setCurrentAndTargetValue(*risetimeParameter);
    falltimeSmoothed.reset(sampleRate, samplesPerBlock/sampleRate);
    falltimeSmoothed.setCurrentAndTargetValue(*falltimeParameter);
    lookaheadSmoothed.reset(sampleRate, samplesPerBlock/sampleRate);
    lookaheadSmoothed.setCurrentAndTargetValue(*lookaheadParameter);

    for (int channel = 0; channel < 2; channel++) {
        lookaheadDelay[channel].setup(0.015, currentSampleRate);
    }
}

void Plugex_29_compressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Plugex_29_compressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Plugex_29_compressorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    threshSmoothed.setTargetValue(*threshParameter);
    ratioSmoothed.setTargetValue(*ratioParameter);
    risetimeSmoothed.setTargetValue(*risetimeParameter);
    falltimeSmoothed.setTargetValue(*falltimeParameter);
    lookaheadSmoothed.setTargetValue(*lookaheadParameter);

    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        float thresh = threshSmoothed.getNextValue();
        float ratio = ratioSmoothed.getNextValue();
        float risetime = risetimeSmoothed.getNextValue() * 0.001f;
        float falltime = falltimeSmoothed.getNextValue() * 0.001f;
        float lookahead = lookaheadSmoothed.getNextValue() * 0.001f;

        ratio = 1.0f / ratio;
        risetime = expf(-1.0f / (currentSampleRate * risetime));
        falltime = expf(-1.0f / (currentSampleRate * falltime));
        float knee = 0.5f;                                                      /* 0.001 = hard knee, 1 = soft knee */
        thresh += 3.0f * knee;
        if (thresh > 0.0)
            thresh = 0.0;
        float ampthresh = powf(10.0f, thresh * 0.05f);                          /* up to 3 dB above threshold */
        float kneethresh = powf(10.0f, (thresh - (knee * 8.5f + 0.5f)) * 0.05); /* up to 6 dB under threshold */
        float invKneeRange = 1.0f / (ampthresh - kneethresh);

        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            auto* channelData = buffer.getWritePointer (channel);

            /* Envelope follower */
            float rectified = channelData[i] < 0.0f ? -channelData[i] : channelData[i];
            if (follower[channel] < rectified) {
                follower[channel] = rectified + risetime * (follower[channel] - rectified);
            } else {
                follower[channel] = rectified + falltime * (follower[channel] - rectified);
            }

            /* Look ahead */
            float delayedSample = lookaheadDelay[channel].read(lookahead);
            lookaheadDelay[channel].write(channelData[i]);

            /* Compress signal */
            float outAmplitude = 1.0f;
            if (follower[channel] > ampthresh) {                          /* Above threshold */
                float indb = 20.0f * log10f(follower[channel] + 1.0e-20);
                float diff = indb - thresh;
                float outdb = diff - diff * ratio;
                outAmplitude = powf(10.0f, -outdb * 0.05f);
            } else if (follower[channel] > kneethresh) {                  /* Under the knee */
                float kneescl = (follower[channel] - kneethresh) * invKneeRange;
                float kneeratio = (((knee + 1.0f) * kneescl) / (knee + kneescl)) * (ratio - 1.0f) + 1.0f;
                float indb = 20.0f * log10f(follower[channel] + 1.0e-20);
                float diff = indb - thresh;
                float outdb = diff - diff * kneeratio;
                outAmplitude = powf(10.0f, -outdb * 0.05f);
            }
            outAmplitude = outAmplitude < 1.0e-20 ? 1.0e-20 : outAmplitude > 1.0f ? 1.0f : outAmplitude;
            channelData[i] = delayedSample * outAmplitude;
        }
    }
}

//==============================================================================
bool Plugex_29_compressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Plugex_29_compressorAudioProcessor::createEditor()
{
    return new Plugex_29_compressorAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void Plugex_29_compressorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Plugex_29_compressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Plugex_29_compressorAudioProcessor();
}
