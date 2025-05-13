cmake_minimum_required(VERSION 3.20)

project(
    MidiTest
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

find_package(RtMidi CONFIG REQUIRED)
find_package(RtAudio CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(MidiTest 
    src/main.cpp

    src/core/midi/mididevice.cpp
    src/core/midi/midimanager.cpp
    src/core/midi/midimessage.cpp
)

target_link_libraries(MidiTest PRIVATE RtMidi::rtmidi RtAudio::rtaudio spdlog::spdlog)