#include "MainComponent.h"

MainComponent::MainComponent()
    : state(Stopped)
{
    openButton.setButtonText("Open...");
    openButton.onClick = [this] { openButtonClicked(); };
    addAndMakeVisible(openButton);

    playButton.setButtonText("Play");
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(
        juce::TextButton::buttonColourId,
        juce::Colours::green
    );
    playButton.setEnabled(false);
    addAndMakeVisible(playButton);

    libraryBox.setTextWhenNothingSelected("Library (empty)");
    libraryBox.onChange = [this] { trackSelected(); };
    addAndMakeVisible(libraryBox);

    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);

    setSize(600, 300);
    setAudioChannels(0, 2);

    refreshLibraryList();
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


    // Volume Label

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
}

MainComponent::~MainComponent()
{
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    shutdownAudio();
}

void MainComponent::prepareToPlay(
    int samplesPerBlockExpected,
    double sampleRate)
{
    transportSource.prepareToPlay(
        samplesPerBlockExpected,
        sampleRate
    );
}

void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& bufferToFill)
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
    g.fillAll(
        getLookAndFeel().findColour(
            juce::ResizableWindow::backgroundColourId
        )
    );
}

void MainComponent::resized()
{
    openButton.setBounds(10, 10, 80, 30);
    playButton.setBounds(100, 10, 80, 30);
    libraryBox.setBounds(10, 50, 300, 30);

    volumeLabel.setBounds(
        getWidth() - 150,
        getHeight() - 270,
        100,
        30
    );

    volumeSlider.setBounds(
        getWidth() - 140,
        getHeight() - 235,
        80,
        190
    );
}

void MainComponent::changeListenerCallback(
    juce::ChangeBroadcaster* source)
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
    if (state == newState)
        return;

    state = newState;

    switch (state)
    {
    case Stopped:
        playButton.setButtonText("Play");
        playButton.setColour(
            juce::TextButton::buttonColourId,
            juce::Colours::green
        );
        break;

    case Starting:
        playButton.setEnabled(true);
        transportSource.start();
        break;

    case Playing:
        playButton.setButtonText("Pause");
        playButton.setColour(
            juce::TextButton::buttonColourId,
            juce::Colours::red
        );
        break;

    case Stopping:
        transportSource.stop();
        break;
    }
}


juce::File MainComponent::getLibraryDirectory()
{
    auto libraryDir =
        juce::File::getSpecialLocation(
            juce::File::currentExecutableFile)
        .getParentDirectory()
        .getChildFile("library");

    if (!libraryDir.exists())
        libraryDir.createDirectory();

    return libraryDir;
}

void MainComponent::refreshLibraryList()
{
    libraryFiles.clear();
    libraryBox.clear(juce::dontSendNotification);

    auto libraryDir = getLibraryDirectory();

    auto files = libraryDir.findChildFiles(
        juce::File::findFiles,
        false
    );

    int itemId = 1;

    for (auto& f : files)
    {
        if (f.getFileName() == ".gitkeep")
            continue;


        libraryFiles.add(f);
        libraryBox.addItem(f.getFileName(), itemId);

        ++itemId;
    }
}


bool MainComponent::loadTrack(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);

    if (reader == nullptr)
        return false;

    // Stop and remove the current track first
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    // Create the new audio source
    auto newSource =
        std::make_unique<juce::AudioFormatReaderSource>(
            reader,
            true
        );

    // Connect the new source to the transport
    transportSource.setSource(
        newSource.get(),
        0,
        nullptr,
        reader->sampleRate
    );

    // Store the source so it stays alive
    readerSource = std::move(newSource);

    // Restore current volume
    transportSource.setGain(
        static_cast<float>(
            volumeSlider.getValue() / 100.0
            )
    );

    // Reset playback state
    transportSource.setPosition(0.0);
    state = Stopped;

    playButton.setButtonText("Play");
    playButton.setColour(
        juce::TextButton::buttonColourId,
        juce::Colours::green
    );
    playButton.setEnabled(true);

    return true;
}



void MainComponent::trackSelected()
{
    auto index = libraryBox.getSelectedId() - 1;

    if (index >= 0 &&
        index < static_cast<int>(libraryFiles.size()))
    {
        if (!loadTrack(libraryFiles[index]))
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Playback failed",
                "That library file couldn't be read as audio."
            );
        }
    }
}


void MainComponent::openButtonClicked()
{
    auto libraryDirectory = getLibraryDirectory();

    // Allow the user to select any file
    chooser = std::make_unique<juce::FileChooser>(
        "Select an audio file...",
        libraryDirectory
    );

    auto fileChooserFlags =
        juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(
        fileChooserFlags,
        [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            // User cancelled the file chooser
            if (file == juce::File{})
                return;

            // Check the file extension
            auto extension = file.getFileExtension().toLowerCase();

   

            // test whether JUCE can actually read the file
            auto* testReader =
                formatManager.createReaderFor(file);

            if (testReader == nullptr)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Unsupported audio file",
                    "JUCE could not read this file as audio.\n\n"
                    "Please select a supported audio file."
                );

                return;
            }

            // test reader is no longer needed
            delete testReader;

            // copy the file into the library
            auto libraryDir = getLibraryDirectory();

            auto destFile =
                libraryDir.getNonexistentChildFile(
                    file.getFileNameWithoutExtension(),
                    file.getFileExtension()
                );

            if (!file.copyFileTo(destFile))
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Import failed",
                    "Couldn't copy that file into your library."
                );

                return;
            }

            // update the library list
            refreshLibraryList();

            // load the imported track
            if (!loadTrack(destFile))
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Playback failed",
                    "The file was copied to your library, "
                    "but couldn't be loaded for playback."
                );
            }
        }
    );
}



void MainComponent::playButtonClicked()
{
    if (state == Stopped)
        changeState(Starting);
    else if (state == Playing)
        changeState(Stopping);
}
