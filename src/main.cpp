#include <iostream>
#include <rtmidi/RtMidi.h>
#include <spdlog/spdlog.h>
#include <thread>

#include "core/midi/midimanager.h"


int main() {
    MidiMessage message;
    //9f 47 7f
    message.status = 0x9F;
    message.key = 0x47;
    message.velocity = 0x7F;
    std::cout << "CHANNEL: " << (int)message.channel() << '\n';
    std::cout << "KEY: " << (int)message.type() << '\n';

    spdlog::set_level(spdlog::level::debug);

    MidiManager midiManager;

    try {
        // Refresh MIDI devices (starts asynchronous verification)
        midiManager.refresh();

        int attempts = 0;
        const int maxAttempts = 5;

        while (attempts < maxAttempts) {
            attempts++;

            if (!midiManager.getAvailableDevices().empty()) {
                for (int i = 0; i < midiManager.getAvailableDevices().size(); i++) {
                    std::cout << midiManager.getAvailableDevices().at(i)->is_available() << " ";
                }
                std::cout << '\n';
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        

    } catch (const RtMidiError &error) {
        std::cerr << "MIDI Error: " << error.getMessage() << "\n";
        return 1;
    }

    

    return 0;
}