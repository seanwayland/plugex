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

static String freqSliderValueToText(float value) {
    return String(value, 2) + String(" Hz");
}

static float freqSliderTextToValue(const String& text) {
    return text.getFloatValue();
}

static String qSliderValueToText(float value) {
    return String(value, 2) + String(" Q");
}

static float qSliderTextToValue(const String& text) {
    return text.getFloatValue();
}

AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    using Parameter = AudioProcessorValueTreeState::Parameter;

    std::vector<std::unique_ptr<Parameter>> parameters;

    parameters.push_back(std::make_unique<Parameter>(String("freq"), String("Freq"), String(),
                                                     NormalisableRange<float>(20.0f, 18000.0f, 0.01f, 0.3f),
                                                     1000.0f, freqSliderValueToText, freqSliderTextToValue));

    parameters.push_back(std::make_unique<Parameter>(String("q"), String("Q"), String(),
                                                     NormalisableRange<float>(0.5f, 50.0f, 0.01f, 0.5f),
                                                     1.0f, qSliderValueToText, qSliderTextToValue));

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
Plugex_09_butterworthBpAudioProcessor::Plugex_09_butterworthBpAudioProcessor()
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
    freqParameter = parameters.getRawParameterValue("freq");
    qParameter = parameters.getRawParameterValue("q");
}

Plugex_09_butterworthBpAudioProcessor::~Plugex_09_butterworthBpAudioProcessor()
{
}

//==============================================================================
const String Plugex_09_butterworthBpAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Plugex_09_butterworthBpAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Plugex_09_butterworthBpAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Plugex_09_butterworthBpAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Plugex_09_butterworthBpAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Plugex_09_butterworthBpAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Plugex_09_butterworthBpAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Plugex_09_butterworthBpAudioProcessor::setCurrentProgram (int index)
{
}

const String Plugex_09_butterworthBpAudioProcessor::getProgramName (int index)
{
    return {};
}

void Plugex_09_butterworthBpAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Plugex_09_butterworthBpAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    memset(lastInputSample1, 0, sizeof(float) * 2);
    memset(lastInputSample2, 0, sizeof(float) * 2);
    memset(lastFilteredSample1, 0, sizeof(float) * 2);
    memset(lastFilteredSample2, 0, sizeof(float) * 2);

    freqSmoothed.reset(sampleRate, samplesPerBlock/sampleRate);
    freqSmoothed.setCurrentAndTargetValue(*freqParameter);
    qSmoothed.reset(sampleRate, samplesPerBlock/sampleRate);
    qSmoothed.setCurrentAndTargetValue(*qParameter);
}

void Plugex_09_butterworthBpAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Plugex_09_butterworthBpAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Plugex_09_butterworthBpAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    freqSmoothed.setTargetValue(*freqParameter);
    qSmoothed.setTargetValue(*qParameter);

    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        float cf = freqSmoothed.getNextValue();
        float bw = cf / qSmoothed.getNextValue();

        float piOnSr = M_PI / currentSampleRate;
        float c = 1.0f / tanf(piOnSr * bw);
        float d = 2.0f * cosf(2.0f * piOnSr * cf);

        float a0 = 1.0f / (1.0f + c);
        float a2 = -a0;
        float b1 = a2 * c * d;
        float b2 = a0 * (c - 1.0f);
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            float filtered = a0 * channelData[i] + a2 * lastInputSample2[channel] - 
                             b1 * lastFilteredSample1[channel] - b2 * lastFilteredSample2[channel];
            lastInputSample2[channel] = lastInputSample1[channel];
            lastInputSample1[channel] = channelData[i];
            lastFilteredSample2[channel] = lastFilteredSample1[channel];
            channelData[i] = lastFilteredSample1[channel] = filtered;
        }
    }
}

//==============================================================================
bool Plugex_09_butterworthBpAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Plugex_09_butterworthBpAudioProcessor::createEditor()
{
    return new Plugex_09_butterworthBpAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void Plugex_09_butterworthBpAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Plugex_09_butterworthBpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Plugex_09_butterworthBpAudioProcessor();
}
