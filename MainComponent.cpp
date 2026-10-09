#include "MainComponent.h"

MainComponent::MainComponent()
    : state(Stopped),
    lyricsExpanded(false)
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

    // =========================================================
    // SONG INFORMATION UI
    // =========================================================

    songTitleLabel.setText(
        "No song selected",
        juce::dontSendNotification
    );

    songTitleLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::white
    );

    songTitleLabel.setFont(
        juce::Font(
            juce::FontOptions()
            .withHeight(26.0f)
            .withStyle("Bold")
        )
    );

    songTitleLabel.setJustificationType(
        juce::Justification::centred
    );

    addAndMakeVisible(songTitleLabel);


    artistLabel.setText(
        "Artist",
        juce::dontSendNotification
    );

    artistLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::lightgrey
    );

    artistLabel.setFont(
        juce::Font(
            juce::FontOptions()
            .withHeight(18.0f)
        )
    );

    artistLabel.setJustificationType(
        juce::Justification::centred
    );

    addAndMakeVisible(artistLabel);


    albumLabel.setText(
        "Album",
        juce::dontSendNotification
    );

    albumLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::grey
    );

    albumLabel.setFont(
        juce::Font(
            juce::FontOptions()
            .withHeight(16.0f)
        )
    );

    albumLabel.setJustificationType(
        juce::Justification::centred
    );

    addAndMakeVisible(albumLabel);


    // =========================================================
    // ALBUM ART
    // =========================================================

    albumArtComponent.setImagePlacement(
        juce::RectanglePlacement::centred
        | juce::RectanglePlacement::onlyReduceInSize
    );

    addAndMakeVisible(albumArtComponent);


    // =========================================================
    // LYRICS TAB
    // =========================================================

    // Use ASCII characters only.
    // This avoids the JUCE ASCII assertion caused by Unicode arrows.

    lyricsButton.setButtonText(
        "+  LYRICS"
    );

    lyricsButton.setColour(
        juce::TextButton::buttonColourId,
        juce::Colours::black
    );

    lyricsButton.setColour(
        juce::TextButton::textColourOffId,
        juce::Colours::white
    );

    lyricsButton.setColour(
        juce::TextButton::buttonOnColourId,
        juce::Colours::darkgrey
    );

    lyricsButton.setColour(
        juce::TextButton::textColourOnId,
        juce::Colours::white
    );

    lyricsButton.onClick = [this]
        {
            lyricsExpanded = !lyricsExpanded;

            if (lyricsExpanded)
            {
                lyricsButton.setButtonText(
                    "-  LYRICS"
                );
            }
            else
            {
                lyricsButton.setButtonText(
                    "+  LYRICS"
                );
            }

            resized();
        };

    addAndMakeVisible(lyricsButton);


    lyricsEditor.setMultiLine(true);
    lyricsEditor.setReadOnly(true);
    lyricsEditor.setScrollbarsShown(true);
    lyricsEditor.setCaretVisible(false);
    lyricsEditor.setPopupMenuEnabled(false);

    lyricsEditor.setColour(
        juce::TextEditor::backgroundColourId,
        juce::Colours::black
    );

    lyricsEditor.setColour(
        juce::TextEditor::textColourId,
        juce::Colours::white
    );

    lyricsEditor.setColour(
        juce::TextEditor::outlineColourId,
        juce::Colours::darkgrey
    );

    lyricsEditor.setFont(
        juce::Font(
            juce::FontOptions()
            .withHeight(17.0f)
        )
    );

    lyricsEditor.setText(
        "Lyrics will appear here...",
        false
    );

    addAndMakeVisible(lyricsEditor);


    // =========================================================
    // ONLINE STATUS
    // =========================================================

    onlineStatusLabel.setText(
        "Online information: Ready",
        juce::dontSendNotification
    );

    onlineStatusLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::grey
    );

    onlineStatusLabel.setFont(
        juce::Font(
            juce::FontOptions()
            .withHeight(13.0f)
        )
    );

    onlineStatusLabel.setJustificationType(
        juce::Justification::centred
    );

    addAndMakeVisible(onlineStatusLabel);


    setSize(1100, 700);

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
                static_cast<float>(
                    volumeSlider.getValue() / 100.0
                    )
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
        juce::Font(
            juce::FontOptions()
            .withHeight(18.0f)
            .withStyle("Bold")
        )
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
    openButton.setBounds(
        10,
        10,
        80,
        30
    );

    playButton.setBounds(
        (getWidth() / 2) - 40,
        getHeight() - 100,
        80, 
        30
    );

    libraryBox.setBounds(
        190,
        10,
        300,
        30
    );


    // =========================================================
    // ALBUM ART
    // =========================================================

    albumArtComponent.setBounds(
        30,
        70,
        300,
        300
    );


    // =========================================================
    // LYRICS TAB
    // =========================================================

    lyricsButton.setBounds(
        30,
        380,
        300,
        35
    );


    if (lyricsExpanded)
    {
        lyricsEditor.setVisible(true);

        lyricsEditor.setBounds(
            30,
            420,
            300,
            juce::jmax(
                100,
                getHeight() - 440
            )
        );
    }
    else
    {
        lyricsEditor.setVisible(false);

        lyricsEditor.setBounds(
            30,
            420,
            300,
            0
        );
    }


    // =========================================================
    // SONG INFORMATION
    // =========================================================

    songTitleLabel.setBounds(
        350,
        75,
        500,
        40
    );

    artistLabel.setBounds(
        350,
        120,
        500,
        30
    );

    albumLabel.setBounds(
        350,
        155,
        500,
        30
    );


    // =========================================================
    // ONLINE STATUS
    // =========================================================

    onlineStatusLabel.setBounds(
        350,
        getHeight() - 55,
        500,
        25
    );


    // =========================================================
    // VOLUME
    // =========================================================

    playButton.setBounds(
        (getWidth() / 2) - 40,
        getHeight() - 100,
        80, 
        30
    );

    volumeSlider.setBounds(getWidth() - 90,
        getHeight() - 360, 
        80, 
        190
    );

    soundIcon.setBounds(
        getWidth() - 65,
        getHeight() - 395,
        30,
        30
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


void MainComponent::changeState(
    TransportState newState)
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

        libraryBox.addItem(
            f.getFileName(),
            itemId
        );

        ++itemId;
    }
}


