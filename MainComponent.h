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


    // Heavy Metal Volume Control
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::ComboBox libraryBox;

    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::FileChooser> chooser;

    juce::Array<juce::File> libraryFiles; //maps combobox item ids to files

    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
