/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BirdcallAudioProcessor::BirdcallAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

BirdcallAudioProcessor::~BirdcallAudioProcessor()
{
}

//==============================================================================
const juce::String BirdcallAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BirdcallAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BirdcallAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BirdcallAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BirdcallAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BirdcallAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BirdcallAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BirdcallAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BirdcallAudioProcessor::getProgramName (int index)
{
    return {};
}

void BirdcallAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BirdcallAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    auto chainSettings = getChainSettings(apvts);
    auto peakQuality = 10.f;
    auto peakGain = juce::Decibels::decibelsToGain(24.f);
    auto peakFrequency = 20.f;

    switch( chainSettings.note )
    {
        case Bypass:
            peakGain = juce::Decibels::decibelsToGain(0.f);
            break;
        case C4:
            peakFrequency = 261.63;
        case Db4:
            peakFrequency = 277.18;
        case D4:
            peakFrequency = 293.66;
        case Eb4:
            peakFrequency = 311.13;
        case E4:
            peakFrequency = 329.63;
        case F4:
            peakFrequency = 349.23;
        case Gb4:
            peakFrequency = 369.99;
        case G4:
            peakFrequency = 392.00;
        case Ab4:
            peakFrequency = 415.30;
        case A4:
            peakFrequency = 440.00;
        case Bb4:
            peakFrequency = 466.16;
        case B4:
            peakFrequency = 493.88;
        }
    
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, peakFrequency, peakQuality, peakGain);

    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
}

void BirdcallAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BirdcallAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void BirdcallAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto chainSettings = getChainSettings(apvts);
    auto peakQuality = 10.f;
    auto peakGain = juce::Decibels::decibelsToGain(24.f);
    auto peakFrequency = 20.f;

    switch( chainSettings.note )
    {
        case Bypass:
            peakGain = juce::Decibels::decibelsToGain(0.f);
            break;
        case C4:
            peakFrequency = 261.63;
        case Db4:
            peakFrequency = 277.18;
        case D4:
            peakFrequency = 293.66;
        case Eb4:
            peakFrequency = 311.13;
        case E4:
            peakFrequency = 329.63;
        case F4:
            peakFrequency = 349.23;
        case Gb4:
            peakFrequency = 369.99;
        case G4:
            peakFrequency = 392.00;
        case Ab4:
            peakFrequency = 415.30;
        case A4:
            peakFrequency = 440.00;
        case Bb4:
            peakFrequency = 466.16;
        case B4:
            peakFrequency = 493.88;
        }
    
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), peakFrequency, peakQuality, peakGain);
    
    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;

    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(0);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool BirdcallAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BirdcallAudioProcessor::createEditor()
{
    // return new BirdcallAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void BirdcallAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BirdcallAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.note = static_cast<Notes>(apvts.getRawParameterValue("Note")->load());

    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout BirdcallAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    juce::StringArray stringArray;
    stringArray.add("Bypass");
    stringArray.add("C4");
    stringArray.add("Db4");
    stringArray.add("D4");
    stringArray.add("Eb4");
    stringArray.add("E4");
    stringArray.add("F4");
    stringArray.add("Gb4");
    stringArray.add("G4");
    stringArray.add("Ab4");
    stringArray.add("A4");
    stringArray.add("Bb4");
    stringArray.add("B4");

    layout.add(std::make_unique<juce::AudioParameterChoice>("Note", "Note", stringArray, 0));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BirdcallAudioProcessor();
}
