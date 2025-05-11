#include "midimanager.h"
#include <cctype>
#include <thread>
#include <chrono>
#include <regex>
#include <rtmidi/RtMidi.h>
#include <spdlog/spdlog.h>
#include <regex>
#include <algorithm>
#include <string>
#include <iomanip>
#include <iostream>


namespace {
    constexpr int VERIFICATION_TIMEOUT = 2;
}

MidiManager::MidiManager() = default;


MidiManager::~MidiManager() {
    m_activeSessions.clear();

    if (_midiin.isPortOpen()) _midiin.closePort();
    if (_midiout.isPortOpen()) _midiout.closePort();
}


void MidiManager::identityCallback(double deltaTime, std::vector<unsigned char> *message, void *userData) {
    auto* session = static_cast<ValidationSession*>(userData);

    if (session && message && !message->empty()) {
        std::cout << "OUT: " << session->outPort << "\n";
        session->manager->cleanupSession(session);
        std::cout << "Callback\n";
    }
}


bool MidiManager::verifyIdentity(unsigned int inPort, unsigned int outPort,
                                 const std::vector<unsigned char>& targetId) 
{
    try {
        auto it = std::find_if(m_activeSessions.begin(), m_activeSessions.end(), [inPort, outPort](const auto& session) {
            return session->inPort == inPort && session->outPort == outPort;
        });
        if (it != m_activeSessions.end()) {
            spdlog::warn("Verification already in progress for ports {} and {}", inPort, outPort);
            return false;
        }


        auto session = std::make_unique<ValidationSession>();
        session->inPort = inPort;
        session->outPort = outPort;
        session->targetId = targetId;
        session->midiIn = std::make_shared<RtMidiIn>();
        session->midiOut = std::make_shared<RtMidiOut>();
        session->manager = this;

        session->midiIn->openPort(inPort);
        session->midiOut->openPort(outPort);
        session->midiIn->setCallback(identityCallback, session.get());
        session->startTime = std::chrono::steady_clock::now();

        std::vector<unsigned char> sysex = {0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7};
        session->midiOut->sendMessage(&sysex);

        m_activeSessions.push_back(std::move(session));

        std::cout << "Verifying Identity\n";
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
                RtMidiIn in;
                RtMidiOut out;
                in.setErrorCallback([](RtMidiError::Type type, const std::string &msg, void *data) {
                    std::cout << msg << "\n";
                });
                in.setCallback([](double deltaTime, std::vector<unsigned char> *message, void *userData) {
                    for (auto m : *message) {
                        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)m << " ";
                    }
                    std::cout << "\n";
                });

                in.openPort(i);
                out.openPort(j);


                std::cout << in.isPortOpen() << " " << out.isPortOpen() << "\n";
                std::vector<unsigned char> sysex = {0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7};
                out.sendMessage(&sysex);

                std::this_thread::sleep_for(std::chrono::seconds(1));
                in.closePort();
                out.closePort();

                
                in.openPort(i);
                out.openPort(j);


                std::cout << in.isPortOpen() << " " << out.isPortOpen() << "\n";
                out.sendMessage(&sysex);

                std::this_thread::sleep_for(std::chrono::seconds(1));
                in.closePort();
                out.closePort();
                



                // m_ports.push_back({i, j});

                // spdlog::debug("Ports Match! (" + std::to_string(i) + ", " + std::to_string(j) + ", " + inPortName + ")");
                // if (this->verifyIdentity(i, j, {})) {
                // }
            }
        }
    }
}