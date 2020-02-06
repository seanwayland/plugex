/*******************************************************************************
* Plugex - PLUGin EXamples
*
* Plugex est une série de plugiciels auto-documentés permettant une étude 
* autonome du développement de plugiciels avec JUCE ainsi que des bases du
* traitement de signal audio avec le langage C++.
*
* © Olivier Bélanger 2020
*
*******************************************************************************/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "Granulator.h"

//==============================================================================
/**
*/
class Plugex_34_granularStretcherAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    Plugex_34_granularStretcherAudioProcessor();
    ~Plugex_34_granularStretcherAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    AudioProcessorValueTreeState parameters;

    double m_sampleRate;

    Random jitterRandom;

    Granulator granulator[2];

    float portLastSample = 0.f;
    float readerIndex = 0.f;
    float readerBaseInc = 0.f;

    bool isRecording;

    bool isActive = false;
    float *activeParameter = nullptr;

    float *durationParameter = nullptr;
    SmoothedValue<float> durationSmoothed;

    float *pitchParameter = nullptr;
    SmoothedValue<float> pitchSmoothed;

    float *speedParameter = nullptr;
    SmoothedValue<float> speedSmoothed;

    float *jitterParameter = nullptr;
    SmoothedValue<float> jitterSmoothed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Plugex_34_granularStretcherAudioProcessor)
};
