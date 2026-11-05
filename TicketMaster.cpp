/*
  ==============================================================================

    JS.cpp
    Created: 5 Oct 2026 12:15:54pm
    Author:  ajmul

  ==============================================================================
*/




#include "TicketMaster.h"
#include <iostream>
#include <random>
#include <thread>


using namespace std;

void getTicketmasterEvent()
{
    const juce::String apiKey = "6QUJTGSQZ5r8h9ZeZpFEfl7udPUvFBC0";

    juce::URL url("https://app.ticketmaster.com/discovery/v2/events.json");

    url = url.withParameter("keyword", "concert");
    url = url.withParameter("countryCode", "US");
    url = url.withParameter("size", "50");
    url = url.withParameter("apikey", apiKey);

    auto saveFile = juce::File::getCurrentWorkingDirectory()
        .getChildFile("ticketmaster_event.json");

    std::thread([url, saveFile]()
        {
            auto stream = url.createInputStream(
                juce::URL::InputStreamOptions(
                    juce::URL::ParameterHandling::inAddress
                ).withConnectionTimeoutMs(10000)
            );

            if (stream == nullptr)
            {
                juce::Logger::writeToLog(
                    "Failed to connect to Ticketmaster."
                );
                return;
            }

            const auto response = stream->readEntireStreamAsString();
            auto json = juce::JSON::parse(response);

            if (!json.isObject())
            {
                juce::Logger::writeToLog("Invalid JSON received.");
                return;
            }

            // Get the events array from the API response.
            auto eventsObject = json.getProperty(
                "_embedded", juce::var()
            );

            if (!eventsObject.isObject())
            {
                juce::Logger::writeToLog("No events object found.");
                return;
            }

            auto events = eventsObject.getProperty(
                "events", juce::var()
            );

            if (!events.isArray() || events.size() == 0)
            {
                juce::Logger::writeToLog("No events found.");
                return;
            }

            // Randomly select one event.
            std::random_device rd;
            std::mt19937 generator(rd());

            std::uniform_int_distribution<int> distribution(
                0, static_cast<int>(events.size()) - 1
            );

            int randomIndex = distribution(generator);

            juce::var selectedEvent = events[randomIndex];

            // Ensure the destination directory exists.
            auto directory = saveFile.getParentDirectory();

            if (!directory.createDirectory().wasOk())
            {
                juce::Logger::writeToLog(
                    "Failed to create directory."
                );
                return;
            }

            // Load existing JSON or create a new array.
            juce::var savedEvents;

            if (saveFile.existsAsFile())
            {
                auto existingJson = juce::JSON::parse(
                    saveFile.loadFileAsString()
                );

                if (existingJson.isArray())
                {
                    savedEvents = existingJson;
                }
                else
                {
                    juce::Logger::writeToLog(
                        "Existing JSON is invalid or is not an array."
                    );
                    return;
                }
            }
            else
            {
                savedEvents = juce::Array<juce::var>();
            }

            // Append the newly selected event.
            savedEvents.getArray()->add(selectedEvent);

            // Serialize the complete array as formatted JSON.
            juce::String updatedJson =
                juce::JSON::toString(savedEvents, true);

            // Save all events, including the newly appended event.
            if (saveFile.replaceWithText(updatedJson))
            {
                juce::Logger::writeToLog(
                    "Concert appended successfully: "
                    + selectedEvent.getProperty(
                        "name", juce::var()
                    ).toString()
                );

                ParseJSON();
            }
            else
            {
                juce::Logger::writeToLog(
                    "Failed to save event JSON."
                );
            }

        }).detach();
}

void ParseJSON()
{
    auto saveFile = juce::File::getCurrentWorkingDirectory()
        .getChildFile("ticketmaster_event.json");

    if (!saveFile.existsAsFile())
    {
        juce::Logger::writeToLog(
            "Cannot clean JSON because ticketmaster_event.json was not found."
        );
        return;
    }

    auto json = juce::JSON::parse(saveFile.loadFileAsString());

    if (!json.isArray())
    {
        juce::Logger::writeToLog(
            "Cannot clean JSON because the file is not an array."
        );
        return;
    }

    juce::Array<juce::var> cleanedEvents;

    for (const auto& event : *json.getArray())
    {
        if (!event.isObject())
            continue;

        auto cleanedEvent = juce::var(new juce::DynamicObject());
        auto* cleanedObject = cleanedEvent.getDynamicObject();

        cleanedObject->setProperty("name", event.getProperty("name", juce::var()));
        cleanedObject->setProperty("artist", juce::var());
        cleanedObject->setProperty("date", juce::var());
        cleanedObject->setProperty("venue", juce::var());
        cleanedObject->setProperty("city", juce::var());
        cleanedObject->setProperty("url", event.getProperty("url", juce::var()));
        cleanedObject->setProperty("image", juce::var());

        auto embedded = event.getProperty("_embedded", juce::var());
        auto attractions = embedded.getProperty("attractions", juce::var());

        if (attractions.isArray() && attractions.size() > 0)
        {
            cleanedObject->setProperty(
                "artist", attractions[0].getProperty("name", juce::var())
            );
        }

        auto start = event.getProperty("dates", juce::var())
            .getProperty("start", juce::var());

        cleanedObject->setProperty(
            "date", start.getProperty("localDate", juce::var())
        );

        auto venues = embedded.getProperty("venues", juce::var());

        if (venues.isArray() && venues.size() > 0)
        {
            auto venue = venues[0];

            cleanedObject->setProperty(
                "venue", venue.getProperty("name", juce::var())
            );
            cleanedObject->setProperty(
                "city", venue.getProperty("city", juce::var())
                    .getProperty("name", juce::var())
            );
        }

        auto images = event.getProperty("images", juce::var());

        if (images.isArray() && images.size() > 0)
        {
            cleanedObject->setProperty(
                "image", images[0].getProperty("url", juce::var())
            );
        }

        cleanedEvents.add(cleanedEvent);
    }

    juce::String cleanedJson = "[\n";

    for (int i = 0; i < cleanedEvents.size(); ++i)
    {
        cleanedJson += "    ";
        cleanedJson += juce::JSON::toString(cleanedEvents[i], false);

        if (i < cleanedEvents.size() - 1)
            cleanedJson += ",";

        cleanedJson += "\n";
    }

    cleanedJson += "]";

    if (saveFile.replaceWithText(cleanedJson))
    {
        juce::Logger::writeToLog(
            "ticketmaster_event.json cleaned successfully."
        );
    }
    else
    {
        juce::Logger::writeToLog(
            "Failed to save cleaned ticketmaster_event.json."
        );
    }
}



