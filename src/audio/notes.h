#pragma once
#include <Arduino.h>

// =====================================================
// NOTE FREQUENCIES (Hz)
// =====================================================

#define NOTE_REST  0
#define NOTE_B2   123
#define NOTE_C3   131
#define NOTE_CS3  139
#define NOTE_D3   147
#define NOTE_DS3  156
#define NOTE_E3   165
#define NOTE_F3   175
#define NOTE_FS3  185
#define NOTE_G3   196
#define NOTE_GS3  208
#define NOTE_A3   220
#define NOTE_AS3  233
#define NOTE_B3   247
#define NOTE_C4   262
#define NOTE_CS4  277
#define NOTE_D4   294
#define NOTE_DS4  311
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_FS4  370
#define NOTE_G4   392
#define NOTE_GS4  415
#define NOTE_A4   440
#define NOTE_AS4  466
#define NOTE_B4   494
#define NOTE_C5   523
#define NOTE_CS5  554
#define NOTE_D5   587
#define NOTE_DS5  622
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_FS5  740
#define NOTE_G5   784
#define NOTE_GS5  831
#define NOTE_A5   880
#define NOTE_AS5  932
#define NOTE_B5   988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976

// =====================================================
// SHORT ALIASES  (MIDI-to-Arduino convention)
// Natural notes : letter + octave        (C3, D3, E3 …)
// Sharps        : letter + 'b' + octave  (Cb3=C#3, Db3=D#3, Fb3=F#3, Gb3=G#3, Ab3=A#3)
// REST          : REST (silence)
// =====================================================

#define REST   NOTE_REST

// Octave 2
#define A2 110
#define B2     NOTE_B2

// Octave 3
#define C3     NOTE_C3
#define Cb3    NOTE_CS3
#define D3     NOTE_D3
#define Db3    NOTE_DS3
#define E3     NOTE_E3
#define F3     NOTE_F3
#define Fb3    NOTE_FS3
#define G3     NOTE_G3
#define Gb3    NOTE_GS3
#define A3     NOTE_A3
#define Ab3    NOTE_AS3
#define B3     NOTE_B3

// Octave 4
#define C4     NOTE_C4
#define Cb4    NOTE_CS4
#define D4     NOTE_D4
#define Db4    NOTE_DS4
#define E4     NOTE_E4
#define F4     NOTE_F4
#define Fb4    NOTE_FS4
#define G4     NOTE_G4
#define Gb4    NOTE_GS4
#define A4     NOTE_A4
#define Ab4    NOTE_AS4
#define B4     NOTE_B4

// Octave 5
#define C5     NOTE_C5
#define Cb5    NOTE_CS5
#define D5     NOTE_D5
#define Db5    NOTE_DS5
#define E5     NOTE_E5
#define F5     NOTE_F5
#define Fb5    NOTE_FS5
#define G5     NOTE_G5
#define Gb5    NOTE_GS5
#define A5     NOTE_A5
#define Ab5    NOTE_AS5
#define B5     NOTE_B5

// Octave 6
#define C6     NOTE_C6
#define Cb6    NOTE_CS6
#define D6     NOTE_D6
#define Db6    NOTE_DS6
#define E6     NOTE_E6
#define F6     NOTE_F6
#define Fb6    NOTE_FS6
#define G6     NOTE_G6
#define Gb6    NOTE_GS6
#define A6     NOTE_A6
#define Ab6    NOTE_AS6
#define B6     NOTE_B6

// =====================================================
// SONG DATA TYPES
// {freq, durationMs, gapMs}
// gapMs: silence after the note (0 = no gap)
// =====================================================

struct MusNote {
    uint16_t freq;
    uint16_t durationMs;
    uint16_t gapMs;
};

struct Song {
    const char*    name;
    const MusNote* notes;
    int            count;
};
