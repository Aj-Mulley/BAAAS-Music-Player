#pragma once

#include <JuceHeader.h>
//Functions Declarations 
class MainComponent : public juce::AudioAppComponent,
    public juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(
        const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void changeListenerCallback(
        juce::ChangeBroadcaster* source) override;

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    bool loadTrack(const juce::File& file);
    void changeState(TransportState newState);
    void openButtonClicked();
    void playButtonClicked();
    void trackSelected();

    juce::File getLibraryDirectory();
    void refreshLibraryList();

    // =========================================================
    // ONLINE MUSIC INFORMATION
    // =========================================================

    void fetchOnlineMusicInfo(
        juce::String title,
        juce::String artist,
        juce::String album,
        double duration);

    void updateMusicInfo(
        const juce::String& title,
        const juce::String& artist,
        const juce::String& album,
        const juce::String& lyrics,
        const juce::Image& artwork);

    juce::String parseArtistFromFileName(
        const juce::String& fileName);

    juce::String parseTitleFromFileName(
        const juce::String& fileName);


    // Heavy Metal Volume Control
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::ComboBox libraryBox;

    juce::Slider volumeSlider;
    juce::Label volumeLabel;


    // =========================================================
    // SONG INFORMATION UI
    // =========================================================

    juce::Label songTitleLabel;
    juce::Label artistLabel;
    juce::Label albumLabel;

    juce::TextButton lyricsButton;
    juce::TextEditor lyricsEditor;

    bool lyricsExpanded = false;

    juce::ImageComponent albumArtComponent;
    juce::Image albumArt;

    juce::Label onlineStatusLabel;


    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::FileChooser> chooser;

    juce::Array<juce::File> libraryFiles; //maps combobox item ids to files

    juce::ImageComponent soundIcon;

    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
