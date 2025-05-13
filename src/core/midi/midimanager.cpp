#include "midimanager.h"
#include "mididevice.h"
#include <cctype>
#include <regex>
#include <rtmidi/RtMidi.h>
#include <spdlog/spdlog.h>
#include <regex>
#include <algorithm>
#include <iostream>
#include <vector>


namespace {
    constexpr int VERIFICATION_TIMEOUT = 2;
}

MidiManager::MidiManager() {
}


MidiManager::~MidiManager() {
    m_devices.clear();

    if (_midiin.isPortOpen()) _midiin.closePort();
    if (_midiout.isPortOpen()) _midiout.closePort();
}


bool MidiManager::verifyIdentity(unsigned int inPort, unsigned int outPort,
                                 const std::vector<unsigned char>& targetId) 
{
    try {
        std::shared_ptr<MidiDevice> device = std::make_shared<MidiDevice>(inPort, outPort);

        m_devices.push_back(device);
        
        return true;
    } catch (RtMidiError &error) {
        error.printMessage();
    }
    return false;
}


bool MidiManager::portsMatch(std::string in, std::string out) {
    // Go to Uppercase
    std::transform(in.begin(), in.end(), in.begin(), ::toupper);
    std::transform(out.begin(), out.end(), out.begin(), ::toupper);
    
    //Remove index number at the end
    in = std::regex_replace(in, std::regex("\\d+$"), "");
    out = std::regex_replace(out, std::regex("\\d+$"), "");

    // Remove IN AND OUT from the names
    in = std::regex_replace(in, std::regex("IN"), "");
    out = std::regex_replace(out, std::regex("OUT"), "");

    return in == out;
}


void MidiManager::refresh() { 
    spdlog::debug("MidiManager: Refreshing Midi Devices");

    m_ports.clear();

    int inPortCount = _midiin.getPortCount();
    int outPortCount = _midiout.getPortCount();

    for (int i = 0; i < inPortCount; i++) {
        std::string inPortName = _midiin.getPortName(i);

        for (int j = 0; j < outPortCount; j++) {
            std::string outPortName = _midiout.getPortName(j);

            if (this->portsMatch(inPortName, outPortName)) {
                std::cout << inPortName << " " << outPortName << "\n";
                m_ports.push_back({i, j});

                spdlog::info("Ports Match! (" + std::to_string(i) + ", " + std::to_string(j) + ", " + inPortName + ")");
                if (this->verifyIdentity(i, j, {})) {
                }
            }
        }
    }
}
