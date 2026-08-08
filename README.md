# ESP32 Modular Tool

Consola portátil multiuso para ESP32 con pantalla circular GC9A01A (240x240) y 8 botones.
Sistema modular con menú por secciones: juegos, herramientas, reproductor de música,
teclado/mando Bluetooth LE y control de dispositivos por ESP-NOW.

## Menú

El menú tiene 6 secciones. `LEFT`/`RIGHT` cambian de sección, `UP`/`DOWN` cambian de ítem,
`OK` entra y `MENU` vuelve atrás desde cualquier ítem.

### Info
- **Version** — versión del firmware
- **Creditos**

### Juegos
| Juego | Descripción |
|-------|-------------|
| **Snake** | Serpiente clásica en rejilla 22x16 |
| **Simon** | Secuencias de color y sonido |
| **Tetris** | Rejilla 20x10 con pieza siguiente y hold |
| **2048** | Rejilla 4x4 deslizante |
| **Pong** | 1 jugador contra la IA |
| **Breakout** | Rompeladrillos con vidas y puntuación |
| **Flappy Bird** | Un botón, tuberías infinitas |
| **Invaders** | Space Invaders con disparos de ambos lados |
| **Buscaminas** | Rejilla 9x9 con banderas |
| **DOOM** | Raycaster 3D con enemigos, munición y vida |

### Herramientas
- **Cronometro** — cronómetro con vueltas (hasta 10)
- **Dado** — dado configurable (d2 a d100)
- **Canvas** — pintura por píxeles en rejilla 20x20 con 9 colores
- **Temporizador** — cuenta atrás con pausa y aviso sonoro
- **Morse** — codificador/decodificador de morse con buzzer
- **Synth** — sintetizador: modo TOCAR (selector circular de nota + octava) y
  modo SECUENCIA (secuenciador de hasta 8 pasos en bucle, con tempo en BPM)
- **Linterna** — LED WS2812B con 8 colores y brillo ajustable
- **Sirena** — 3 patrones de luz + sonido (Policía, Wail, Yelp)

### Musica
Reproductor sobre el buzzer con visualización de progreso (play/pausa, bucle):
Aerodynamic · Get Lucky · Harder Better Faster Stronger · Never Gonna Give You Up ·
Kids · Adults Are Talking · Outer Wilds · Riptide.

### Mis Dispositivos
- **Cam Car** — control por ESP-NOW de un vehículo ESP32-S3 con cámara. Envía comandos
  de dirección y recibe el vídeo (frames JPEG) que se dibuja a pantalla completa con
  overlay de comando y FPS.

### Bluetooth
La radio BLE se enciende al entrar al ítem y se apaga al salir con `MENU`, para no gastar
batería. El dispositivo se anuncia como **ESP32 Tool** (HID genérico).

- **Teclado WASD** — teclado BLE HID: `UP/LEFT/DOWN/RIGHT` → W/A/S/D, más ESC, ENTER y
  SPACE. Las teclas se envían sostenidas (mantener = mantener pulsado en el host).
- **Musica** — control multimedia (Consumer Control): `UP/DOWN` volumen con repetición al
  mantener, `LEFT/RIGHT` pista anterior/siguiente, `OK` play/pausa.
- **Olvidar vinculos** — borra los emparejamientos guardados. Necesario cuando cambia el
  descriptor HID: el host cachea los servicios y no los relee, así que hay que romper el
  vínculo por ambos lados.

## Hardware

| Componente | Pin |
|------------|-----|
| TFT CS     | 5   |
| TFT RST    | 22  |
| TFT DC     | 21  |
| TFT MOSI   | 23  |
| TFT SCLK   | 18  |
| TFT MISO   | -1  |
| BTN MENU   | 16  |
| BTN A      | 4   |
| BTN B      | 15  |
| BTN UP     | 27  |
| BTN DOWN   | 13  |
| BTN LEFT   | 14  |
| BTN RIGHT  | 26  |
| BTN OK     | 12  |
| BUZZER     | 25  |
| LED WS2812B| 32  |

Los botones usan `INPUT_PULLUP` (activo a nivel bajo). Los pines se definen en
[GlobalConfig.h](src/GlobalConfig/GlobalConfig.h), junto con la MAC del vehículo
Cam Car (`CAMCAR_MAC`).

## Estructura del proyecto

```
src/
├── main.cpp                  # Setup y loop
├── GlobalConfig/             # Orquestación: menú, navegación, cableado de módulos
├── ScreenManager/            # Todo el renderizado (solo depende de las libs de display)
├── ButtonManager/            # Antirrebote y callbacks de botones
├── SpeakerManager/           # Reproducción de notas/canciones sobre LEDC
├── LinternaManager/          # LED WS2812B (linterna y sirena)
├── BluetoothManager/         # Radio BLE + perfiles HID (teclado, consumer control)
│   └── HidKeys.h             # Códigos de uso HID (teclas y multimedia)
├── EspNowManager/            # Radio ESP-NOW: comandos + recepción de vídeo JPEG
├── EspNowLedManager/         # Control de tira LED remota por ESP-NOW (sin usar en el menú)
├── Protocol/                 # Protocolo compartido con el vehículo Cam Car
├── audio/                    # Partituras (notes.h + una cabecera por canción)
├── SnakeGame/  SimonGame/  TetrisGame/  Game2048/
├── PongGame/   BreakoutGame/ FlappyGame/ InvadersGame/
├── MinesweeperGame/ DoomGame/
├── CronoManager/ TimerManager/ DiceManager/ CanvasManager/ MorseCode/
```

**Arquitectura**: cada módulo expone solo estado y lógica mediante getters `const`, sin
dibujar nada. `ScreenManager` concentra todos los métodos `draw*()`. `GlobalConfig` lee el
estado de los módulos y se lo pasa a `ScreenManager` para renderizar, y conecta los
callbacks de botones de cada ítem.

Las radios (BLE y ESP-NOW) siguen el mismo criterio: `begin()` al entrar al ítem,
`end()` al salir, para no consumir batería en reposo.

## Build

Proyecto [PlatformIO](https://platformio.org/) sobre Arduino framework, placa `esp32dev`.
Usa la partición `huge_app.csv` porque el binario con BLE + todos los módulos no cabe en
el esquema por defecto.

```bash
pio run
```

```bash
pio run --target upload
```

```bash
pio device monitor
```

## Dependencias

- Adafruit GFX Library
- Adafruit GC9A01A
- GFX Library for Arduino
- TJpg_Decoder (decodificación del vídeo del Cam Car)
- FastLED (LED WS2812B)
