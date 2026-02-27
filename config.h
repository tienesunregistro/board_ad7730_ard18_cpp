#ifndef CONFIG_H
#define CONFIG_H

#define IDMAQ "GENERIC"
#define IDBOARD "ARD18-V1-2020-v1.0"
#define SKERNEL "KERNEL - 3.1 - 2026-02-27"
#define CLAVE_VERSION 303

#define ID_ESTACION 1
#define BAUDRATE_RS232 38400 // 115200
#define ADC_USE_INTERRUPT 1

#define LF 0x0a // 10
#define CR 0x0d // 13

// Caracteres de inicio y fin del comando
#define CHAR_INICIO_CMD ':'
#define CHAR_FIN_CMD CR

#define TIMEOUT_CHECK_ALARMAS 200 // ms

//-----------------------------------
// Definición de pines

// Para el RS232
const int PIN_RX_RS232 = 0;
const int PIN_TX_RS232 = 1;

// pines del AD7730
const int PIN_CS_AD7730 = 10;
const int PIN_RDY_AD730 = 2; // Int 0
const int PIN_RESET_AD730 = 8;

// pines del LS7366_CS
const int PIN_CS_LS7366 = 9;

// Pin test debug
const int PIN_TEST_DEBUG = 3;

// pines ALARMAS fuerza positiva y fuerza negativa
const int PIN_ALARMA_FUERZA_POST = 4;
const int PIN_ALARMA_FUERZA_NEG = 5;

//---- ADS7730
#define CS_AD7730_LOW() digitalWrite(PIN_CS_AD7730, LOW)
#define CS_AD7730_HIGH() digitalWrite(PIN_CS_AD7730, HIGH)

#define RESET_AD7730_LOW() digitalWrite(PIN_RESET_AD730, LOW)
#define RESET_AD7730_HIGH() digitalWrite(PIN_RESET_AD730, HIGH)

#define AD7730_RDY() digitalRead(PIN_RDY_AD730)

//-- LS7366
#define CS_LS7366_LOW() digitalWrite(PIN_CS_LS7366, LOW)
#define CS_LS7366_HIGH() digitalWrite(PIN_CS_LS7366, HIGH)

//-- ALARMAS DE SOBRECARGA
#define ALARMA_FUERZA_POST_OFF() digitalWrite(PIN_ALARMA_FUERZA_POST, LOW)
#define ALARMA_FUERZA_POST_ON() digitalWrite(PIN_ALARMA_FUERZA_POST, HIGH)

#define ALARMA_FUERZA_NEG_OFF() digitalWrite(PIN_ALARMA_FUERZA_NEG, LOW)
#define ALARMA_FUERZA_NEG_ON() digitalWrite(PIN_ALARMA_FUERZA_NEG, HIGH)

//-----------------------------------
#define LONG_COMANDO 150

//-- Buffer datos DAC18
#define DATOS_BUFFER_SIZE 32                // Debe ser potencia de 2 (2, 4, 8, 16, 32, 64...)
#define BUFFER_MASK (DATOS_BUFFER_SIZE - 1) // Resultado: 31 (0x1F)

#endif
