cmake_minimum_required(VERSION 3.20)

project(
    MidiTest
    VERSION 1.0
    LANGUAGES CXX
)

find_package(RtMidi CONFIG REQUIRED)

add_executable(MidiTest src/main.cpp)

target_link_libraries(MidiTest PRIVATE RtMidi::rtmidi)