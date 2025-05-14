cmake_minimum_required(VERSION 3.20)

project(
    MidiManager
    VERSION 1.0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

find_package(RtMidi CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(MidiManager 
    src/main.cpp

    src/MidiManager.h
    src/core/mididevice.h src/core/mididevice.cpp
    src/core/midimanager.h src/core/midimanager.cpp
    src/core/midimessage.h src/core/midimessage.cpp
)

target_link_libraries(MidiManager PRIVATE RtMidi::rtmidi spdlog::spdlog)