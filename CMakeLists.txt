cmake_minimum_required(VERSION 3.20)

project(
    MidiTest
    VERSION 1.0
    LANGUAGES CXX
)

find_package(RtMidi CONFIG REQUIRED)
find_package(RtAudio CONFIG REQUIRED)

add_executable(MidiTest 
    src/main.cpp

    src/core/midi/mididevice.cpp
    src/core/midi/midimanager.cpp
)

target_link_libraries(MidiTest PRIVATE RtMidi::rtmidi)
target_link_libraries(MidiTest PRIVATE RtAudio::rtaudio)