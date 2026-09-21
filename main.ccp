#include "MainComponent.h"
#include <JuceHeader.h>
// Window Design and Audio Setup
MainComponent::MainComponent()
    : state(Stopped)
{
    openButton.setButtonText("Open...");
    openButton.onClick = [this] { openButtonClicked(); };
    addAndMakeVisible(openButton);

    playButton.setButtonText("Play");
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);
    addAndMakeVisible(playButton);

    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);

    setSize(600, 300);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (transportSource.getTotalLength() <= 0)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    openButton.setBounds(10, 10, 80, 30);
    playButton.setBounds(100, 10, 80, 30);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState(Playing);
        else
            changeState(Stopped);
    }
}

void MainComponent::changeState(TransportState newState)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
        case Stopped:
            playButton.setButtonText("Play");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
            transportSource.setPosition(0.0);
            break;

        case Starting:
            playButton.setEnabled(true);
            transportSource.start();
            break;

        case Playing:
            playButton.setButtonText("Stop");
            playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            break;

        case Stopping:
            transportSource.stop();
            break;
        }
    }
}

void MainComponent::openButtonClicked()
{
    auto libraryDirectory = juce::File(__FILE__) //Finds the library internally
                                .getParentDirectory()
                                .getChildFile("Library");

    chooser = std::make_unique<juce::FileChooser>(//File explorer music select needs to be deprecated with GUI
        "Select a WAV or MP3 file to play...",
        libraryDirectory,
        "*.wav;*.mp3;*.flac;*.aiff");

    auto fileChooserFlags = juce::FileBrowserComponent::openMode
                          | juce::FileBrowserComponent::canSelectFiles; //file selector

    chooser->launchAsync(fileChooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file != juce::File{})//Play features likely to change with play/pause
        {
            auto* reader = formatManager.createReaderFor(file);

            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(
                    reader, true);

                transportSource.setSource(
                    newSource.get(),
                    0,
                    nullptr,
                    reader->sampleRate);

                playButton.setEnabled(true);
                readerSource = std::move(newSource);
            }
        }
    });
}



void MainComponent::playButtonClicked()
{
    if (state == Stopped)
        changeState(Starting);
    else if (state == Playing)
        changeState(Stopping);
}
