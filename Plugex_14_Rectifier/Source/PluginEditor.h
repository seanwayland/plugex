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
class Plugex_14_rectifierAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    Plugex_14_rectifierAudioProcessorEditor (Plugex_14_rectifierAudioProcessor&, AudioProcessorValueTreeState& vts);
    ~Plugex_14_rectifierAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Plugex_14_rectifierAudioProcessor& processor;

    AudioProcessorValueTreeState& valueTreeState;

    PlugexLookAndFeel plugexLookAndFeel;

    Label title;

    Label  percentLabel;
    Slider percentKnob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> percentAttachment;

    Label  cutoffLabel;
    Slider cutoffKnob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Plugex_14_rectifierAudioProcessorEditor)
};
