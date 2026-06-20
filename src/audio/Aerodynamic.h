#pragma once
#include "notes.h"

// Aerodynamic — Daft Punk
// BPM=116: eighth=258ms

static const MusNote AERODYNAMIC_NOTES[] = {
    // --- Bar 1: Bm ---
    {D4, 258, 0}, {Fb3, 258, 0},
    {B3, 258, 0}, {Fb3, 258, 0},
    // --- Bar 2: D ---
    {D4, 258, 0}, {Gb3, 258, 0},
    {B3, 258, 0}, {Gb3, 258, 0},
    // --- Bar 3: F#m ---
    {G4, 258, 0}, {B3,  258, 0},
    {E4, 258, 0}, {B3,  258, 0},
    // --- Bar 4: E ---
    {E4, 258, 0}, {A3,  258, 0},
    {Cb4,258, 0}, {A3,  258, 0},

    // --- Bar 5: Bm (variación) ---
    {D4, 258, 0}, {Fb4, 258, 0},
    {B3, 258, 0}, {Fb4, 258, 0},
    // --- Bar 6: D ---
    {D4, 258, 0}, {Gb3, 258, 0},
    {B3, 258, 0}, {Gb3, 258, 0},
    // --- Bar 7: F#m ---
    {G4, 258, 0}, {B3,  258, 0},
    {E4, 258, 0}, {B3,  258, 0},
    // --- Bar 8: E ---
    {E4, 258, 0}, {A3,  258, 0},
    {Cb4,258, 0}, {A3,  258, 0},
};

static const Song SONG_AERODYNAMIC = {
    "Aerodynamic",
    AERODYNAMIC_NOTES,
    (int)(sizeof(AERODYNAMIC_NOTES) / sizeof(AERODYNAMIC_NOTES[0]))
};
