#include "midimanager.h"
#include <cctype>
#include <regex>
#include <spdlog/spdlog.h>
#include <regex>
#include <algorithm>
#include <string>

MidiManager::MidiManager() {

}


MidiManager::~MidiManager() {
    
}


void MidiManager::identityCallback(double deltaTime, std::vector<unsigned char> *message, void *userData) {

}


bool MidiManager::verifyIdentity(unsigned int inPort, unsigned int outPort,
                                 const std::vector<unsigned char>& targetId) 
{
    try {
        RtMidiIn* midiIn = new RtMidiIn;
        RtMidiOut* midiOut = new RtMidiOut;
        
        midiIn->openPort(inPort);
        midiOut->openPort(outPort);

        midiIn->ignoreTypes(false, true, true);

        std::pair<MidiManager*, int> callbackData(this, inPort);

        midiIn->setCallback([](double deltaTime, std::vector<unsigned char> *message, void *userData) -> void {
            auto* self = static_cast<std::pair<MidiManager*, int>*>(userData);
            
            // for (int i = 0; i < self->m_ports.size(); i++) {
            //     std::cout << self->m_ports.at(i).first << " - " << self->m_ports.at(i).second << '\n';
            // }

            std::cout << "IN: " << self->second << "\n";


            for (int i = 0; i < message->size(); i++) {
                std::cout << (int)message->at(i) << " ";
            }

            std::cout << "\n";
        }, &callbackData);
        // SysEx: Identity Request
        std::vector<unsigned char> sysex = { 0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7 };
        
        midiOut->sendMessage(&sysex);

        midiIn->cancelCallback();
        midiIn->closePort();
        midiOut->closePort();
        delete midiIn;
        delete midiOut;
    } catch (RtMidiError &error) {
        error.printMessage();
    }
    return false;
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
    spdlog::info("MidiManager: Refreshing Midi Devices");

    int inPortCount = _midiin.getPortCount();
    int outPortCount = _midiout.getPortCount();

    for (int i = 0; i < inPortCount; i++) {
        std::string inPortName = _midiin.getPortName(i);

        for (int j = 0; j < outPortCount; j++) {
            std::string outPortName = _midiout.getPortName(j);

            if (this->portsMatch(inPortName, outPortName)) {
                m_ports.push_back({i, j});

                spdlog::info("Ports Match! (" + std::to_string(i) + ", " + std::to_string(j) + ", " + inPortName + ")");
                if (this->verifyIdentity(i, j, {})) {
                    std::cout << "IDENTITY VERIFIED\n";
                }
            }
        }
    }
}