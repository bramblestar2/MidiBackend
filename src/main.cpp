#include <iostream>
#include <rtmidi/RtMidi.h>
#include <spdlog/spdlog.h>

#include "Midi/Midi.h"


int main() {
    spdlog::set_level(spdlog::level::debug);

    MidiManager midiManager;
    midiManager.setMidiCallback([](MidiDevice* device, MidiMessage msg) {
        std::cout << device->name() << " : CALLED\n";
    });

    try {
        midiManager.refresh();

    } catch (const RtMidiError &error) {
        std::cerr << "MIDI Error: " << error.getMessage() << "\n";
        return 1;
    }

    midiManager.startRecording();

    std::cin.get();

    midiManager.stopRecording();

    for (auto &[name, recording] : midiManager.getRecordings()) {
        std::cout << name << " : " << recording.size() << "\n";
    }

    return 0;
}