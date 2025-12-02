#pragma once

#include "imgui.h"
#include <vector>
#include <array>
#include <deque>
#include <mutex>
#include <cmath>

// NES APU channel info for piano visualization
struct NesNoteInfo {
    int channel;        // 0-4: Square1, Square2, Triangle, Noise, DMC
    int midi_note;      // MIDI note number (0-127)
    float velocity;     // 0.0 - 1.0
    bool active;        // Is the note currently playing
};

// Piano roll note event
struct PianoRollNote {
    int channel;
    int midi_note;
    float velocity;
    float start_time;   // In seconds
    float duration;     // In seconds, 0 if still playing
    bool active;
};

// Channel colors for piano visualization
inline const ImU32 PianoChannelColors[] = {
    IM_COL32(255, 80, 80, 220),   // Square 1 - Red
    IM_COL32(255, 160, 60, 220),  // Square 2 - Orange
    IM_COL32(80, 180, 255, 220),  // Triangle - Blue
    IM_COL32(230, 80, 230, 220),  // Noise - Magenta
    IM_COL32(230, 230, 80, 220)   // DMC - Yellow
};

inline const char* PianoChannelNames[] = {
    "Sq1", "Sq2", "Tri", "Noi", "DMC"
};

class PianoVisualizer {
public:
    PianoVisualizer();
    ~PianoVisualizer() = default;

    // Reset state
    void reset();

    // Update with detected frequencies from audio analysis
    // frequencies: array of 5 frequencies (one per channel), 0 if silent
    // amplitudes: array of 5 amplitudes (0.0 - 1.0)
    void updateFromFrequencies(const float* frequencies, const float* amplitudes, float current_time);

    // Update with NES APU register data directly
    // periods: array of 5 period values from APU oscillators
    // lengths: array of 5 length counter values (0 = silent)
    // amplitudes: array of 5 amplitude values
    void updateFromAPU(const int* periods, const int* lengths, const int* amplitudes, float current_time);

    // Update with audio data for frequency detection (fallback)
    void updateFromAudio(const short* samples, int sample_count, long sample_rate, float current_time);

    // Draw the piano keyboard
    void drawPianoKeyboard(const char* label, float width, float height);

    // Draw the piano roll (scrolling notes)
    void drawPianoRoll(const char* label, float width, float height, float current_time);

    // Draw complete piano visualizer window
    void drawPianoWindow(bool* p_open, float current_time);

    // Settings
    void setPianoRollSpeed(float seconds_visible) { piano_roll_seconds_ = seconds_visible; }
    void setOctaveRange(int low, int high) { octave_low_ = low; octave_high_ = high; }

private:
    // Constants
    static constexpr int NUM_CHANNELS = 5;
    static constexpr int MIDI_NOTE_MIN = 21;   // A0
    static constexpr int MIDI_NOTE_MAX = 108;  // C8
    static constexpr float NES_CPU_CLOCK = 1789773.0f;  // NTSC

    // Current note state per channel
    std::array<NesNoteInfo, NUM_CHANNELS> current_notes_;
    
    // Piano roll history
    std::deque<PianoRollNote> piano_roll_notes_;
    static constexpr int MAX_ROLL_NOTES = 2000;
    
    // Previous state for note tracking
    std::array<int, NUM_CHANNELS> prev_midi_notes_;
    std::array<float, NUM_CHANNELS> note_start_times_;
    
    // Settings
    float piano_roll_seconds_ = 4.0f;  // How many seconds of notes to show
    int octave_low_ = 2;   // C2
    int octave_high_ = 7;  // C7
    
    // Thread safety
    std::mutex mutex_;

    // FFT for frequency detection
    static constexpr int FFT_SIZE = 4096;
    std::vector<float> fft_buffer_;
    
    // Helper functions
    static int frequencyToMidi(float frequency);
    static float midiToFrequency(int midi_note);
    static bool isBlackKey(int midi_note);
    static int getWhiteKeyIndex(int midi_note);  // Index among white keys
    static int getOctave(int midi_note);
    static int getNoteInOctave(int midi_note);   // 0-11
    
    void drawKey(ImDrawList* draw_list, ImVec2 pos, float width, float height, 
                 int midi_note, bool is_black, int pressed_channel, float velocity);
    
    void processNoteChange(int channel, int new_midi_note, float velocity, float current_time);
    
    // Detect dominant frequency from audio samples
    float detectFrequency(const float* samples, int count, long sample_rate);
};

