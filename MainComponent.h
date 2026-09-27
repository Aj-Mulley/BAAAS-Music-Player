#pragma once

#include <JuceHeader.h>
class MainComponent : public juce::AudioAppComponent,
    public juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState(TransportState newState);
    void openButtonClicked();
    void playButtonClicked();
    juce::File getLibraryDirectory();
    void refreshLibraryList();
    void trackSelected();

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::ComboBox libraryBox;

    // Heavy Metal Volume Control
    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    std::unique_ptr<juce::FileChooser> chooser;

    std::vector<juce::File> libraryFiles; //maps combobox item IDs to files

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};