// =============================================================
// PARSE ARTIST FROM FILE NAME
// =============================================================

juce::String MainComponent::parseArtistFromFileName(
    const juce::String& fileName)
{
    auto separator =
        fileName.indexOf(" - ");

    if (separator > 0)
    {
        return fileName.substring(
            0,
            separator
        ).trim();
    }

    return {};
}


// =============================================================
// PARSE TITLE FROM FILE NAME
// =============================================================

juce::String MainComponent::parseTitleFromFileName(
    const juce::String& fileName)
{
    auto separator =
        fileName.indexOf(" - ");

    if (separator >= 0)
    {
        return fileName.substring(
            separator + 3
        ).trim();
    }

    return fileName.trim();
}


bool MainComponent::loadTrack(
    const juce::File& file)
{
    auto* reader =
        formatManager.createReaderFor(file);

    if (reader == nullptr)
        return false;


    // =========================================================
    // READ EMBEDDED SONG INFORMATION
    // =========================================================

    juce::String title =
        reader->metadataValues["title"];

    juce::String artist =
        reader->metadataValues["artist"];

    juce::String album =
        reader->metadataValues["album"];


    if (title.isEmpty())
        title = reader->metadataValues["TITLE"];

    if (artist.isEmpty())
        artist = reader->metadataValues["ARTIST"];

    if (album.isEmpty())
        album = reader->metadataValues["ALBUM"];


    // =========================================================
    // FALL BACK TO FILE NAME
    // =========================================================

    if (title.isEmpty())
        title =
        parseTitleFromFileName(
            file.getFileNameWithoutExtension()
        );

    if (artist.isEmpty())
        artist =
        parseArtistFromFileName(
            file.getFileNameWithoutExtension()
        );


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


    // =========================================================
    // SHOW LOCAL INFORMATION IMMEDIATELY
    // =========================================================

    songTitleLabel.setText(
        title.isNotEmpty()
        ? title
        : "Searching...",
        juce::dontSendNotification
    );

    artistLabel.setText(
        artist.isNotEmpty()
        ? artist
        : "Searching...",
        juce::dontSendNotification
    );

    albumLabel.setText(
        album.isNotEmpty()
        ? album
        : "Searching...",
        juce::dontSendNotification
    );


    lyricsEditor.setText(
        "Searching for lyrics...",
        false
    );


    albumArtComponent.setImage(
        juce::Image(),
        juce::RectanglePlacement::centred
        | juce::RectanglePlacement::onlyReduceInSize
    );


    onlineStatusLabel.setText(
        "Searching online...",
        juce::dontSendNotification
    );


    // =========================================================
    // SEARCH ONLINE
    // =========================================================

    fetchOnlineMusicInfo(
        title,
        artist,
        album,
        reader->lengthInSamples /
        reader->sampleRate
    );


    return true;
}


