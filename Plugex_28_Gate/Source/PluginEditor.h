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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "PlugexLookAndFeel.h"

//==============================================================================
/**
*/
class Plugex_28_gateAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    Plugex_28_gateAudioProcessorEditor (Plugex_28_gateAudioProcessor&, AudioProcessorValueTreeState& vts);
    ~Plugex_28_gateAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Plugex_28_gateAudioProcessor& processor;

    AudioProcessorValueTreeState& valueTreeState;

    PlugexLookAndFeel plugexLookAndFeel;

    Label title;

    Label  threshLabel;
    Slider threshKnob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> threshAttachment;

    Label  risetimeLabel;
    Slider risetimeKnob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> risetimeAttachment;

    Label  falltimeLabel;
    Slider falltimeKnob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> falltimeAttachment;

    Label  lookaheadLabel;
    Slider lookaheadKnob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> lookaheadAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Plugex_28_gateAudioProcessorEditor)
};
