# ESP32 Modular Tool

Multifunctional tool for ESP32 with GC9A01A circular display. Modular menu-driven system with games and utilities.

## Features

- **Snake** — classic snake game on a 22x16 grid
- **Cronómetro** — stopwatch with lap times (up to 10)
- **Temporizador** — countdown timer with pause
- **Dado** — configurable dice (d2 to d100)
- **Canvas** — pixel paint with 9 colors
- **Test buttons** — verify each button works
- **Display tests** — fill screen with red/green/blue
- **JPEG viewer** — display JPEG images via TJpg_Decoder

## Hardware

| Component | Pin |
|-----------|-----|
| TFT CS    | 5   |
| TFT RST   | 22  |
| TFT DC    | 21  |
| TFT MOSI  | 23  |
| TFT SCLK  | 18  |
| TFT MISO  | -1  |
| BTN MENU  | 16  |
| BTN A     | 4   |
| BTN B     | 15  |
| BTN UP    | 27  |
| BTN DOWN  | 13  |
| BTN LEFT  | 14  |
| BTN RIGHT | 26  |
| BTN OK    | 12  |

## Project Structure

```
src/
├── main.cpp                  # Setup and loop
├── ButtonManager/            # Button debounce and callbacks
├── ScreenManager/            # All rendering (depends on display libs only)
├── GlobalConfig/             # Orchestration, menu, navigation, wiring
├── SnakeGame/                # Snake game logic (no rendering)
├── CronoManager/             # Stopwatch logic (no rendering)
├── TimerManager/             # Countdown timer logic (no rendering)
├── DiceManager/              # Dice logic (no rendering)
└── CanvasManager/            # Paint logic (no rendering)
```

Architecture: each `*Manager` exposes only state and logic via `const` getters. `ScreenManager` holds all `draw*()` methods. `GlobalConfig` reads manager state and passes it to `ScreenManager` for rendering.

## Build

Built with [PlatformIO](https://platformio.org/).

```bash
pio run
pio upload
pio device monitor
```

## Dependencies

- Adafruit GFX Library
- Adafruit GC9A01A
- GFX Library for Arduino (TFT_eSPI compatible)
- TJpg_Decoder
