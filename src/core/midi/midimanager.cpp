#include "midimanager.h"
#include "mididevice.h"
#include <cctype>
#include <chrono>
#include <regex>
#include <rtmidi/RtMidi.h>
#include <spdlog/spdlog.h>
#include <regex>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>


namespace {
    constexpr int VERIFICATION_TIMEOUT = 2;
}

MidiManager::MidiManager() {
    populateDeviceMap();
}


MidiManager::~MidiManager() {
    for (auto& session : m_activeSessions) {
        session->midiIn->cancelCallback();
        session->midiIn->closePort();
        session->midiOut->closePort();
    }

    m_activeSessions.clear();

    m_availableDevices.clear();

    if (_midiin.isPortOpen()) _midiin.closePort();
    if (_midiout.isPortOpen()) _midiout.closePort();
}


void MidiManager::identityCallback(double deltaTime, std::vector<unsigned char> *message, void *userData) {
    auto* session = static_cast<ValidationSession*>(userData);

    if (session && message && !message->empty()) {

        session->manager->handleIdentityResponse(session, message);

        session->manager->cleanupSession(session);
    }
}


void MidiManager::handleIdentityResponse(ValidationSession* session, std::vector<unsigned char> *message) {
    for (auto m : *message) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)m << " ";
    }
    std::cout << "\n";

    // The midi response given from the identity header
    //             ** --------
    // f0 7e 00 06 02 00 20 29 51 00 00 00 00 63 66 79 f7

}


bool MidiManager::verifyIdentity(unsigned int inPort, unsigned int outPort,
                                 const std::vector<unsigned char>& targetId) 
{
    try {
        std::shared_ptr<MidiDevice> device = std::make_shared<MidiDevice>(inPort, outPort);

        m_availableDevices.push_back(device);
        
        return true;
    } catch (RtMidiError &error) {
        error.printMessage();
    }
    return false;
}


void MidiManager::cleanupSession(ValidationSession* session) {
    auto it = std::find_if(m_activeSessions.begin(), m_activeSessions.end(),
                           [session](const auto& s) { return s.get() == session; });
    if (it != m_activeSessions.end()) {
        m_activeSessions.erase(it);
        std::cout << "Cleaned\n";
    }
    std::cout << it->get() << "\n";
}

void MidiManager::checkTimeouts() {
    auto now = std::chrono::steady_clock::now();
    auto it = m_activeSessions.begin();
    while (it != m_activeSessions.end()) {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - (*it)->startTime).count();
        if (duration > VERIFICATION_TIMEOUT) {
            (*it)->midiIn->cancelCallback();
            it = m_activeSessions.erase(it);
        } else {
            ++it;
        }
    }
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
    m_activeSessions.clear();

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





void MidiManager::populateDeviceMap() {
    for(const auto& [name, id] : MidiDeviceDB::KNOWN_DEVICES) {
        std::vector<unsigned char> key;
        key.insert(key.end(), id.manufacturerId.begin(), id.manufacturerId.end());
        key.insert(key.end(), id.deviceCode.begin(), id.deviceCode.end());
        m_deviceMap[key] = name;
    }
}