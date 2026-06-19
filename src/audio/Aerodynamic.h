#pragma once
#include "notes.h"

static const MusNote AERODYNAMIC_NOTES[] = {
    // --- Bar 1: Bm ---
    {NOTE_D4,  DUR_EIGHTH}, {NOTE_F3,   DUR_EIGHTH},
    {NOTE_B3,  DUR_EIGHTH}, {NOTE_F3,   DUR_EIGHTH},
    // --- Bar 2: D ---
    {NOTE_D4,  DUR_EIGHTH}, {NOTE_GS3,  DUR_EIGHTH},
    {NOTE_B3,  DUR_EIGHTH}, {NOTE_GS3,  DUR_EIGHTH},
    // --- Bar 3: F#m ---
    {NOTE_G4,  DUR_EIGHTH}, {NOTE_B3,   DUR_EIGHTH},
    {NOTE_E4,  DUR_EIGHTH}, {NOTE_B3,   DUR_EIGHTH},
    // --- Bar 4: E ---
    {NOTE_E4,  DUR_EIGHTH}, {NOTE_A3,   DUR_EIGHTH},
    {NOTE_CS4, DUR_EIGHTH}, {NOTE_A3,   DUR_EIGHTH},

    // --- Bar 5: Bm (variación) ---
    {NOTE_D4,  DUR_EIGHTH}, {NOTE_FS4,  DUR_EIGHTH},
    {NOTE_B3,  DUR_EIGHTH}, {NOTE_FS4,  DUR_EIGHTH},
    // --- Bar 6: D ---
    {NOTE_D4,  DUR_EIGHTH}, {NOTE_GS3,  DUR_EIGHTH},
    {NOTE_B3,  DUR_EIGHTH}, {NOTE_GS3,  DUR_EIGHTH},
    // --- Bar 7: F#m ---
    {NOTE_G4,  DUR_EIGHTH}, {NOTE_B3,   DUR_EIGHTH},
    {NOTE_E4,  DUR_EIGHTH}, {NOTE_B3,   DUR_EIGHTH},
    // --- Bar 8: E ---
    {NOTE_E4,  DUR_EIGHTH}, {NOTE_A3,   DUR_EIGHTH},
    {NOTE_CS4, DUR_EIGHTH}, {NOTE_A3,   DUR_EIGHTH},
};

static const Song SONG_AERODYNAMIC = {
    "Aerodynamic",
    116,
    AERODYNAMIC_NOTES,
    (int)(sizeof(AERODYNAMIC_NOTES) / sizeof(AERODYNAMIC_NOTES[0]))
};
