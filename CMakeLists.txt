cmake_minimum_required(VERSION 3.20)

project(
    MidiManager
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

find_package(RtMidi CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# add_executable
add_library(MidiManager 
    # src/main.cpp

    src/Midi/Midi.h
    src/Midi/mididevice.h src/Midi/mididevice.cpp
    src/Midi/midimanager.h src/Midi/midimanager.cpp
    src/Midi/midimessage.h src/Midi/midimessage.cpp
)

target_include_directories(MidiManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(MidiManager PRIVATE RtMidi::rtmidi spdlog::spdlog)


add_executable(test src/main.cpp)
target_link_libraries(test PRIVATE MidiManager RtMidi::rtmidi spdlog::spdlog)