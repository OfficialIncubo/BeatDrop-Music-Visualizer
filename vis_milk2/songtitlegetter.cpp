#include "songtitlegetter.h"

SongTitleGetter::SongTitleGetter()
{
}

void SongTitleGetter::Init() {
    winrt::init_apartment(); // Initialize the WinRT runtime
    start_time = std::chrono::steady_clock::now();
}

void SongTitleGetter::PollMediaInfo() {

    if (!doPoll && !doPollExplicit) return;

    // Get the current time
    auto current_time = std::chrono::steady_clock::now();

    // Calculate the elapsed time in seconds
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

    if (elapsed_seconds >= 0 || doPollExplicit) {

        auto smtcManager = winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        auto currentSession = smtcManager.GetCurrentSession();
        updated = false;
        if (currentSession) {
            auto properties = currentSession.TryGetMediaPropertiesAsync().get();
            if (properties) {
                if (doPollExplicit || properties.Artist().c_str() != currentArtist || properties.Title().c_str() != currentTitle) {
                    isSongChange = currentArtist.length() || currentTitle.length();
                    currentArtist = properties.Artist().c_str();
                    currentTitle = properties.Title().c_str();

                    updated = true;
                }
            }
        }
        else {
            if (currentArtist.length() || currentTitle.length()) {
                currentArtist = L"";
                currentTitle = L"";
                updated = true;
            }
        }

        // Reset the start time to the current time
        start_time = current_time;
    }
}