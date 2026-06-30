#pragma once
#include <Arduino.h>

// Protocolo compartido con el vehiculo ESP32-S3 (Cam Car).
// Debe coincidir exactamente con src/Protocol/Protocol.h del vehiculo.

enum CommandType : uint8_t {
    CMD_STOP     = 0,
    CMD_FORWARD  = 1,
    CMD_BACKWARD = 2,
    CMD_LEFT     = 3,
    CMD_RIGHT    = 4,
};

struct __attribute__((packed)) ControlPacket {
    uint8_t     magic;    // siempre CONTROL_MAGIC
    CommandType command;
    uint8_t     speed;    // 0-255, reservado
};

const uint8_t CONTROL_MAGIC = 0xC1;

// Marcador de fin de frame JPEG en el stream de video del vehiculo
const uint8_t FRAME_END_MARKER[4] = {0xFF, 0xAA, 0xFF, 0xAA};
