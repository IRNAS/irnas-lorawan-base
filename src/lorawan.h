#ifndef LORAWAN_PROJECT_H
#define LORAWAN_PROJECT_H

#include "Arduino.h"
#include <LoRaWAN.h>
#include <STM32L0.h>
#include "settings.h"
#include <TimerMillis.h>
#include "stm32l0_eeprom.h"

extern bool lorawan_send_successful;
extern bool lorawan_settings_new;
extern uint8_t lorawan_settings_buffer[256];
extern size_t lorawan_settings_length;

bool lorawan_init(void);
bool lorawan_send(uint8_t port, const uint8_t *buffer, size_t size);
bool lorawan_joined(void);
void lorawan_joinCallback(void);
void lorawan_checkCallback(void);
void lorawan_receiveCallback(void);
void lorawan_doneCallback(void);

#if defined(DATA_EEPROM_BANK2_END)
#define EEPROM_OFFSET_START            ((((DATA_EEPROM_BANK2_END - DATA_EEPROM_BASE) + 1023) & ~1023) - 1024)
#else
#define EEPROM_OFFSET_START            ((((DATA_EEPROM_END - DATA_EEPROM_BASE) + 1023) & ~1023) - 1024)
#endif

#define EEPROM_OFFSET_COMMISSIONING    (EEPROM_OFFSET_START + 0)

#endif /* LORAWAN_PROJECT_H */
/*** end of file ***/