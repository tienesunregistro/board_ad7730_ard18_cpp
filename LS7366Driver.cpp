#include "LS7366Driver.h"

// MDR0 configuration constants
#define N_QUAD 0x00
#define A_QUAD_B 0x01
#define B_QUAD_A 0x02
#define FREE_RUN 0x03
#define DISABLE_INDX 0x00
#define LOAD_CNTR 0x04
#define RESET_CNTR 0x08
#define LOAD_OTR 0x0C
#define ASYNCH_INDX 0x00
#define SYNCH_INDX 0x10
#define FILTER_1 0x00
#define FILTER_2 0x20
#define FILTER_3 0x40
#define FILTER_4 0x60
#define EN_CNTR 0x00
#define DIS_CNTR 0x80

// MDR1 configuration constants
#define FOUR_BYTE 0x00
#define THREE_BYTE 0x01
#define TWO_BYTE 0x02
#define ONE_BYTE 0x03
#define EN_CNTR_M1 0x00
#define DIS_CNTR_M1 0x04
#define NO_FLAGS 0x00
#define BW_FLAG 0x10
#define CY_FLAG 0x20
#define IDX_FLAG 0x40
#define CMP_FLAG 0x80

// Register addresses
#define CLR_MDR0 0x08
#define CLR_MDR1 0x10
#define CLR_CNTR 0x20
#define CLR_OTR 0x28
#define CLR_STR 0x30
#define READ_MDR0 0x48
#define READ_MDR1 0x50
#define READ_CNTR 0x60
#define READ_OTR 0x68
#define READ_STR 0x70
#define WRITE_MDR0 0x88
#define WRITE_MDR1 0x90
#define WRITE_DTR 0x98
#define LOAD_CNTR_CMD 0xE0
#define LOAD_OTR_CMD 0xE4

LS7366Driver::LS7366Driver(int csPin) :
    _csPin(csPin),
    _spiSettings(4000000, MSBFIRST, SPI_MODE0) // 4MHz, MSB first, Mode 0
{
}

void LS7366Driver::init(byte mode) {
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);

    // Configurar MDR0: modo, index, filtro
    _writeReg(WRITE_MDR0, mode | ASYNCH_INDX | FILTER_1);
    // Configurar MDR1: 4-byte counter, no flags
    _writeReg(WRITE_MDR1, FOUR_BYTE | EN_CNTR_M1);
    
    reset();
}

void LS7366Driver::reset() {
    _writeReg(CLR_CNTR, 0x00);
}

void LS7366Driver::cero() {
    // El comando para poner a cero es el mismo que resetear el contador
    reset();
}

long LS7366Driver::leer() {
    return _readReg(READ_CNTR);
}

void LS7366Driver::_writeReg(byte reg, byte value) {
    noInterrupts();
    SPI.beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    SPI.transfer(reg);
    SPI.transfer(value);
    digitalWrite(_csPin, HIGH);
    SPI.endTransaction();
    interrupts();
}

long LS7366Driver::_readReg(byte reg) {
    long data = 0;
    noInterrupts();
    SPI.beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    SPI.transfer(reg);
    data |= (long)SPI.transfer(0) << 24;
    data |= (long)SPI.transfer(0) << 16;
    data |= (long)SPI.transfer(0) << 8;
    data |= (long)SPI.transfer(0);
    digitalWrite(_csPin, HIGH);
    SPI.endTransaction();
    interrupts();
    return data;
}