// =============================================================
// ONLINE MUSIC INFORMATION
// =============================================================

void MainComponent::fetchOnlineMusicInfo(
    juce::String title,
    juce::String artist,
    juce::String album,
    double duration)
{
    std::thread(
        [this, title, artist, album, duration]()
        {
            juce::String onlineTitle = title;
            juce::String onlineArtist = artist;
            juce::String onlineAlbum = album;

            juce::String lyrics;
            juce::Image artwork;


            // =================================================
            // ITUNES SEARCH
            // =================================================

            if (title.isNotEmpty())
            {
                juce::String searchTerm = title;

                if (artist.isNotEmpty())
                {
                    searchTerm =
                        artist + " " + title;
                }


                auto iTunesURL =
                    juce::URL(
                        "https://itunes.apple.com/search"
                    )
                    .withParameter(
                        "term",
                        searchTerm
                    )
                    .withParameter(
                        "media",
                        "music"
                    )
                    .withParameter(
                        "entity",
                        "song"
                    )
                    .withParameter(
                        "limit",
                        "5"
                    );


                auto iTunesOptions =
                    juce::URL::InputStreamOptions(
                        juce::URL::ParameterHandling::inAddress
                    )
                    .withConnectionTimeoutMs(
                        10000
                    )
                    .withNumRedirectsToFollow(
                        5
                    )
                    .withExtraHeaders(
                        "User-Agent: MetalMachine/1.0\r\n"
                    );


                std::unique_ptr<juce::InputStream>
                    iTunesStream(
                        iTunesURL.createInputStream(
                            iTunesOptions
                        )
                    );


                if (iTunesStream != nullptr)
                {
                    auto iTunesText =
                        iTunesStream
                        ->readEntireStreamAsString();


                    auto iTunesJson =
                        juce::JSON::parse(
                            iTunesText
                        );


                    if (auto* iTunesObject =
                        iTunesJson.getDynamicObject())
                    {
                        auto results =
                            iTunesObject->getProperty(
                                "results"
                            );


                        if (auto* resultArray =
                            results.getArray())
                        {
                            if (!resultArray->isEmpty())
                            {
                                auto bestResult =
                                    resultArray->getFirst();


                                if (auto* resultObject =
                                    bestResult.getDynamicObject())
                                {
                                    auto foundTitle =
                                        resultObject
                                        ->getProperty(
                                            "trackName"
                                        )
                                        .toString();

                                    auto foundArtist =
                                        resultObject
                                        ->getProperty(
                                            "artistName"
                                        )
                                        .toString();

                                    auto foundAlbum =
                                        resultObject
                                        ->getProperty(
                                            "collectionName"
                                        )
                                        .toString();

                                    auto artworkURL =
                                        resultObject
                                        ->getProperty(
                                            "artworkUrl100"
                                        )
                                        .toString();


                                    if (foundTitle.isNotEmpty())
                                        onlineTitle =
                                        foundTitle;

                                    if (foundArtist.isNotEmpty())
                                        onlineArtist =
                                        foundArtist;

                                    if (foundAlbum.isNotEmpty())
                                        onlineAlbum =
                                        foundAlbum;


                                    // =================================================
                                    // DOWNLOAD ALBUM ART
                                    // =================================================

                                    if (artworkURL.isNotEmpty())
                                    {
                                        artworkURL =
                                            artworkURL.replace(
                                                "100x100",
                                                "600x600"
                                            );


                                        auto artworkURLObject =
                                            juce::URL(
                                                artworkURL
                                            );


                                        auto artworkOptions =
                                            juce::URL::InputStreamOptions(
                                                juce::URL::ParameterHandling::inAddress
                                            )
                                            .withConnectionTimeoutMs(
                                                10000
                                            )
                                            .withNumRedirectsToFollow(
                                                5
                                            )
                                            .withExtraHeaders(
                                                "User-Agent: MetalMachine/1.0\r\n"
                                            );


                                        std::unique_ptr<
                                            juce::InputStream>
                                            imageStream(
                                                artworkURLObject
                                                .createInputStream(
                                                    artworkOptions
                                                )
                                            );


                                        if (imageStream != nullptr)
                                        {
                                            juce::MemoryBlock imageData;


                                            imageStream
                                                ->readIntoMemoryBlock(
                                                    imageData
                                                );


                                            artwork =
                                                juce::ImageFileFormat::loadFrom(
                                                    imageData.getData(),
                                                    imageData.getSize()
                                                );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }


            // =================================================
            // LRCLIB EXACT SEARCH
            // =================================================

            if (onlineTitle.isNotEmpty() &&
                onlineArtist.isNotEmpty())
            {
                auto lyricsURL =
                    juce::URL(
                        "https://lrclib.net/api/get"
                    )
                    .withParameter(
                        "track_name",
                        onlineTitle
                    )
                    .withParameter(
                        "artist_name",
                        onlineArtist
                    );


                if (onlineAlbum.isNotEmpty())
                {
                    lyricsURL =
                        lyricsURL.withParameter(
                            "album_name",
                            onlineAlbum
                        );
                }


                if (duration > 0.0 &&
                    duration <= 3600.0)
                {
                    lyricsURL =
                        lyricsURL.withParameter(
                            "duration",
                            juce::String(
                                juce::roundToInt(
                                    duration
                                )
                            )
                        );
                }


                auto lyricsOptions =
                    juce::URL::InputStreamOptions(
                        juce::URL::ParameterHandling::inAddress
                    )
                    .withConnectionTimeoutMs(
                        10000
                    )
                    .withNumRedirectsToFollow(
                        5
                    )
                    .withExtraHeaders(
                        "User-Agent: MetalMachine/1.0\r\n"
                    );


                std::unique_ptr<juce::InputStream>
                    lyricsStream(
                        lyricsURL.createInputStream(
                            lyricsOptions
                        )
                    );


                if (lyricsStream != nullptr)
                {
                    auto lyricsText =
                        lyricsStream
                        ->readEntireStreamAsString();


                    auto lyricsJson =
                        juce::JSON::parse(
                            lyricsText
                        );


                    if (auto* lyricsObject =
                        lyricsJson.getDynamicObject())
                    {
                        lyrics =
                            lyricsObject
                            ->getProperty(
                                "plainLyrics"
                            )
                            .toString();


                        if (lyrics.isEmpty())
                        {
                            lyrics =
                                lyricsObject
                                ->getProperty(
                                    "syncedLyrics"
                                )
                                .toString();
                        }
                    }
                }
            }


            // =================================================
            // LRCLIB BROAD SEARCH FALLBACK
            // =================================================

            if (lyrics.isEmpty() &&
                onlineTitle.isNotEmpty())
            {
                auto searchTerm =
                    onlineTitle;

                if (onlineArtist.isNotEmpty())
                {
                    searchTerm +=
                        " " + onlineArtist;
                }


                auto lyricsSearchURL =
                    juce::URL(
                        "https://lrclib.net/api/search"
                    )
                    .withParameter(
                        "q",
                        searchTerm
                    );


                auto lyricsSearchOptions =
                    juce::URL::InputStreamOptions(
                        juce::URL::ParameterHandling::inAddress
                    )
                    .withConnectionTimeoutMs(
                        10000
                    )
                    .withNumRedirectsToFollow(
                        5
                    )
                    .withExtraHeaders(
                        "User-Agent: MetalMachine/1.0\r\n"
                    );


                std::unique_ptr<juce::InputStream>
                    lyricsSearchStream(
                        lyricsSearchURL.createInputStream(
                            lyricsSearchOptions
                        )
                    );


                if (lyricsSearchStream != nullptr)
                {
                    auto lyricsSearchText =
                        lyricsSearchStream
                        ->readEntireStreamAsString();


                    auto lyricsSearchJson =
                        juce::JSON::parse(
                            lyricsSearchText
                        );


                    if (auto* resultArray =
                        lyricsSearchJson.getArray())
                    {
                        for (auto& result :
                            *resultArray)
                        {
                            if (auto* resultObject =
                                result.getDynamicObject())
                            {
                                auto resultTitle =
                                    resultObject
                                    ->getProperty(
                                        "trackName"
                                    )
                                    .toString();


                                auto resultArtist =
                                    resultObject
                                    ->getProperty(
                                        "artistName"
                                    )
                                    .toString();


                                bool titleMatches =
                                    resultTitle
                                    .equalsIgnoreCase(
                                        onlineTitle
                                    );


                                bool artistMatches =
                                    onlineArtist.isEmpty()
                                    ||
                                    resultArtist.equalsIgnoreCase(
                                        onlineArtist
                                    );


                                if (titleMatches &&
                                    artistMatches)
                                {
                                    lyrics =
                                        resultObject
                                        ->getProperty(
                                            "plainLyrics"
                                        )
                                        .toString();


                                    if (lyrics.isEmpty())
                                    {
                                        lyrics =
                                            resultObject
                                            ->getProperty(
                                                "syncedLyrics"
                                            )
                                            .toString();
                                    }


                                    if (lyrics.isNotEmpty())
                                        break;
                                }
                            }
                        }
                    }
                }
            }


            // =================================================
            // UPDATE JUCE UI ON MESSAGE THREAD
            // =================================================

            juce::MessageManager::callAsync(
                [this,
                onlineTitle,
                onlineArtist,
                onlineAlbum,
                lyrics,
                artwork]()
                {
                    updateMusicInfo(
                        onlineTitle,
                        onlineArtist,
                        onlineAlbum,
                        lyrics,
                        artwork
                    );
                }
            );
        }
    ).detach();
}


// =============================================================
// UPDATE MUSIC INFORMATION
// =============================================================

void MainComponent::updateMusicInfo(
    const juce::String& title,
    const juce::String& artist,
    const juce::String& album,
    const juce::String& lyrics,
    const juce::Image& artwork)
{
    songTitleLabel.setText(
        title.isNotEmpty()
        ? title
        : "Unknown Song",
        juce::dontSendNotification
    );


    artistLabel.setText(
        artist.isNotEmpty()
        ? artist
        : "Unknown Artist",
        juce::dontSendNotification
    );


    albumLabel.setText(
        album.isNotEmpty()
        ? album
        : "Unknown Album",
        juce::dontSendNotification
    );


    if (lyrics.isNotEmpty())
    {
        lyricsEditor.setText(
            lyrics,
            false
        );

        onlineStatusLabel.setText(
            "Online information loaded",
            juce::dontSendNotification
        );
    }
    else
    {
        lyricsEditor.setText(
            "Lyrics not found.",
            false
        );

        onlineStatusLabel.setText(
            "Song information loaded - lyrics unavailable",
            juce::dontSendNotification
        );
    }


    if (artwork.isValid())
    {
        albumArt = artwork;

        albumArtComponent.setImage(
            albumArt,
            juce::RectanglePlacement::centred
            | juce::RectanglePlacement::onlyReduceInSize
        );
    }
    else
    {
        albumArtComponent.setImage(
            juce::Image(),
            juce::RectanglePlacement::centred
        );
    }
}


void MainComponent::trackSelected()
{
    auto index =
        libraryBox.getSelectedId() - 1;

    if (index >= 0 &&
        index < static_cast<int>(
            libraryFiles.size()))
    {
        if (!loadTrack(
            libraryFiles[index]))
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
    auto libraryDirectory =
        getLibraryDirectory();

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
            auto extension =
                file.getFileExtension()
                .toLowerCase();


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
            auto libraryDir =
                getLibraryDirectory();

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
