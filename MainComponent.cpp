#include "MainComponent.h"

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

    setSize(1500, 750);
    setAudioChannels(0, 2);

    //==========================================================================
    // HEAVY METAL VOLUME CONTROL
    //==========================================================================

    volumeSlider.setRange(0.0, 100.0, 1.0);
    volumeSlider.setValue(75.0);

    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);

    volumeSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        70,
        25
    );

    volumeSlider.setTextValueSuffix("%");

    // Heavy metal colors
    volumeSlider.setColour(
        juce::Slider::backgroundColourId,
        juce::Colours::black
    );

    volumeSlider.setColour(
        juce::Slider::trackColourId,
        juce::Colours::darkgrey
    );

    volumeSlider.setColour(
        juce::Slider::thumbColourId,
        juce::Colours::red
    );

    volumeSlider.setColour(
        juce::Slider::textBoxTextColourId,
        juce::Colours::white
    );

    volumeSlider.setColour(
        juce::Slider::textBoxBackgroundColourId,
        juce::Colours::black
    );

    volumeSlider.setColour(
        juce::Slider::textBoxOutlineColourId,
        juce::Colours::darkgrey
    );

    // Actually change the music volume
    volumeSlider.onValueChange = [this]
        {
            transportSource.setGain(
                static_cast<float>(volumeSlider.getValue() / 100.0)
            );
        };

    addAndMakeVisible(volumeSlider);

    // Volume label
    volumeLabel.setText(
        "VOLUME",
        juce::dontSendNotification
    );

    volumeLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::white
    );

    volumeLabel.setFont(
        juce::Font(18.0f, juce::Font::bold)
    );

    volumeLabel.setJustificationType(
        juce::Justification::centred
    );

    addAndMakeVisible(volumeLabel);

    auto soundButton = juce::ImageCache::getFromMemory(BinaryData::Speaker_Icon_png, BinaryData::Speaker_Icon_pngSize);
    soundIcon.setImage(soundButton, juce::RectanglePlacement::stretchToFit);
    addAndMakeVisible(soundIcon);
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
    //background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));  

    //volume bar background
    g.setColour(juce::Colour(0xff56009E));
    g.drawRoundedRectangle(getWidth() - 100, 20, 100, getHeight() - 160, 10, 5);
    g.setColour(juce::Colour(0xff6800BD));
    g.fillRoundedRectangle(getWidth() - 100, 20, 100, getHeight() - 160, 10);

    //play button background
    g.setColour(juce::Colour(0xff56009E));
    g.drawRoundedRectangle(0, getHeight() - 120, getWidth(), getHeight(), 10, 5);
    g.setColour(juce::Colour(0xff6800BD));
    g.fillRoundedRectangle(0, getHeight() - 120, getWidth(), getHeight(), 10);
    
}

void MainComponent::resized()
{
    //open music files
    openButton.setBounds(10, 10, 80, 30);

    //play button
    playButton.setBounds((getWidth() / 2) - 40, getHeight() - 100, 80, 30);

    // Heavy Metal Volume Control
    volumeSlider.setBounds(getWidth() - 90, getHeight() - 360, 80, 190);
    soundIcon.setBounds(getWidth() - 65, getHeight() - 395, 30, 30);
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
