#include <iostream>
#include <rtmidi/RtMidi.h>
#include <thread>

#include "core/midi/midimanager.h"

int main() {
    MidiManager midiManager;

    try {
        // Refresh MIDI devices (starts asynchronous verification)
        midiManager.refresh();

        int attempts = 0;
        const int maxAttempts = 5;

        while (attempts < maxAttempts) {
            midiManager.checkTimeouts();
            attempts++;

            if (!midiManager.getAvailableDevices().empty()) {
                std::cout << "There are available devices: " << midiManager.getAvailableDevices().size() << "\n";
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        

    } catch (const RtMidiError &error) {
        std::cerr << "MIDI Error: " << error.getMessage() << "\n";
        return 1;
    }

    return 0;
}