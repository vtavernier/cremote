#pragma once

#define LINE_BUFFER_SIZE 64
#ifndef BLE_VERBOSE
#define BLE_VERBOSE 0
#endif

#if BLE_VERBOSE
#define BLE_DEBUG(msg) do { Serial.println(F(msg)); } while (0)
#else
#define BLE_DEBUG(msg)
#endif

#define BTH_RST 7
#define BTH_RX 8
#define BTH_TX 9

#define OUTPUT_1 6
#define OUTPUT_2 5
#define BTN_TRIGGER 2

#define REBOUND_MS 25
#define MAX_PROGRAM_STEPS 34
#define DEFAULT_HALFPRESS_DELAY 200
