#ifndef AD7730DRIVER_H
#define AD7730DRIVER_H

#include <SPI.h>
#include "config.h"
#include "tipos.h"

// Constantes específicas del driver que no necesitan ser globales
#define AD7730_SPI_CLOCK 2000000
#define AD7730_TIMEOUT 0x4FFFF
#define AD7730_SHIFT_BITS 6
#define AD7730_18BIT_MASK (1UL << 17U)
#define AD7730_18BIT_RANGE (1L << 18)
#define AD7730_GLITCH_THRESHOLD 8000

// ---- Valores del Communication Register (idénticos al original driver_ad7730.h) ----
#define CR_SINGLE_WRITE 0x00
#define CR_SINGLE_READ 0x10
#define CR_CONTINUOUS_READ_START 0x20
#define CR_CONTINUOUS_READ_STOP 0x30

#define CR_COMMUNICATION_REGISTER 0x00
#define CR_STATUS_REGISTER 0x00
#define CR_DATA_REGISTER 0x01
#define CR_MODE_REGISTER 0x02
#define CR_FILTER_REGISTER 0x03
#define CR_DAC_REGISTER 0x04
#define CR_OFFSET_REGISTER 0x05
#define CR_GAIN_REGISTER 0x06
#define CR_TEST_REGISTER 0x07

// ---- Mode Register ----
#define MR1_MODE_IDLE 0x00
#define MR1_MODE_CONTINUOUS 0x20
#define MR1_MODE_SINGLE_CONVERSION 0x40
#define MR1_MODE_STANDBY 0x60
#define MR1_MODE_INTERNAL_ZERO_CALIBRATION 0x80
#define MR1_MODE_INTERNAL_FULL_CALIBRATION 0xA0
#define MR1_MODE_SYSTEM_ZERO_CALIBRATION 0xC0
#define MR1_MODE_SYSTEM_FULL_CALIBRATION 0xE0
#define MR1_BU_BIPOLAR 0x00
#define MR1_BU_UNIPOLAR 0x10
#define MR1_WL_24_BIT 0x01
#define MR1_WL_16_BIT 0x00

#define MR0_HIREF_5V 0x80
#define MR0_HIREF_2P5V 0x00
#define MR0_RANGE_10MV 0x00
#define MR0_RANGE_20MV 0x01
#define MR0_RANGE_40MV 0x02
#define MR0_RANGE_80MV 0x03
#define MR0_CHANNEL_1 0x00
#define MR0_CHANNEL_2 0x01

#define CURRENT_MODE_1_SETTINGS (MR1_BU_BIPOLAR | MR1_WL_24_BIT)
#define CURRENT_MODE_0_SETTINGS (MR0_HIREF_5V | MR0_RANGE_10MV | MR0_CHANNEL_1)

// ---- Filter Register (data rates con CHOP OFF) ----
#define FR2_DATA_RATE_50 0x30
#define FR2_DATA_RATE_150 0x80
#define FR2_DATA_RATE_300 0x40
#define FR2_DATA_RATE_400 0x30
#define FR2_DATA_RATE_600 0x20
#define FR2_DATA_RATE_800 0x18
#define FR2_DATA_RATE_1000 0x13
#define FR2_DATA_RATE_1200 0x10

#define FR1_SKIP_OFF 0x00
#define FR1_SKIP_ON 0x02
#define FR1_FAST_ON 0x01
#define FR1_FAST_OFF 0x00

#define FR0_CHOP_ON 0x10
#define FR0_CHOP_OFF 0x00

// ---- DAC Register ----
#define DACR_OFFSET_SIGN_POSITIVE 0x00
#define DACR_OFFSET_SIGN_NEGATIVE 0x20
#define DACR_OFFSET_NONE 0x00

class AD7730Driver
{
public:
    AD7730Driver(int csPin, int rdyPin, int resetPin);
    void init(TDataRate dataRate);
    void reset();
    long leerDatoConFiltro();
    long leerDatoConFiltroISR(); // Versión para llamar desde ISR (sin beginTransaction)
    void setIsrFlag();
    bool getIsrFlag();
    void clearIsrFlag();
    void handleInterrupt();

    volatile bool configurado = false;

private:
    int _csPin;
    int _rdyPin;
    int _resetPin;
    SPISettings _spiSettings;
    volatile long _ultimoValorValido = 0;
    volatile bool _isrFlag = false;

    void _sendByte(uint8_t toSend);
    void _sendBytes(const uint8_t *data, uint8_t len);
    void _sendByteISR(uint8_t toSend); // SPI directo sin transaction
    long _readConversionDataISR();     // Lectura SPI directa para ISR
    bool _waitForReady();
    long _readConversionData();
};

#endif // AD7730DRIVER_H