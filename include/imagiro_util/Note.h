#pragma once

#include <iostream>
#include <string>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

const std::string notes = "C C#D D#E F F#G G#A A#B ";

class Note {
public:
    static std::string midiNumberToNote(int number) {
        if (number < 0) return "";
        int octave = number / 12 - 2;
        auto start = static_cast<unsigned long>((number % 12) * 2);
        std::string note = notes.substr(start, 2);
        if (note[1] == ' ') note = note[0];
        return note + std::to_string(octave);
    }
        
    static int noteToMidiNumber(const juce::String& note) {
		if (note.length() == 0) return -1;
		if (!isalpha(note[0])) return -1;
        return noteToMidiNumber(note.toStdString());
    }
    
    static int noteToMidiNumber(std::string note) {
		if (note.empty()) return -1;
        int octave = note.back() - '0';
        note = note.substr(0, note.size() - 1);

		auto notePos = notes.find(note);
		if (notePos == std::string::npos) return -1;

        int noteNum = (int) notePos / 2;
        
        return (octave + 2) * 12 + noteNum;
    }

};
