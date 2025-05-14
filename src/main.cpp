#include <iostream>
#include <rtmidi/RtMidi.h>
#include <spdlog/spdlog.h>

#include "MidiManager.h"


int main() {
    spdlog::set_level(spdlog::level::debug);

    MidiManager midiManager;
    midiManager.setMidiCallback([](std::shared_ptr<MidiDevice> device, MidiMessage msg) {
        std::cout << device->name() << " : CALLED\n";
    });

    try {
        midiManager.refresh();

    } catch (const RtMidiError &error) {
        std::cerr << "MIDI Error: " << error.getMessage() << "\n";
        return 1;
    }

    std::cin.get();

    return 0;
}