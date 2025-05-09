#include <iostream>
#include <rtmidi/RtMidi.h>

#include "core/midi/midimanager.h"

int main() {
    std::cout << "Hello World!\n";

    MidiManager manager;
    manager.refresh();

    return 0;
}